/**
 * @file IntrospectionParser.cpp
 * @brief Source file for class IntrospectionParser
 * @date 19/01/2016
 * @author Giuseppe Ferrò
 *
 * @copyright Copyright 2015 F4E | European Joint Undertaking for ITER and
 * the Development of Fusion Energy ('Fusion for Energy').
 * Licensed under the EUPL, Version 1.1 or - as soon they will be approved
 * by the European Commission - subsequent versions of the EUPL (the "Licence")
 * You may not use this work except in compliance with the Licence.
 * You may obtain a copy of the Licence at: http://ec.europa.eu/idabc/eupl
 *
 * @warning Unless required by applicable law or agreed to in writing, 
 * software distributed under the Licence is distributed on an "AS IS"
 * basis, WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express
 * or implied. See the Licence permissions and limitations under the Licence.

 * @details This source file contains the definition of all the methods for
 * the class IntrospectionParser (public, protected, and private). Be aware that some 
 * methods, such as those inline could be defined on the header file, instead.
 */

/*---------------------------------------------------------------------------*/
/*                         Standard header includes                          */
/*---------------------------------------------------------------------------*/

/*---------------------------------------------------------------------------*/
/*                         Project header includes                           */
/*---------------------------------------------------------------------------*/

#include "StandardParser.h"
#include "XMLParser.h"
#include "JsonParser.h"
#include "BasicFile.h"
#include "ConfigurationDatabase.h"
#include "AnyTypeCreator.h"
#include "StreamString.h"
#include <stdio.h>

/*---------------------------------------------------------------------------*/
/*                           Static definitions                              */
/*---------------------------------------------------------------------------*/

/*---------------------------------------------------------------------------*/
/*                           Method definitions                              */
/*---------------------------------------------------------------------------*/

using namespace MARTe;

static const char8 * nodeNames[] = { "Parameters", "InputSignals", "OutputSignals", 0 };

static void PrintOnFile(BasicFile &fileIn,
                        const char8 *data) {
    uint32 stringSize = StringHelper::Length(data);
    fileIn.Write(data, stringSize);
}

static void PrintOnFile(BasicFile &fileIn,
                        const char8 data) {
    uint32 charSize = 1;
    fileIn.Write(&data, charSize);
}

static uint32 GetModifierString(char8* data,
                                StreamString &modifiers) {
    uint32 dataSize = StringHelper::Length(data);
    uint32 i;
    for (i = 0u; i < dataSize; i++) {
        if (data[i] == '*') {
            modifiers += "* ";
        }
        else if (data[i] == 'C') {
            modifiers += "const ";
        }
        else {
            break;
        }
    }

    return i;
}

static void IntrospectionOnHeader(const char8 *memberName,
                                  StreamString &modifiers,
                                  StreamString &type,
                                  StreamString &attributes,
                                  StreamString &comments,
                                  BasicFile &structHeader,
                                  bool isMacro = false) {
    // print the attributes as comment before the member on the header file
    if (attributes.Size() > 0u) {
        PrintOnFile(structHeader, "    ");
        PrintOnFile(structHeader, "/** ");
        PrintOnFile(structHeader, attributes.Buffer());
        PrintOnFile(structHeader, " */");
        if (isMacro) {
            PrintOnFile(structHeader, "\\");
        }
        PrintOnFile(structHeader, "\n");
    }

    // print the comments before the member on the header file
    if (comments.Size() > 0u) {
        PrintOnFile(structHeader, "    ");
        PrintOnFile(structHeader, "/** ");
        PrintOnFile(structHeader, comments.Buffer());
        PrintOnFile(structHeader, " */");
        if (isMacro) {
            PrintOnFile(structHeader, "\\");
        }
        PrintOnFile(structHeader, "\n");
    }

    // print the type
    PrintOnFile(structHeader, "    ");

    PrintOnFile(structHeader, type.Buffer());
    PrintOnFile(structHeader, " ");
    StreamString modifiersString;

    uint32 nextIndex = GetModifierString(modifiers.BufferReference(), modifiersString);
    // print the modifiers
    PrintOnFile(structHeader, modifiersString.Buffer());

    // print the member name
    PrintOnFile(structHeader, memberName);
    while (modifiers[nextIndex] != '\0') {
        PrintOnFile(structHeader, modifiers[nextIndex]);
        nextIndex++;
    }
    PrintOnFile(structHeader, ";");

}

static void IntrospectionOnSourceBefore(const char8 *memberName,
                                        StreamString &modifiers,
                                        StreamString &structType,
                                        StreamString &type,
                                        StreamString &attributes,
                                        BasicFile &structSource,
                                        bool isParent = false) {
    if (isParent) {
        PrintOnFile(structSource, "DECLARE_CLASS_PARENT(");
    }
    else {
        PrintOnFile(structSource, "DECLARE_CLASS_MEMBER(");
    }
    PrintOnFile(structSource, structType.Buffer());
    PrintOnFile(structSource, ", ");
    PrintOnFile(structSource, memberName);
    PrintOnFile(structSource, ", ");
    if (!isParent) {
        PrintOnFile(structSource, type.Buffer());
        PrintOnFile(structSource, ", ");
    }
    PrintOnFile(structSource, "\"");
    PrintOnFile(structSource, modifiers.Buffer());
    PrintOnFile(structSource, "\"");
    PrintOnFile(structSource, ", ");
    PrintOnFile(structSource, "\"");
    PrintOnFile(structSource, attributes.Buffer());
    PrintOnFile(structSource, "\"");

    PrintOnFile(structSource, ");\n");

}

static void IntrospectionOnSourceAfter(ConfigurationDatabase &database,
                                       StreamString &structType,
                                       BasicFile &structSource,
                                       bool isClass = false) {

    PrintOnFile(structSource, "\nstatic const IntrospectionEntry * ");
    PrintOnFile(structSource, structType.Buffer());
    PrintOnFile(structSource, "_array[] = {\n");

    for (uint32 j = 0u; j < database.GetNumberOfChildren(); j++) {
        const char8 *memberName = database.GetChildName(j);
        if (database.MoveRelative(memberName)) {
            if (memberName[0] != '*') {
                PrintOnFile(structSource, "&");
                PrintOnFile(structSource, structType.Buffer());
                PrintOnFile(structSource, "_");
                PrintOnFile(structSource, &memberName[memberName[0] == '$']);
                PrintOnFile(structSource, "_introspectionEntry, \n");
            }
            database.MoveToAncestor(1u);
        }
    }
    PrintOnFile(structSource, "0 };\n\n");

    // declare the class introspection
    if (isClass) {
        PrintOnFile(structSource, "DECLARE_CLASS_INTROSPECTION(");
    }
    else {
        PrintOnFile(structSource, "DECLARE_STRUCT_INTROSPECTION(");
    }
    PrintOnFile(structSource, structType.Buffer());
    PrintOnFile(structSource, ", ");
    PrintOnFile(structSource, structType.Buffer());
    PrintOnFile(structSource, "_array);\n");

}

static void ReadIntrospectionAttributes(ConfigurationDatabase &database,
                                        StreamString &modifiers,
                                        StreamString &type,
                                        StreamString &attributes,
                                        StreamString &comments,
                                        const char8* structName,
                                        const char8* memberName) {

    // print the attributes as comment before the member on the header file
    if (!database.Read("attributes", attributes)) {
        printf("\n[%s.%s][attributes] lacks: the attributes is considered as an empty string", structName, memberName);
        attributes = "";
    }

    // print the comments before the member on the header file
    if (!database.Read("comments", comments)) {
        printf("\n[%s.%s][comments] lacks: the comments is considered as an empty string", structName, memberName);
        comments = "";
    }

    if (!database.Read("type", type)) {
        printf("\n[%s.%s][type] lacks: the member type is considered void", structName, memberName);
        type = "void";
    }

    if (!database.Read("modifiers", modifiers)) {
        printf("\n[%s.%s][modifiers] lacks: the modifiers is considered as an empty string", structName, memberName);
        modifiers = "";
    }

}

static void PrintIntrospection(ConfigurationDatabase &database,
                               const char8 * structName,
                               BasicFile &structHeader,
                               BasicFile &structSource) {
    StreamString structType;
    if (database.Read("type", structType)) {

        PrintOnFile(structHeader, "struct ");
        PrintOnFile(structHeader, structType.Buffer());
        PrintOnFile(structHeader, " { \n");
        for (uint32 j = 0u; j < database.GetNumberOfChildren(); j++) {
            // HEADER FILE MANAGEMENT
            const char8 *memberName = database.GetChildName(j);
            if (database.MoveRelative(memberName)) {

                StreamString modifiers;
                StreamString type;
                StreamString attributes;
                StreamString comments;

                ReadIntrospectionAttributes(database, modifiers, type, attributes, comments, structName, memberName);
                IntrospectionOnHeader(memberName, modifiers, type, attributes, comments, structHeader);
                PrintOnFile(structHeader, "\n");

                // SOURCE FILE MANAGEMENT
                // declare the member introspection
                IntrospectionOnSourceBefore(memberName, modifiers, structType, type, attributes, structSource);

                database.MoveToAncestor(1);
            }
        }

        IntrospectionOnSourceAfter(database, structType, structSource);

        PrintOnFile(structHeader, "}; \n\n");
    }
    else {
        printf("\nError, undefined type for %s", structName);
    }
}

static void GenerateStructFile(ConfigurationDatabase &database,
                               BasicFile &structHeader,
                               BasicFile &structSource) {

    const char8* structName = database.GetName();
    StreamString comments;
    database.Read("comments", comments);
    bool isStruct = false;
    for (uint32 j = 0u; j < database.GetNumberOfChildren(); j++) {
        // HEADER FILE MANAGEMENT
        const char8 *memberName = database.GetChildName(j);
        if (database.MoveRelative(memberName)) {
            isStruct = true;
            GenerateStructFile(database, structHeader, structSource);
            database.MoveToAncestor(1u);
        }
    }
    if (isStruct) {
        if (comments.Size() > 0u) {
            PrintOnFile(structHeader, "/*");
            PrintOnFile(structHeader, comments.Buffer());
            PrintOnFile(structHeader, "*/\n");
        }
        PrintIntrospection(database, structName, structHeader, structSource);
    }

}

static void ReadTheType(const char8 *paramName,
                        const char8 *typeName,
                        const char8 *paramAddress,
                        const char8 *attributes,
                        BasicFile &objSource) {
    bool isStructType = (TypeDescriptor::GetTypeDescriptorFromTypeName(typeName) == InvalidType);

    PrintOnFile(objSource, "    if(ret) {\n");
    if (isStructType) {
        PrintOnFile(objSource, "        ANY_TYPE_STRUCT_BUILDER(");
        PrintOnFile(objSource, typeName);
        PrintOnFile(objSource, ", ");
        PrintOnFile(objSource, paramName);
        PrintOnFile(objSource, ");\n");
        PrintOnFile(objSource, "        ret = data.AdvancedRead(\"");
        PrintOnFile(objSource, paramAddress);
        PrintOnFile(objSource, "\",\"");
        PrintOnFile(objSource, attributes);
        PrintOnFile(objSource, "\", ");
        PrintOnFile(objSource, typeName);
        PrintOnFile(objSource, "_at);\n");
    }
    else {
        PrintOnFile(objSource, "        ret = data.AdvancedRead(\"");
        PrintOnFile(objSource, paramAddress);
        PrintOnFile(objSource, "\", \"");
        PrintOnFile(objSource, attributes);
        PrintOnFile(objSource, "\", ");
        PrintOnFile(objSource, paramName);
        PrintOnFile(objSource, ");\n");

    }
    PrintOnFile(objSource, "        if(!ret) {\n"
                "            REPORT_ERROR(ErrorManagement::FatalError, \"Failed loading the parameter ");
    PrintOnFile(objSource, paramAddress);
    PrintOnFile(objSource, "\");\n");
    PrintOnFile(objSource, "        }\n");
    PrintOnFile(objSource, "    }\n");

}

static void ReadTheTypeArray(const char8 *paramName,
                             const char8 *typeName,
                             const char8 *paramAddress,
                             const char8 *attributes,
                             BasicFile &objSource) {

    //TODO calculate the alias here
    StreamString paramAddressStr = paramAddress;
    paramAddressStr.Seek(0ull);
    char8 terminator;
    StreamString paramAlias;
    StreamString paramPath;

    while (paramAddressStr.GetToken(paramAlias, ".", terminator)) {
        if (terminator == '.') {
            if (paramPath.Size() > 0u) {
                paramPath += ".";
            }
            paramPath += paramAlias;
            paramAlias = "";
        }
    }

    PrintOnFile(objSource, "    if(ret) {\n");

    if (paramPath.Size() > 0u) {
        PrintOnFile(objSource, "        StreamString currentPath;\n"
                    "        data.GetFullPath(currentPath);\n");
        PrintOnFile(objSource, "        data.AdvancedMove(\"");
        PrintOnFile(objSource, paramPath.Buffer());
        PrintOnFile(objSource, "\");\n");
    }

    PrintOnFile(objSource, "        numberOf");
    PrintOnFile(objSource, paramName);
    PrintOnFile(objSource, " = 0u;\n");
    PrintOnFile(objSource, "        for(uint32 i=0u; i < data.GetNumberOfChildren(); i++){\n"
                "            StreamString childName = \"");
    PrintOnFile(objSource, paramAlias.Buffer());
    PrintOnFile(objSource, "\";\n");
    PrintOnFile(objSource, "            childName.Printf(\"%d\", numberOf");
    PrintOnFile(objSource, paramName);
    PrintOnFile(objSource, ");\n");
    bool isStructType = (TypeDescriptor::GetTypeDescriptorFromTypeName(typeName) == InvalidType);
    if (isStructType) {
        PrintOnFile(objSource, "            if(data.MoveRelative(childName.Buffer())){\n");
        PrintOnFile(objSource, "                numberOf");
        PrintOnFile(objSource, paramName);
        PrintOnFile(objSource, "++;\n"
                    "                data.MoveToAncestor(1u);\n"
                    "            }\n");
    }
    else {
        PrintOnFile(objSource, "            AnyType at=data.GetType(childName.Buffer());\n"
                    "            if(!at.IsVoid()) {\n");
        PrintOnFile(objSource, "                numberOf");
        PrintOnFile(objSource, paramName);
        PrintOnFile(objSource, "++;\n"
                    "            }\n");
    }

    PrintOnFile(objSource, "        }\n");
    if (paramPath.Size() > 0u) {
        PrintOnFile(objSource, "        data.MoveAbsolute(currentPath.Buffer());\n");
    }
    PrintOnFile(objSource, "        ");
    PrintOnFile(objSource, paramName);
    PrintOnFile(objSource, " = new ");
    PrintOnFile(objSource, typeName);
    PrintOnFile(objSource, "[numberOf");
    PrintOnFile(objSource, paramName);
    PrintOnFile(objSource, "];\n");
    PrintOnFile(objSource, "        for(uint32 i=0u; i < numberOf");
    PrintOnFile(objSource, paramName);
    PrintOnFile(objSource, " && ret; i++){\n"
                "            StreamString childName = \"");
    PrintOnFile(objSource, paramAddress);
    PrintOnFile(objSource, "\";\n");
    PrintOnFile(objSource, "            childName.Printf(\"%d\", i);\n");

    if (isStructType) {

        PrintOnFile(objSource, "            ANY_TYPE_STRUCT_BUILDER(");
        PrintOnFile(objSource, typeName);
        PrintOnFile(objSource, ", ");
        PrintOnFile(objSource, paramName);
        PrintOnFile(objSource, "[i]);\n");

        PrintOnFile(objSource, "            ret = data.AdvancedRead(childName.Buffer(), \"");
        PrintOnFile(objSource, attributes);
        PrintOnFile(objSource, "\", ");
        PrintOnFile(objSource, typeName);
        PrintOnFile(objSource, "_at);\n");
    }
    else {

        PrintOnFile(objSource, "            ret = data.AdvancedRead(childName.Buffer(), \"");
        PrintOnFile(objSource, attributes);
        PrintOnFile(objSource, "\", ");
        PrintOnFile(objSource, paramName);
        PrintOnFile(objSource, "[i]);\n");
    }
    PrintOnFile(objSource, "        }\n");
    PrintOnFile(objSource, "        if(!ret) {\n"
                "            REPORT_ERROR(ErrorManagement::FatalError, \"Failed loading the parameters *");
    if (paramPath.Size() > 0u) {
        PrintOnFile(objSource, paramPath.Buffer());
        PrintOnFile(objSource, ".");
    }
    PrintOnFile(objSource, paramAlias.Buffer());
    PrintOnFile(objSource, "\");\n");
    PrintOnFile(objSource, "        }\n");
    PrintOnFile(objSource, "    }\n");
}

static void GenerateInitialiseFunction(ConfigurationDatabase &database,
                                       BasicFile &objHeader,
                                       BasicFile &objSource,
                                       BasicFile &structSource,
                                       const char8 * className) {
    StreamString classNameStr = className;

    PrintOnFile(objHeader, "    virtual bool ConfigureToolMembers(StructuredDataI &data);\n");

    PrintOnFile(objSource, "bool ");
    PrintOnFile(objSource, className);
    PrintOnFile(objSource, "::");
    PrintOnFile(objSource, "ConfigureToolMembers(StructuredDataI &data){\n");
    PrintOnFile(objSource, "    bool ret = true;\n");

    PrintOnFile(objHeader, "#define TOOL_MEMBERS_DECLARATION()");

    if (database.MoveRelative("Parameters")) {
        uint32 numberOfPars = database.GetNumberOfChildren();

        //flat view on the parameters
        for (uint32 i = 0u; i < numberOfPars; i++) {
            const char8 * paramName = database.GetChildName(i);
            if (database.MoveRelative(paramName)) {

                StreamString modifiers;
                StreamString type;
                StreamString attributes;
                StreamString comments;

                ReadIntrospectionAttributes(database, modifiers, type, attributes, comments, className, paramName);

                if (paramName[0] != '$') {
                    if (type.Size() == 0u) {
                        printf("\nError, undefined type for %s", paramName);
                    }
                    else {
                        PrintOnFile(objHeader, "\\\n");
                        IntrospectionOnHeader(paramName, modifiers, type, attributes, comments, objHeader, true);

                        if (paramName[0] == '*') {
                            PrintOnFile(objHeader, "\\\n    uint32 numberOf");
                            PrintOnFile(objHeader, paramName + 1);
                            PrintOnFile(objHeader, ";");
                        }
                        StreamString paramPath;
                        StreamString paramAddress;
                        if (paramName[0] == '*') {
                            if (!database.Read("source", paramAddress)) {
                                paramAddress = "";
                            }
                            ReadTheTypeArray(&paramName[1], type.Buffer(), paramAddress.Buffer(), attributes.Buffer(), objSource);
                        }
                        else {
                            if (!database.Read("source", paramAddress)) {
                                paramAddress = paramName;
                            }
                            ReadTheType(paramName, type.Buffer(), paramAddress.Buffer(), attributes.Buffer(), objSource);
                        }
                    }
                }

#ifndef SKIP_OBJECT_INTROSPECTION
                //Manage the parameter introspection
                if (paramName[0] != '*') {
                    uint32 begin = paramName[0] == '$';
                    // SOURCE FILE MANAGEMENT
                    // declare the member introspection
                    IntrospectionOnSourceBefore(&paramName[begin], modifiers, classNameStr, type, attributes, structSource, begin);
                }
#endif
                database.MoveToAncestor(1u);
            }
        }

#ifndef SKIP_OBJECT_INTROSPECTION
        IntrospectionOnSourceAfter(database, classNameStr, structSource, true);
#endif
        database.MoveToAncestor(1u);

    }
    PrintOnFile(objSource, "    return ret;\n}\n\n");

}

static void AssignSignalArray(const char8 *signalName,
                              const char8 *signalAlias,
                              const char8 *direction,
                              const char8 *type,
                              BasicFile &objSource) {
    StreamString dir = direction;
    PrintOnFile(objSource, "    if(ret) {\n");
    PrintOnFile(objSource, "        uint32 numberOfSignals = GetNumberOf");
    PrintOnFile(objSource, direction);
    PrintOnFile(objSource, "();\n");
    PrintOnFile(objSource, "        bool ok = true;\n"
                "        numberOf");
    PrintOnFile(objSource, signalName);
    PrintOnFile(objSource, "= 0u;\n");
    PrintOnFile(objSource, "        for(uint32 i=0u; (i<numberOfSignals) && ret && ok; i++){\n"
                "            StreamString signalName = \"");
    PrintOnFile(objSource, signalAlias);
    PrintOnFile(objSource, "\";\n");
    PrintOnFile(objSource, "            signalName.Printf(\"%d\", numberOf");
    PrintOnFile(objSource, signalName);
    PrintOnFile(objSource, ");\n");
    PrintOnFile(objSource, "            uint32 signalId;\n"
                "            ok = GetSignalIndex(");
    PrintOnFile(objSource, direction);
    PrintOnFile(objSource, ", signalId, signalName.Buffer()) >= 0;\n");
    PrintOnFile(objSource, "            if(ok) {\n"
                "                numberOf");
    PrintOnFile(objSource, signalName);
    PrintOnFile(objSource, "++;\n");
    PrintOnFile(objSource, "            }\n"
                "        }\n");
    PrintOnFile(objSource, "        ");
    PrintOnFile(objSource, signalName);
    PrintOnFile(objSource, " = new ");
    PrintOnFile(objSource, type);
    PrintOnFile(objSource, "* [numberOf");
    PrintOnFile(objSource, signalName);
    PrintOnFile(objSource, "];\n");
    PrintOnFile(objSource, "        ok = true;\n");
    PrintOnFile(objSource, "        for(uint32 i=0u; (i<numberOf");
    PrintOnFile(objSource, signalName);
    PrintOnFile(objSource, ") && ret && ok; i++){\n"
                "            StreamString signalName = \"");
    PrintOnFile(objSource, signalAlias);
    PrintOnFile(objSource, "\";\n");
    PrintOnFile(objSource, "            signalName.Printf(\"%d\", i);\n");
    PrintOnFile(objSource, "            uint32 signalId;\n"
                "            int32 level = GetSignalIndex(");
    PrintOnFile(objSource, direction);
    PrintOnFile(objSource, ", signalId, signalName.Buffer());\n");
    PrintOnFile(objSource, "            ret = (level >= 0);\n");

    PrintOnFile(objSource, "            if(ret) {\n"
                "                StreamString typeName;\n"
                "                ret = GetSignalType(");
    PrintOnFile(objSource, direction);
    PrintOnFile(objSource, ", signalId, typeName, level);\n");
    PrintOnFile(objSource, "                uint32 byteSize;\n"
                "                if(ret) {\n"
                "                    ret = GetSignalTypeByteSize(");
    PrintOnFile(objSource, direction);
    PrintOnFile(objSource, ", signalId, byteSize, level);\n"
                "                }\n");
    PrintOnFile(objSource, "                if(ret) {\n"
                "                    ret = byteSize == sizeof(");
    PrintOnFile(objSource, type);
    PrintOnFile(objSource, ");\n"
                "                }\n");
    PrintOnFile(objSource, "                if(ret) {\n"
                "                    if(typeName != \"");
    PrintOnFile(objSource, type);
    PrintOnFile(objSource, "\") {\n"
                "                        REPORT_ERROR_PARAMETERS(ErrorManagement::Warning, \"Signal ");
    PrintOnFile(objSource, signalAlias);
    PrintOnFile(objSource, " with type %s assigned to type ");
    PrintOnFile(objSource, type);
    PrintOnFile(objSource, "\", typeName.Buffer())\n"
                "                    }\n");
    PrintOnFile(objSource, "                }\n");
    PrintOnFile(objSource, "                if(ret) {\n"
                "                    ");
    PrintOnFile(objSource, signalName);
    PrintOnFile(objSource, "[i]");
    if (dir == "InputSignals") {
        PrintOnFile(objSource, " = (");
        PrintOnFile(objSource, type);
        PrintOnFile(objSource, " *) ");
        PrintOnFile(objSource, "GetInputSignalMemory(signalId);\n");
    }
    else {
        PrintOnFile(objSource, " = (");
        PrintOnFile(objSource, type);
        PrintOnFile(objSource, " *) ");
        PrintOnFile(objSource, "GetOutputSignalMemory(signalId);\n");
    }
    PrintOnFile(objSource, "                    ret = (");
    PrintOnFile(objSource, signalName);
    PrintOnFile(objSource, "[i]");
    PrintOnFile(objSource, " != NULL);\n");
    PrintOnFile(objSource, "                }\n");
    PrintOnFile(objSource, "            }\n"
                "        }\n");
    PrintOnFile(objSource, "        if(!ret) {\n"
                "            REPORT_ERROR(ErrorManagement::FatalError, \"Failed loading the signals *");
    PrintOnFile(objSource, signalAlias);
    PrintOnFile(objSource, "\");\n");
    PrintOnFile(objSource, "        }\n");
    PrintOnFile(objSource, "    }\n");
}

static void AssignSignal(const char8 *signalName,
                         const char8 *signalAlias,
                         const char8 *direction,
                         const char8 *type,
                         BasicFile &objSource) {
    StreamString dir = direction;
    PrintOnFile(objSource, "    if(ret) {\n"
                "        uint32 signalId;\n"
                "        int32 level = GetSignalIndex(");
    PrintOnFile(objSource, direction);
    PrintOnFile(objSource, ", signalId, \"");
    PrintOnFile(objSource, signalAlias);
    PrintOnFile(objSource, "\");\n");
    PrintOnFile(objSource, "        ret = (level >= 0);\n");

    PrintOnFile(objSource, "        if(ret) {\n"
                "            StreamString typeName;\n"
                "            ret = GetSignalType(");
    PrintOnFile(objSource, direction);
    PrintOnFile(objSource, ", signalId, typeName, level);\n");
    PrintOnFile(objSource, "            uint32 byteSize;\n"
                "            if(ret) {\n"
                "                ret = GetSignalTypeByteSize(");
    PrintOnFile(objSource, direction);
    PrintOnFile(objSource, ", signalId, byteSize, level);\n"
                "            }\n");
    PrintOnFile(objSource, "            if(ret) {\n"
                "                ret = byteSize == sizeof(");
    PrintOnFile(objSource, type);
    PrintOnFile(objSource, ");\n"
                "            }\n");
    PrintOnFile(objSource, "            if(ret) {\n"
                "                if(typeName != \"");
    PrintOnFile(objSource, type);
    PrintOnFile(objSource, "\") {\n"
                "                    REPORT_ERROR_PARAMETERS(ErrorManagement::Warning, \"Signal ");
    PrintOnFile(objSource, signalAlias);
    PrintOnFile(objSource, " with type %s assigned to type ");
    PrintOnFile(objSource, type);
    PrintOnFile(objSource, "\", typeName.Buffer())\n"
                "                }\n");
    PrintOnFile(objSource, "            }\n");
    PrintOnFile(objSource, "        }\n");
    PrintOnFile(objSource, "        if(ret) {\n");
    PrintOnFile(objSource, "            ");
    PrintOnFile(objSource, signalName);
    if (dir == "InputSignals") {
        PrintOnFile(objSource, " = (");
        PrintOnFile(objSource, type);
        PrintOnFile(objSource, " *) ");
        PrintOnFile(objSource, "GetInputSignalMemory(signalId);\n");
    }
    else {
        PrintOnFile(objSource, " = (");
        PrintOnFile(objSource, type);
        PrintOnFile(objSource, " *) ");
        PrintOnFile(objSource, "GetOutputSignalMemory(signalId);\n");
    }
    PrintOnFile(objSource, "            ret = (");
    PrintOnFile(objSource, signalName);
    PrintOnFile(objSource, " != NULL);\n");
    PrintOnFile(objSource, "        }\n");
    PrintOnFile(objSource, "        if(!ret) {\n"
                "            REPORT_ERROR(ErrorManagement::FatalError, \"Failed loading the signal ");
    PrintOnFile(objSource, signalAlias);
    PrintOnFile(objSource, "\");\n");
    PrintOnFile(objSource, "        }\n");
    PrintOnFile(objSource, "    }\n");
}

static void GenerateConfigureFunction(ConfigurationDatabase &database,
                                      BasicFile &objHeader,
                                      BasicFile &objSource,
                                      const char8 * className) {
    PrintOnFile(objSource, "bool ");
    PrintOnFile(objSource, className);
    PrintOnFile(objSource, "::ConfigureToolSignals() {\n");
    PrintOnFile(objSource, "    bool ret = true;\n");

    const char8 * signalsNode[] = { "InputSignals", "OutputSignals", 0 };
    uint32 n = 0u;
    while (signalsNode[n] != NULL) {
        if (database.MoveRelative(signalsNode[n])) {
            uint32 numberOfPars = database.GetNumberOfChildren();

            for (uint32 i = 0u; i < numberOfPars; i++) {
                const char8 * signalName = database.GetChildName(i);
                if (database.MoveRelative(signalName)) {
                    StreamString typeName;
                    if (!database.Read("type", typeName)) {
                        printf("\nError, undefined type for %s", signalName);
                    }
                    else {
                        StreamString signalAlias;
                        if (!database.Read("source", signalAlias)) {
                            uint32 begin = signalName[0] == '*';
                            signalAlias = signalName + begin;
                        }
                        PrintOnFile(objHeader, "\\\n    ");
                        PrintOnFile(objHeader, typeName.Buffer());
                        PrintOnFile(objHeader, " *");
                        PrintOnFile(objHeader, signalName);
                        PrintOnFile(objHeader, ";");

                        if (signalName[0] == '*') {
                            PrintOnFile(objHeader, "\\\n    uint32 numberOf");
                            PrintOnFile(objHeader, signalName + 1);
                            PrintOnFile(objHeader, ";");
                            AssignSignalArray(signalName + 1, signalAlias.Buffer(), signalsNode[n], typeName.Buffer(), objSource);
                        }
                        else {
                            AssignSignal(signalName, signalAlias.Buffer(), signalsNode[n], typeName.Buffer(), objSource);
                        }
                    }
                    database.MoveToAncestor(1u);
                }
            }
            database.MoveToAncestor(1u);
        }
        n++;
    }
    PrintOnFile(objSource, "    return ret;\n");
    PrintOnFile(objSource, "}\n\n");

}

static void GenerateObjFile(ConfigurationDatabase &database,
                            BasicFile &objHeader,
                            BasicFile &objSource,
                            BasicFile &structSource,
                            const char8 * className) {

    //generate the .cpp and .h files with all the object configuration
    //implementing the Object::Initialise() function

    PrintOnFile(objHeader, "#define TOOL_METHODS_DECLARATION() \\\n");
    bool isSignal = true;
    if (!database.MoveRelative("InputSignals")) {
        if (!database.MoveRelative("InputSignals")) {
            isSignal = false;
        }
    }
    if (isSignal) {
        PrintOnFile(objHeader, "    virtual bool ConfigureToolSignals();\\\n");
        database.MoveToAncestor(1u);
    }

    PrintOnFile(objHeader, "    virtual void ToolMembersConstructor();\\\n");
    PrintOnFile(objHeader, "    virtual void ToolMembersDestructor(); \\\n");

    PrintOnFile(objSource, "#include \"");
    PrintOnFile(objSource, className);
    PrintOnFile(objSource, ".h\"\n");

    PrintOnFile(objSource, "#include \"AdvancedErrorManagement.h\"\n\n");

    // Constructor and destructor
    PrintOnFile(objSource, "void ");
    PrintOnFile(objSource, className);
    PrintOnFile(objSource, "::");
    PrintOnFile(objSource, "ToolMembersConstructor");
    PrintOnFile(objSource, "() {\n");

    uint32 n = 0u;
    while (nodeNames[n] != NULL) {
        if (database.MoveRelative(nodeNames[n])) {
            uint32 numberOfPars = database.GetNumberOfChildren();

            for (uint32 i = 0u; i < numberOfPars; i++) {
                const char8 * paramName = database.GetChildName(i);
                if (database.MoveRelative(paramName)) {
                    bool initAsNull = StringHelper::Compare(nodeNames[n], "Parameters") != 0;
                    if (!initAsNull) {
                        initAsNull = (paramName[0] == '*');
                    }
                    if (initAsNull) {
                        PrintOnFile(objSource, "    ");
                        PrintOnFile(objSource, &paramName[paramName[0] == '*']);
                        PrintOnFile(objSource, " = NULL;\n");
                    }
                    if (paramName[0] == '*') {
                        PrintOnFile(objSource, "    numberOf");
                        PrintOnFile(objSource, &paramName[1]);
                        PrintOnFile(objSource, " = 0u;\n");
                    }
                    database.MoveToAncestor(1u);
                }
            }
            database.MoveToAncestor(1u);
        }
        n++;
    }

    PrintOnFile(objSource, "}\n\n");

    PrintOnFile(objSource, "void ");
    PrintOnFile(objSource, className);
    PrintOnFile(objSource, "::");
    PrintOnFile(objSource, "ToolMembersDestructor");
    PrintOnFile(objSource, "() {\n");
    n = 0u;
    while (nodeNames[n] != NULL) {
        if (database.MoveRelative(nodeNames[n])) {
            uint32 numberOfPars = database.GetNumberOfChildren();

            for (uint32 i = 0u; i < numberOfPars; i++) {
                const char8 * paramName = database.GetChildName(i);
                if (database.MoveRelative(paramName)) {
                    if (paramName[0] == '*') {
                        PrintOnFile(objSource, "    if(");
                        PrintOnFile(objSource, &paramName[1]);
                        PrintOnFile(objSource, " != NULL) {\n"
                                    "        delete [] ");
                        PrintOnFile(objSource, &paramName[1]);
                        PrintOnFile(objSource, ";\n"
                                    "    }\n");
                    }
                    database.MoveToAncestor(1u);
                }
            }
            database.MoveToAncestor(1u);
        }
        n++;
    }
    PrintOnFile(objSource, "}\n\n");

    //Populates the Initialise function in the cpp file
    GenerateInitialiseFunction(database, objHeader, objSource, structSource, className);
    //Populates the Setup function in the cpp file
    GenerateConfigureFunction(database, objHeader, objSource, className);

}

static void GenerateOutputFiles(ConfigurationDatabase &database) {
    // files for introspection
    BasicFile structHeader;
    BasicFile structSource;

    StreamString className;
    if (!database.Read("Class", className)) {
        printf("\nclass name not specified");
        return;
    }

    StreamString classStructName = className;
    classStructName += "_params";

    StreamString structHeaderName = classStructName;
    structHeaderName += ".h";

    // open the .h output file
    if (!structHeader.Open(structHeaderName.Buffer(),
                           BasicFile::FLAG_APPEND | BasicFile::FLAG_TRUNC | BasicFile::FLAG_CREAT | BasicFile::ACCESS_MODE_R | BasicFile::ACCESS_MODE_W)) {
        printf("\nUnable to open %s", structHeaderName.Buffer());
        return;
    }

    StreamString structSourceName = classStructName;
    structSourceName += ".cpp";

    // open the .cpp output file
    if (!structSource.Open(structSourceName.Buffer(),
                           BasicFile::FLAG_APPEND | BasicFile::FLAG_TRUNC | BasicFile::FLAG_CREAT | BasicFile::ACCESS_MODE_R | BasicFile::ACCESS_MODE_W)) {
        printf("\nUnable to open %s", structSourceName.Buffer());
        return;
    }

    // print the header in the header file
    PrintOnFile(structHeader, "#include \"GeneralDefinitions.h\"\n\n");
    PrintOnFile(structHeader, "namespace MARTe{ \n\n");

    // print the header in the source file
    PrintOnFile(structSource, "#include \"Object.h\"\n#include \"ClassRegistryDatabase.h\"\n");
    PrintOnFile(structSource, "#include \"IntrospectionT.h\"\n#include \"");
    PrintOnFile(structSource, className.Buffer());
    PrintOnFile(structSource, ".h\"\n\n");
    PrintOnFile(structSource, "namespace MARTe{ \n\n");

    BasicFile objHeader;
    StreamString objheaderName = className;
    objheaderName += "_macros.h";
    // open the .h gam file
    if (!objHeader.Open(objheaderName.Buffer(),
                        BasicFile::FLAG_APPEND | BasicFile::FLAG_TRUNC | BasicFile::FLAG_CREAT | BasicFile::ACCESS_MODE_R | BasicFile::ACCESS_MODE_W)) {
        printf("\nUnable to open the gam header file");
        return;
    }

    BasicFile objSource;
    StreamString objSourceName = className;
    objSourceName += "_aux.cpp";
    // open the .cpp gam file
    if (!objSource.Open(objSourceName.Buffer(),
                        BasicFile::FLAG_APPEND | BasicFile::FLAG_TRUNC | BasicFile::FLAG_CREAT | BasicFile::ACCESS_MODE_R | BasicFile::ACCESS_MODE_W)) {
        printf("\nUnable to open the gam source file");
        return;
    }

    GenerateObjFile(database, objHeader, objSource, structSource, className.Buffer());

    uint32 n = 0u;
    while (nodeNames[n] != NULL) {
        if (database.MoveRelative(nodeNames[n])) {
            for (uint32 i = 0u; i < database.GetNumberOfChildren(); i++) {
                const char8* structName = database.GetChildName(i);

                if (database.MoveRelative(structName)) {
                    GenerateStructFile(database, structHeader, structSource);
                    database.MoveToAncestor(1u);
                }
            }
            database.MoveToAncestor(1u);
        }
        n++;
    }

    PrintOnFile(structHeader, "\n}");
    PrintOnFile(structSource, "\n}");
}


/**
 * HOWTO:
 * TheTool.ex [config_file_name.cfg] [parser_identifier]
 * @param config_file_name.cfg is the name of the configuration file to be provided to the tool
 * @parser_identifier specifies the parser to be used depending on the language. It can be 1 (MARTe cfg language) 2 (XML language) or 3 (Json language)
 * @details The goal of the tool is generating automatically all the Introspection meta-data for structures in order to make the Object able to read
 * them directly from a StructuredDataI (i.e a ConfigurationDatabase). The input configuration file has to be written in one of the supported parsable languages
 * (MARTe CFG, XML or JSON). Assuming for example to use the MARTe CFG language, the input file has the following form:
 *     Class = "the name of the Object class"
 *     Parameters = {
 *         ...
 *     }
 *     InputSignals = {
 *         ...
 *     }
 *     OutputSignals = {
 *         ...
 *     }
 * It is not mandatory define all the nodes. In particular if the Object is not a GAM there is no need to define InputSignals and OutputSignals nodes.
 * All the structures defined inside the nodes will be declared in a file called [class_name]_Params.h and their introspections will be defined in
 * [class_name]_Params.cpp. Inside the nodes, the user can specify the variables which can be structures or basic types. The structures, as explained before,
 * will be declared and their introspection generated. A variable in the tool in put file has the following form
 *     (*)Variable_Name = {
 *         type = ...
 *         comments = ...
 *         modifiers = ...
 *         attributes = ...
 *         source = ... (the address of the variable in the StructuredDataI used for initialise the Object)
 *         (other possible variables in case of structure)
 *     }
 * These fields can be omitted and they are considered empty, unless the type which has to be specified. If the "source" field is not specified, the address is
 * the "Variable_Name" itself.
 * A structure is simply a variable containing other variables. If a variable is a structure, the modifiers and attributes fields will populate the introspection
 * declaration of members. The special character (*) can be:
 *     - * : the variable is an array of undefined size, namely a pointer to a memory which has to be dynamically allocated.
 *     - $ : (can be used only in "Parameters" node) the following name is the name of a parent class. It is used to define the introspection entry of
 *     the parent classes, since the tool generates the introspection for the object itself and not only for the declared structures.
 *
 * The tool creates also two more files containing all the initialisation and configuration of the Object that can be generated from the tool input file.
 * The file called [class_name]_macros.h contains the two following macros:
 *     TOOL_MEMBERS_DECLARATION():
 *         Contains the declarations of the class members, namely the variables defined in the tool input file. When a variable is
 *         defined as a pointer ('*' before the name), the member is a pointer accordingly and a member called "numberOf[variable_name] is added to store the
 *         size of the array. In this case an incremental index beginning from 0 will be appended at the end of the variable "source" field and it will be
 *         incremented when the variable is found. For instance suppose that in the global configuration file we have a set of parameters declared as follows:
 *
 *         MyParameters = {
 *             param_0 = ...
 *             param_1 = ...
 *             param_2 = ...
 *             another_param
 *             ...
 *         }
 *
 *         If in the tool input file we declare:
 *
 *         *parameters_array = {
 *             source = MyParameters.param_
 *             type = uint32
 *             ...
 *         }
 *
 *         the tool generates the two class members:
 *
 *         uint32 *parameters_array;
 *         uint32 numberOfparameters_array;
 *
 *         and during the configuration numberOfparameters_array can be calculated trying to read "MyParameters.param_(i)" and incrementing i until
 *         the read fails. After that the memory associated to parameters_array can be allocated.
 *         All the variables declared in the "InputSignals" and "OutputSignals" nodes will be added as GAM pointer members. For instance declaring
 *
 *         signal_x = {
 *             type = uint32
 *             ...
 *         }
 *
 *         *signal_array = {
 *             type = uint32
 *             ...
 *         }
 *
 *         the tool generates:
 *
 *         uint32 *signal_x;
 *         uint32 **signal_array;
 *         uint32 numberOfsignal_array;
 *
 *         During the configuration of the signals, the pointers to the signal memory will be associated to the related GAM pointer members.
 *
 *     TOOL_METHODS_DECLARATION(): Contains the declarations of the following functions:
 *             virtual bool ConfigureToolMembers(StructuredDataI &data) :
 *                 To be called in Object::Initialise(StructuredDataI &data), reads from \a data all the variables declared
 *                 in the node "Parameters" of the tool input file (structures or basic types)
 *             virtual bool ConfigureToolSignals() :
 *                 To be called in GAM::Setup(), links the signals to the GAM members declared in the "InputSignals" and "OutputSignals" nodes.
 *             virtual void ToolMembersConstructor() :
 *                 To be called in the constructor, initialises all the members (namely the variables) defined in the tool input file.
 *             virtual void ToolMembersDestructor() :
 *                 To be called in the destructor, destroys all the memory allocated on heap for the variables declared as pointers in the tool input file.
 *
 *  The file [class_name]_aux.cpp contains the definition of the Object methods declared in the TOOL_METHODS_DECLARATION() macro.
 *
 *  @details In order to disable the generation of the Object introspection define the macro SKIP_OBJECT_INTROSPECTION.
 */

int main(int argc,
         char** argv) {

    // the parser
    ParserI *myParser = NULL;

    // the database created by the parser
    ConfigurationDatabase database;

    // the parser errors
    StreamString errors;

    // configuration input file
    BasicFile configFile;

    bool canReturn = false;

    if (argc < 2) {
        printf("\nPlease specify cfg name and output file name");
        canReturn = true;
    }

    if (!canReturn) {

        // open the input file
        if (!configFile.Open((const char*) argv[1], BasicFile::ACCESS_MODE_R)) {
            printf("\nUnable to open the input configuration file");
            canReturn = true;
        }
    }
    // choose the parser
    if (!canReturn) {
        configFile.Seek(0);
        if (argc > 3) {
            if ((argv[2])[0] == '1') {
                myParser = new StandardParser(configFile, database, &errors);
            }
            else if ((argv[2])[0] == '2') {
                myParser = new XMLParser(configFile, database, &errors);
            }
            else if ((argv[2])[0] == '3') {
                myParser = new JsonParser(configFile, database, &errors);
            }
            else {
                printf("\nWrong parser option: use StandardParser by default\n");
                myParser = new StandardParser(configFile, database, &errors);
            }
        }
        // standard parser by default
        else {
            myParser = new StandardParser(configFile, database, &errors);
        }

        // parse the cfg
        if (!myParser->Parse()) {
            printf("\nError Parse(*) %s", errors.Buffer());
            canReturn = true;
        }
    }

    if (!canReturn) {
        // generates the files
        GenerateOutputFiles(database);
    }
    if (myParser != NULL) {
        delete myParser;
    }

    printf("\n");
    return 0;

}