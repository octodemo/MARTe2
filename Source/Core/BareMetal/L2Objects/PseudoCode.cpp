/**
 * @file PseudoCode.cpp
 * @brief Header file for class AnyType
 * @date Mar 26, 2020
 * @author Filippo Sartori
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

 * @details This header file contains the declaration of the class AnyType
 * with all of its public, protected and private members. It may also include
 * definitions for inline methods which need to be visible to the compiler.
*/

#include "PseudoCode.h"
#include <stdio.h>
#include <math.h>
#include "StaticStack.h"
#include "AnyType.h"



namespace MARTe{

namespace PseudoCode{

/** the type of the PCode function */
typedef void (*Function)(Context & context);

/**
 * records information necessary to be able to use it during compilation
 */
struct FunctionRecord{
	/**
	 *	The name of the functions as used in the RPN code
	 */
	CCString name;

	/**
	 * How many stack elements it will consume
	 * !NOTE that for CONST
	 */
	uint16  numberOfInputs;

	/**
	 * How many stack elements it will produce
	 */
	uint16 numberOfOutputs;

	/**
	 * array of types one for each input and output
	 */
	const TypeDescriptor *types;


	/**
	 * The function code itself
	 */
	Function function;

	/**
	 * returns true if the name and types matches
	 * on success replaces the type on the stack with the result type
	 * also simulates variations on the dataStack
	 */
	bool TryConsume(CCString nameIn,StaticStack<TypeDescriptor,32> &typeStack, bool matchOutput,DataMemoryAddress &dataStackSize);

};

bool FunctionRecord::TryConsume(CCString nameIn,StaticStack<TypeDescriptor,32> &typeStack, bool matchOutput,DataMemoryAddress &dataStackSize){
	bool ret = false;

	// match function name
	ret = (name == nameIn);

//if (ret){
//	printf("found %s\n",name.GetList());
//}

	// match first output if matchOutput is set
	uint32 index = 0U;
	if (ret && matchOutput){
		TypeDescriptor type;
//printf("Stack[%i].Peek(%i) \n",typeStack.GetSize(),index);
		ret = typeStack.Peek(index++,type);
		if (ret){
//printf("%s out checking %x vs %x\n",name.GetList(),type.all,types[numberOfInputs].all);
			ret = (type.SameAs(types[numberOfInputs]));
		}
	}

	// match inputs types
	for (uint32 i = 0U; ret && (i < numberOfInputs); i++){
//printf("check inputs\n");
		TypeDescriptor type;
		ret = typeStack.Peek(index++,type);
		if (ret){
//printf("%s in checking %x vs %x\n",name.GetList(),type.all,types[i].all);
			ret = (type.SameAs(types[i]));

		}
	}

	// found! commit changes
	if (ret){
//printf("%s\n",name.GetList());

		// remove first output type
		if (matchOutput){
			TypeDescriptor type;
			typeStack.Pop(type);
//printf("POP t\n");
		}

		// remove inputs types
		for (uint32 i = 0U; ret && (i < numberOfInputs); i++){
			TypeDescriptor type;
			typeStack.Pop(type);
			dataStackSize -= ByteSizeToDataMemorySize(type.StorageSize());
//printf("POP\n");
		}

		// insert output types
		for (uint32 i = 0U; ret && (i < numberOfOutputs); i++){
			typeStack.Push(types[i+numberOfInputs]);
			dataStackSize += ByteSizeToDataMemorySize(types[i+numberOfInputs].StorageSize());
//printf("PUSH\n");
		}
	}

	return ret;
}

/**
 * max number of registered functions
 */
const uint32 maxFunctions = 16384;

/**
 * actually available functions
 */
uint32 availableFunctions = 0;

/**
 * the database of functions
 */
FunctionRecord functionRecords[maxFunctions];

/**
 * to register a function
 */
void RegisterFunction(const FunctionRecord &record){
//printf("Registering fun %s\n",record.name.GetList());
	if (availableFunctions < maxFunctions){
		functionRecords[availableFunctions++] = record;
	}
}

/**
 * find the correct PCode and updates the type in the typestack
 */
bool FindPCodeAndUpdateTypeStack(CodeMemoryElement &code, CCString nameIn,StaticStack<TypeDescriptor,32> &typeStack, bool matchOutput,DataMemoryAddress &dataStackSize){

	uint32 i = 0;
//printf("looking for %s within %i - %i funs\n",nameIn.GetList(),availableFunctions,maxFunctions);

	bool found = false;
	for (i=0; (!found) && (i < availableFunctions);i++ ){
		found = functionRecords[i].TryConsume(nameIn,typeStack,matchOutput,dataStackSize);
		if (found){
			code = i;
		}
	}

	return found;
}


/*************************************************************************************************************************/
/*************************************************************************************************************************/


/**
 * The only 4 standard tokens
 */
const CCString readToken("READ");
const CCString writeToken("WRITE");
const CCString constToken("CONST");
const CCString castToken("CAST");



/**
 * allows searching for a variable with a given name
 */
class VariableFinder: public GenericIterator<Context::VariableInformation>{
public:
	/**
	 *
	 */
	VariableFinder(CCString name);
	/**
	 *
	 */
	VariableFinder(DataMemoryAddress address);
	/**
	 *
	 */
    virtual IteratorAction Do(Context::VariableInformation &data,uint32 depth=0);
    /**
     *
     */
    Context::VariableInformation *variable;
private:
    /**
     *
     */
    DynamicCString variableName;

    /**
     *
     */
    DataMemoryAddress variableAddress;

};


VariableFinder::VariableFinder(CCString name){
	variable = NULL_PTR(Context::VariableInformation*);
	variableName = name;
	variableAddress = MAXDataMemoryAddress;
}

VariableFinder::VariableFinder(DataMemoryAddress address){
	variable = NULL_PTR(Context::VariableInformation*);
	variableAddress = address;
}


IteratorAction VariableFinder::Do(Context::VariableInformation &data,uint32 depth){
	IteratorAction ret;
	if (variableName.GetSize() > 0){
		if (data.name == variableName){
			variable = &data;
			ret.SetActionCode(noAction);
		}
	} else
	if (variableAddress < MAXDataMemoryAddress){
		if (data.location == variableAddress){
			variable = &data;
			ret.SetActionCode(noAction);
		}
	}
	return ret;
}



/*************************************************************************/

Context::Context(){
	variablesMemoryPtr = NULL_PTR(DataMemoryElement*);
	codeMemoryPtr = NULL_PTR(CodeMemoryElement*);
	stackPtr = NULL_PTR(DataMemoryElement*);
	startOfVariables = 0;
}

ErrorManagement::ErrorType Context::FindVariableinDB(CCString name,VariableInformation *&variableInformation,List<VariableInformation> &db){
	ErrorManagement::ErrorType ret;

	VariableFinder finder(name);
	ret = db.Iterate(finder);
	REPORT_ERROR(ret,"Iteration failed");

	variableInformation = NULL;
	if (ret){
		variableInformation = finder.variable;
		ret.unsupportedFeature = (variableInformation == NULL);
	}
	return ret;
}

ErrorManagement::ErrorType Context::AddVariable2DB(CCString name,List<VariableInformation> &db,TypeDescriptor td,DataMemoryAddress location){
	ErrorManagement::ErrorType ret;
	VariableInformation *variableInfo;
	ret = FindVariableinDB(name,variableInfo,db);

	// if it is already there we cannot add
	if (ret.unsupportedFeature){
		VariableInformation variableInfo;
		variableInfo.name = name;
		variableInfo.type = td;
		variableInfo.location = location;
//printf("Add %s @ %i  --> %i\n",name.GetList(),location,variableInfo.location);
		ret = db.Insert(variableInfo );
	} else {
		ret.invalidOperation = true;
	}

	return ret;
}

ErrorManagement::ErrorType Context::FindVariable(DataMemoryAddress address,VariableInformation *&variableInformation){
	ErrorManagement::ErrorType ret;

	VariableFinder finder(address);

	ret = outputVariableInfo.Iterate(finder);

	variableInformation = NULL;
	if (ret){
		variableInformation = finder.variable;
		ret.unsupportedFeature = (variableInformation == NULL);
	}

	if (!ret){
		ret = inputVariableInfo.Iterate(finder);
		if (ret){
			variableInformation = finder.variable;
			ret.unsupportedFeature = (variableInformation == NULL);
		}
	}

	return ret;
}


ErrorManagement::ErrorType Context::BrowseInputVariable(uint32 index,VariableInformation *&variableInformation){
	ErrorManagement::ErrorType ret;
	variableInformation = inputVariableInfo[index];
	ret.outOfRange = (variableInformation == NULL);
	return ret;
}

ErrorManagement::ErrorType Context::BrowseOutputVariable(uint32 index,VariableInformation *&variableInformation){
	ErrorManagement::ErrorType ret;
	variableInformation = outputVariableInfo[index];
	ret.outOfRange = (variableInformation == NULL);
	return ret;
}

ErrorManagement::ErrorType Context::ExtractVariables(CCString RPNCode){
	ErrorManagement::ErrorType ret;

	DataMemoryAddress nextConstantAddress = 0;

	bool finished = false;
	while (!finished  && ret){
		DynamicCString line;
		uint32 limit;
		// divide RPNCode into lines
		RPNCode = DynamicCString::Tokenize(RPNCode,line,limit,"\n","\n\r",false);
		finished = (line.GetSize()==0);
//printf("LINE = %s\n",line.GetList());
		// extract command and parameter
		DynamicCString command;
		DynamicCString parameter;
		if (!finished){
			CCString lineP = line;
			// extract up to two tokens per line
			lineP = DynamicCString::Tokenize(lineP,command,limit," \t,"," \t,",false);
			DynamicCString::Tokenize(lineP,parameter,limit," \t,"," \t,",false);
		}

		// now analyse the command
		if (command.GetSize() > 0){
			bool hasParameter = (parameter.GetSize()> 0);

			if (command == readToken){
				ret.invalidOperation = !hasParameter;
				COMPOSITE_REPORT_ERROR(ret,readToken," without variable name");
				if (ret){
					ret = AddInputVariable(parameter);
					if (ret.invalidOperation == true){
						COMPOSITE_REPORT_ERROR(ret,"variable ",parameter," already registered");
						ret.invalidOperation = false; // mask out the case that we already registered this variable

					}
					COMPOSITE_REPORT_ERROR(ret,"Failed Adding input variable ",parameter);
				}
			} else
			if (command == writeToken){
				ret.invalidOperation = !hasParameter;
				COMPOSITE_REPORT_ERROR(ret,writeToken," without variable name");

				if (ret){
					ret = AddOutputVariable(parameter);
					if (ret.invalidOperation == true){
						COMPOSITE_REPORT_ERROR(ret,"variable ",parameter," already registered");
						ret.invalidOperation = false; // mask out the case that we already registered this variable
					}
					COMPOSITE_REPORT_ERROR(ret,"Failed Adding output variable ",parameter);
				}
			} else
			if (command == constToken){
				ret.invalidOperation = !hasParameter;
				COMPOSITE_REPORT_ERROR(ret,constToken," without type name");

				// transform the type name into a TypeDescriptor
				// check it is one of the supported types
				TypeDescriptor td;
				if (ret){
					td = TypeDescriptor(parameter);
					ret.unsupportedFeature = !td.IsNumericType();
					COMPOSITE_REPORT_ERROR(ret,"type ",parameter, " is not a numeric supported format");
//printf("const type = %x\n",td.all);
				}
				// if supported add up the memory needs
				if (ret){
					DynamicCString constantName;
					constantName().Append("Constant").Append('@').Append(nextConstantAddress);
					ret = AddInputVariable(constantName,td,nextConstantAddress);
				}
				if (ret){
					nextConstantAddress += ByteSizeToDataMemorySize(td.StorageSize());
				}
			}
		}
	}

	if (ret){
		startOfVariables = nextConstantAddress;
	}

	return ret;
}

ErrorManagement::ErrorType Context::Compile(CCString RPNCode){
	ErrorManagement::ErrorType ret;

	DataMemoryAddress nextVariableAddress = startOfVariables;
	// check that all variables have a type and allocate variables + constants

	uint32 index = 0;
	PseudoCode::Context::VariableInformation *var;
	while(BrowseInputVariable(index,var) && ret){
		ret.unsupportedFeature = !var->type.IsNumericType();
		COMPOSITE_REPORT_ERROR(ret,"input variable ",var->name," has incompatible non-numeric type ");

		// skip constants are already allocated
		if (ret && (var->location == MAXDataMemoryAddress)){
			var->location = nextVariableAddress;
			nextVariableAddress += ByteSizeToDataMemorySize(var->type.StorageSize());
		}
		index++;
	}

	index = 0;
	while(BrowseOutputVariable(index,var) && ret){
		ret.unsupportedFeature = !var->type.IsNumericType();
		COMPOSITE_REPORT_ERROR(ret,"input variable ",var->name," has incompatible non-numeric type ");

		if (ret){
			var->location = nextVariableAddress;
			nextVariableAddress += ByteSizeToDataMemorySize(var->type.StorageSize());
		}
		index++;
	}

	// already
	dataMemory.SetSize(nextVariableAddress);
	variablesMemoryPtr = static_cast<DataMemoryElement *>(dataMemory.GetDataPointer());

	// initialise compilation memory
	StaticStack<TypeDescriptor,32> typeStack;
	DataMemoryAddress maxDataStackSize = 0;    // max value of dataStackSize
	DataMemoryAddress dataStackSize = 0;       // current simulated value of data stack size
//	startOfVariables = 0;                      // for now no constants - so variables start at 0
	DataMemoryAddress nextConstantAddress = 0; // pointer to next constant memory area

    // clean all the memory
	codeMemory.Clean();

	bool finished = false;
	while ((!finished)  && ret){
		DynamicCString line;
		uint32 limit;
		// divide RPNCode into lines
		RPNCode = DynamicCString::Tokenize(RPNCode,line,limit,"\n","\n\r",false);

		finished = (line.GetSize()==0);
		// extract command and parameter
		DynamicCString command;
		DynamicCString parameter1;
		DynamicCString parameter2;
		if (!finished){
			CCString lineP = line;
			// extract up to two tokens per line
			lineP = DynamicCString::Tokenize(lineP,command,limit," \t,"," \t,",false);
			lineP = DynamicCString::Tokenize(lineP,parameter1,limit," \t,"," \t,",false);
			DynamicCString::Tokenize(lineP,parameter2,limit," \t,"," \t,",false);
		}

		// now analyze the command
		if (command.GetSize() > 0){
			// assign invalid value
			CodeMemoryElement code2 = TypeCharacteristics<CodeMemoryElement>::MaxValue();
			bool matchOutput = false;

			bool hasParameter1 = (parameter1.GetSize()> 0);

			// PROCESS CAST command
			// PUSH type(parameter1) --> TypeStack
			// matchOutput = true;
			if (command == castToken){
				ret.invalidOperation = !hasParameter1;
				COMPOSITE_REPORT_ERROR(ret,command," without type name");
				if (ret){
					// transform the type name into a TypeDescriptor
					// check it is one of the supported types
					TypeDescriptor td;
					td = TypeDescriptor(parameter1);
					ret.unsupportedFeature = !td.IsNumericType();
					COMPOSITE_REPORT_ERROR(ret,"type ",parameter1, " is not a numeric supported format");

					if (ret){
						ret.fatalError = !typeStack.Push(td);
						REPORT_ERROR(ret,"failed to push type into stack");
					}

					if (ret){
						matchOutput = true;
					}
				}
			} else

			// PROCESS WRITE command
			// find_variable(parameter1) on outputs
			//    mark variable as already written
			// PUSH variable.type --> TypeStack
			// matchOutput = true;
			// assign code2 to address of variable
			if (command == writeToken){
				ret.invalidOperation = !hasParameter1;
				COMPOSITE_REPORT_ERROR(ret,writeToken," without variable name");

				VariableInformation *variableInformation;
				if (ret){
					ret = FindOutputVariable(parameter1,variableInformation);
					COMPOSITE_REPORT_ERROR(ret,"output variable ",parameter1, " not found");
				}

				TypeDescriptor td;
				if (ret){
					td = variableInformation->type;
					ret.unsupportedFeature = !td.IsNumericType();
					COMPOSITE_REPORT_ERROR(ret,"variable ",parameter1, "does not have a numeric supported format");
				}

				if (ret){
//printf("Stack[%i].Push(%x) -->",typeStack.GetSize(),td.all);
					ret.fatalError = !typeStack.Push(td);
					REPORT_ERROR(ret,"failed to push type into stack");
//printf("Stack[%i] \n",typeStack.GetSize());
				}

				if (ret){
					matchOutput = true;
					code2 = variableInformation->location;
					variableInformation->variableUsed = true;
				}

			} else

			// PROCESS READ command
			// find_variable(parameter1)
			//    search first on outputs if already written
			//    search then on inputs
			// PUSH variable.type --> TypeStack
			// matchOutput = true;
			// assign code2 to address of variable
			if (command == readToken){
				ret.invalidOperation = !hasParameter1;
				COMPOSITE_REPORT_ERROR(ret,readToken," without variable name");

				VariableInformation *variableInformation;
				if (ret){
					// try find an output variable with this name
					ret = FindOutputVariable(parameter1,variableInformation);
					if (ret){
						// not set yet - cannot use
						ret.notCompleted = (variableInformation->variableUsed != true);
					}
					// try to see if there is an input variable
					if (!ret){
						ret = FindInputVariable(parameter1,variableInformation);
						COMPOSITE_REPORT_ERROR(ret,"input variable ",parameter1, " not found");
					}
				}

				TypeDescriptor td;
				if (ret){
					td = variableInformation->type;
					ret.unsupportedFeature = !td.IsNumericType();
					COMPOSITE_REPORT_ERROR(ret,"variable ",parameter1, "does not have a numeric supported format");
				}
//printf("read %s type = %x  type = %x\n",variableInformation->name.GetList(),td.all,variableInformation->type.all);
				if (ret){
					ret.fatalError = !typeStack.Push(td);
					REPORT_ERROR(ret,"failed to push type into stack");
				}

				if (ret){
					matchOutput = true;
					code2 = variableInformation->location;
				}

			} else

			// PROCESS CONST command
			// PUSH type(parameter1) --> TypeStack
			// matchOutput = true;
			// create AnyType and convert constant from string to variable memory
			// assign code2 to address of constant
			// command = READ
			if (command == constToken){
				bool hasParameter2 = (parameter2.GetSize()> 0);

				ret.invalidOperation = !hasParameter1 || !hasParameter2;
				COMPOSITE_REPORT_ERROR(ret,constToken," without type name and value");

				// transform the type name into a TypeDescriptor
				// check it is one of the supported types
				TypeDescriptor td;
				if (ret){
					td = TypeDescriptor(parameter1);
					ret.unsupportedFeature = !td.IsNumericType();
					COMPOSITE_REPORT_ERROR(ret,"type ",parameter1, " is not a numeric supported format");
				}

				// convert string to number and save value into memory
				if (ret){
					//nextConstantAddress
					AnyType src(parameter2);
					AnyType dest(td,&variablesMemoryPtr[nextConstantAddress]);
					ret = src.CopyTo(dest);
					REPORT_ERROR(ret,"CopyTo failed ");
				}

				if (ret){
					ret.fatalError = !typeStack.Push(td);
					REPORT_ERROR(ret,"failed to push type into stack");
				}

				if (ret){
					matchOutput = true;
					code2 = nextConstantAddress;
					nextConstantAddress += ByteSizeToDataMemorySize(td.StorageSize());
					// the actual command is a READ from the constant area
					command = readToken;
				}
			}

			CodeMemoryElement code = 0;
			if (ret){
				ret.unsupportedFeature = !FindPCodeAndUpdateTypeStack(code,command,typeStack,matchOutput,dataStackSize);
				if (!ret){
					DynamicCString typeList;
					CStringTool cst = typeList();
					uint32 n2scan = 2;
					if (matchOutput) {
						n2scan++;
					}
					cst.Append('[');
					for(uint32 index = 0;index < n2scan;index++){
						if (index > 0){
							cst.Append('|');
						}
						TypeDescriptor td;
						typeStack.Peek(index,td);
						td.ToString(cst);
					}
					cst.Append(']');
					COMPOSITE_REPORT_ERROR(ret,"command ",command, "(",typeList,") not found");
				}
//printf("after %s %i elements in dataStack\n ",command.GetList(),dataStackSize);
			}

			// finally compile!
			if (ret){
				// update stackSize
				if (dataStackSize > maxDataStackSize){
					maxDataStackSize = dataStackSize;
				}

				ret.fatalError = !codeMemory.Add(code);
				REPORT_ERROR(ret,"failed to add instruction to code");

				if (ret && (code2 != TypeCharacteristics<CodeMemoryElement>::MaxValue())){
					ret.fatalError = !codeMemory.Add(code2);
					REPORT_ERROR(ret,"failed to add instruction to code");
				}
			}
		}
	}

	// final checks
	if (ret){

		// assign the code pointer
		codeMemoryPtr = codeMemory.GetAllocatedMemoryConst();

		// size the stack
		stack.SetSize(maxDataStackSize);
		stackPtr = static_cast<DataMemoryElement*>(stack.GetDataPointer());

		variablesMemoryPtr = static_cast<DataMemoryElement *>(dataMemory.GetDataPointer());
	}

	// check that the TypeStack is empty
	if (ret){
		ret.internalSetupError = (typeStack.GetSize() > 0);
		COMPOSITE_REPORT_ERROR(ret,"operation sequence is incomplete - ",typeStack.GetSize()," data left in stack");
	}

	return ret;
}

/**
 * expands the variableInformation into a readable text
 * if more pCode is required it will get it from context
 */
ErrorManagement::ErrorType Context::FunctionRecord2String(FunctionRecord &functionInformation,CStringTool &cst,bool peekOnly){

	 ErrorManagement::ErrorType ret;


	if ((functionInformation.name == readToken) || (functionInformation.name == writeToken)){
		CodeMemoryElement pCode2 = GetPseudoCode();
		if (peekOnly){
			codeMemoryPtr--;
		}

		Context::VariableInformation *vi;
		ret = FindVariable(pCode2,vi);
		COMPOSITE_REPORT_ERROR(ret,"No variable or constant @ ",pCode2);

		if (ret){
			if (pCode2 < startOfVariables){
				// Converts the value to a string
				DynamicCString value;
				AnyType dest(value);
				AnyType src(vi->type,&variablesMemoryPtr[pCode2]);
				ret = src.CopyTo(dest);
				REPORT_ERROR(ret,"CopyTo failed ");
				if (ret){
					cst.Append(value);
				}
			} else {
				cst.Append(vi->name);
			}
		}
	}

	cst.Append('(');
	for(uint32 i=0;(i<functionInformation.numberOfInputs) && ret;i++){
		if (i!=0) {
			cst.Append(',');
		}
		ret.fatalError = !functionInformation.types[i].ToString(cst);
	}
	if (functionInformation.numberOfOutputs > 0){
		cst.Append(" => ");
	}
	for(uint32 i=0;(i<functionInformation.numberOfOutputs) && ret;i++){
		if (i!=0) {
			cst.Append(',');
		}
		ret.fatalError = !functionInformation.types[i+functionInformation.numberOfInputs].ToString(cst);
	}
	cst.Append(')');

	return ret;

}


ErrorManagement::ErrorType Context::Execute(executionMode mode,StreamI *debugStream,uint32 step){

	stackPtr = static_cast<DataMemoryElement*>(stack.GetDataPointer());

	codeMemoryPtr = codeMemory.GetAllocatedMemoryConst();
	CodeMemoryAddress codeMaxIndex  = codeMemory.GetSize();
	const CodeMemoryElement *codeMemoryMaxPtr = codeMemoryPtr + codeMaxIndex;

	variablesMemoryPtr = static_cast<DataMemoryElement *>(dataMemory.GetDataPointer());
	runtimeError = ErrorManagement::ErrorType(true);

	switch (mode){
	case fastMode:{
		while(codeMemoryPtr < codeMemoryMaxPtr){
			CodeMemoryElement pCode = GetPseudoCode();
			functionRecords[pCode].function(*this);
		}
	}break;
	case safeMode:{
		DataMemoryElement *stackMinPtr = stackPtr;
		DataMemoryElement *stackMaxPtr = stackPtr + stack.GetNumberOfElements();
		while ((codeMemoryPtr < codeMemoryMaxPtr) /*&& (runtimeError.ErrorsCleared())*/){
			CodeMemoryElement pCode = GetPseudoCode();
			functionRecords[pCode].function(*this);
			// note that the syackPtr will reach the max value - as it points to the next value to write
			runtimeError.outOfRange = ((stackPtr > stackMaxPtr) ||  (stackPtr < stackMinPtr));
			COMPOSITE_REPORT_ERROR(runtimeError,"stack over/under flow ", (int64)(stackPtr-stackMinPtr), " [0 - ", (int64)(stackMaxPtr- stackMinPtr), "]");
		}
		runtimeError.notCompleted = (codeMemoryPtr < codeMemoryMaxPtr);
		REPORT_ERROR(runtimeError,"code execution interrupted");
	}break;
	case debugMode:
	default:{
		if (debugStream == NULL_PTR(StreamI *)){
			runtimeError.parametersError = true;
			REPORT_ERROR(runtimeError,"debugMode requested with debugStream set to NULL");
		} else {
			DynamicCString debugMessage;
			CStringTool cst = debugMessage();

			int64 stackOffset = stackPtr - static_cast<DataMemoryElement*>(stack.GetDataPointer());
			int64 codeOffset  = codeMemoryPtr - codeMemory.GetAllocatedMemoryConst();
			cst.Append("stackPtr: ").Append(stackOffset).Append(" codePtr: ").Append(codeOffset).Append("\n");
			printf ("%s",debugMessage.GetList());

			while(codeMemoryPtr < codeMemoryMaxPtr){
				CodeMemoryElement pCode = GetPseudoCode();

				// show update info
				cst.SetSize(0);
     			cst.Append(functionRecords[pCode].name).Append(' ');
				FunctionRecord2String(functionRecords[pCode],cst,true);
				cst.Append('\n');

				// executes code
				functionRecords[pCode].function(*this);

				int64 stackOffset = stackPtr - static_cast<DataMemoryElement*>(stack.GetDataPointer());
				int64 codeOffset  = codeMemoryPtr - codeMemory.GetAllocatedMemoryConst();
				cst.Append("stackPtr: ").Append(stackOffset).Append(" codePtr: ").Append(codeOffset).Append("\n");

				uint32 size = debugMessage.GetSize();
				debugStream->Write(debugMessage.GetList(),size);
			}
		}
	}
	}

	if (stackPtr != static_cast<DataMemoryElement*>(stack.GetDataPointer())){
		runtimeError.internalSetupError = true;
		int64 offset = stackPtr - static_cast<DataMemoryElement*>(stack.GetDataPointer());
		COMPOSITE_REPORT_ERROR(runtimeError,"stack pointer not back to origin : ",offset, " elements left");
	}

	return runtimeError;
}

ErrorManagement::ErrorType Context::DeCompile(DynamicCString &RPNCode){
	ErrorManagement::ErrorType ret ;

	codeMemoryPtr = codeMemory.GetAllocatedMemoryConst();
	CodeMemoryAddress codeMaxIndex  = codeMemory.GetSize();
	const CodeMemoryElement *codeMemoryMaxPtr = codeMemoryPtr + codeMaxIndex;

	variablesMemoryPtr = static_cast<DataMemoryElement *>(dataMemory.GetDataPointer());

	CStringTool cst = RPNCode();

	while((codeMemoryPtr < codeMemoryMaxPtr) && ret){
		CodeMemoryElement pCode = GetPseudoCode();
		cst.Append(functionRecords[pCode].name).Append(' ');

		ret = FunctionRecord2String(functionRecords[pCode],cst);

		cst.Append('\n');
	}

	return ret;
}


/***********************************************************************************************/

/**
 * to register a function
 */
void RegisterFunction(const FunctionRecord &record);

/**
 * generates boiler plate code to register a function
 */
#define REGISTER_PCODE_FUNCTION(name,subName,nInputs,nOutputs,function,...)\
	static const TypeDescriptor name ## subName ## _FunctionTypes[] = {__VA_ARGS__}; \
	static const FunctionRecord name ## subName ## _FunctionRecord={#name,nInputs,nOutputs,name ## subName ## _FunctionTypes,&function}; \
	static class name ## subName ## RegisterClass { \
	public: name ## subName ## RegisterClass(){\
			RegisterFunction(name ## subName ## _FunctionRecord);\
		}\
	} name ## subName ## RegisterClassInstance;

template <typename T> void Read(Context &context){
	CodeMemoryElement index;
	index = context.GetPseudoCode();
	context.Push(context.Variable<T>(index));
}

template <typename T> void Write(Context &context){
	CodeMemoryElement index;
	index = context.GetPseudoCode();
	context.Pop(context.Variable<T>(index));
}

template <typename T> void Duplication(Context &context){
	T var;
	context.Peek(var);
	context.Push(var);
}


REGISTER_PCODE_FUNCTION(DUP,double,1,2,Duplication<float64>,Float64Bit,Float64Bit,Float64Bit)
REGISTER_PCODE_FUNCTION(DUP,float,1,2,Duplication<float32>,Float32Bit,Float32Bit,Float32Bit)
REGISTER_PCODE_FUNCTION(DUP,uint64,1,2,Duplication<uint64>,UnsignedInteger64Bit,UnsignedInteger64Bit,UnsignedInteger64Bit)
REGISTER_PCODE_FUNCTION(DUP,int64,1,2,Duplication<int64>,SignedInteger64Bit,SignedInteger64Bit,SignedInteger64Bit)
REGISTER_PCODE_FUNCTION(DUP,uint32,1,2,Duplication<uint32>,UnsignedInteger32Bit,UnsignedInteger32Bit,UnsignedInteger32Bit)
REGISTER_PCODE_FUNCTION(DUP,int32,1,2,Duplication<int32>,SignedInteger32Bit,SignedInteger32Bit,SignedInteger32Bit)
REGISTER_PCODE_FUNCTION(DUP,uint16,1,2,Duplication<uint16>,UnsignedInteger16Bit,UnsignedInteger16Bit,UnsignedInteger16Bit)
REGISTER_PCODE_FUNCTION(DUP,int16,1,2,Duplication<int16>,SignedInteger16Bit,SignedInteger16Bit,SignedInteger16Bit)
REGISTER_PCODE_FUNCTION(DUP,uint8,1,2,Duplication<uint8>,UnsignedInteger8Bit,UnsignedInteger8Bit,UnsignedInteger8Bit)
REGISTER_PCODE_FUNCTION(DUP,int8,1,2,Duplication<int8>,SignedInteger8Bit,SignedInteger8Bit,SignedInteger8Bit)

REGISTER_PCODE_FUNCTION(READ,double,0,1,Read<float64>,Float64Bit)
REGISTER_PCODE_FUNCTION(READ,float,0,1,Read<float32>,Float32Bit)
REGISTER_PCODE_FUNCTION(READ,uint64,0,1,Read<uint64>,UnsignedInteger64Bit)
REGISTER_PCODE_FUNCTION(READ,int64,0,1,Read<int64>,SignedInteger64Bit)
REGISTER_PCODE_FUNCTION(READ,uint32,0,1,Read<uint32>,UnsignedInteger32Bit)
REGISTER_PCODE_FUNCTION(READ,int32,0,1,Read<int32>,SignedInteger32Bit)
REGISTER_PCODE_FUNCTION(READ,uint16,0,1,Read<uint16>,UnsignedInteger16Bit)
REGISTER_PCODE_FUNCTION(READ,int16,0,1,Read<int16>,SignedInteger16Bit)
REGISTER_PCODE_FUNCTION(READ,uint8,0,1,Read<uint8>,UnsignedInteger8Bit)
REGISTER_PCODE_FUNCTION(READ,int8,0,1,Read<int8>,SignedInteger8Bit)

REGISTER_PCODE_FUNCTION(WRITE,double,1,0,Write<float64>,Float64Bit,Float64Bit)
REGISTER_PCODE_FUNCTION(WRITE,float,1,0,Write<float32> ,Float32Bit,Float32Bit)
REGISTER_PCODE_FUNCTION(WRITE,uint64,1,0,Write<uint64> ,UnsignedInteger64Bit,UnsignedInteger64Bit)
REGISTER_PCODE_FUNCTION(WRITE,int64,1,0,Write<int64>   ,SignedInteger64Bit  ,SignedInteger64Bit  )
REGISTER_PCODE_FUNCTION(WRITE,uint32,1,0,Write<uint32> ,UnsignedInteger32Bit,UnsignedInteger32Bit)
REGISTER_PCODE_FUNCTION(WRITE,int32,1,0,Write<int32>   ,SignedInteger32Bit  ,SignedInteger32Bit  )
REGISTER_PCODE_FUNCTION(WRITE,uint16,1,0,Write<uint16> ,UnsignedInteger16Bit,UnsignedInteger16Bit)
REGISTER_PCODE_FUNCTION(WRITE,int16,1,0,Write<int16>   ,SignedInteger16Bit  ,SignedInteger16Bit  )
REGISTER_PCODE_FUNCTION(WRITE,uint8,1,0,Write<uint8>   ,UnsignedInteger8Bit ,UnsignedInteger8Bit )
REGISTER_PCODE_FUNCTION(WRITE,int8,1,0,Write<int8>     ,SignedInteger8Bit   ,SignedInteger8Bit   )

#define REGISTER_CAST_FUNCTION(name,type1,type2,nInputs,nOutputs,function,...)\
	static const TypeDescriptor name ## type1 ## type2 ## _FunctionTypes[] = {__VA_ARGS__}; \
	static const FunctionRecord name ## type1 ## type2 ## _FunctionRecord={#name,nInputs,nOutputs,name ## type1 ## type2 ## _FunctionTypes,&function<type1,type2>}; \
	static class name ## type1 ## type2 ## RegisterClass { \
	public: name ## type1 ## type2 ## RegisterClass(){\
			RegisterFunction(name ## type1 ## type2 ## _FunctionRecord);\
		}\
	} name ## type1 ## type2 ## RegisterClassInstance;

template <typename T1,typename T2> void Casting(Context &context){
	T1 x1;
	T2 x2;
	bool ret;
	context.Peek(x1);
	ret = SafeNumber2Number(x1,x2);
	context.Push(x2);
	if (!ret){
		context.GetErrorFlag().outOfRange = true;
	}
}

REGISTER_CAST_FUNCTION(CAST,float64,float64,1,1,Casting,Float64Bit          ,Float64Bit)
REGISTER_CAST_FUNCTION(CAST,float64,float32,1,1,Casting,Float64Bit          ,Float32Bit)
REGISTER_CAST_FUNCTION(CAST,float64,uint64 ,1,1,Casting,UnsignedInteger64Bit,UnsignedInteger64Bit)
REGISTER_CAST_FUNCTION(CAST,float64,int64  ,1,1,Casting,SignedInteger64Bit  ,SignedInteger64Bit  )
REGISTER_CAST_FUNCTION(CAST,float64,uint32 ,1,1,Casting,UnsignedInteger32Bit,UnsignedInteger32Bit)
REGISTER_CAST_FUNCTION(CAST,float64,int32  ,1,1,Casting,SignedInteger32Bit  ,SignedInteger32Bit  )
REGISTER_CAST_FUNCTION(CAST,float64,uint16 ,1,1,Casting,UnsignedInteger16Bit,UnsignedInteger16Bit)
REGISTER_CAST_FUNCTION(CAST,float64,int16  ,1,1,Casting,SignedInteger16Bit  ,SignedInteger16Bit  )
REGISTER_CAST_FUNCTION(CAST,float64,uint8  ,1,1,Casting,UnsignedInteger8Bit ,UnsignedInteger8Bit )
REGISTER_CAST_FUNCTION(CAST,float64,int8   ,1,1,Casting,SignedInteger8Bit   ,SignedInteger8Bit   )

#define REGISTER_OPERATOR(name,oper,fname)	    							        \
		template <typename T> void function ## fname ## ication (Context &context){ \
			T x1,x2,x3;													     		\
			context.Pop(x1);                                                        \
			context.Pop(x2);                                                        \
			x3 = x1 oper x2;                                                        \
			context.Push(x3);                                                       \
		}                                                                           \
		REGISTER_PCODE_FUNCTION(name,float64,2,1,function ## fname ## ication <float64>,Float64Bit,Float64Bit,Float64Bit)  \
		REGISTER_PCODE_FUNCTION(name,float32,2,1,function ## fname ## ication <float32>,Float32Bit,Float32Bit,Float32Bit)  \
		REGISTER_PCODE_FUNCTION(name,uint64 ,2,1,function ## fname ## ication <uint64> ,UnsignedInteger64Bit,UnsignedInteger64Bit,UnsignedInteger64Bit) \
		REGISTER_PCODE_FUNCTION(name,int64  ,2,1,function ## fname ## ication <int64>  ,SignedInteger64Bit,SignedInteger64Bit,SignedInteger64Bit)       \
		REGISTER_PCODE_FUNCTION(name,uint32 ,2,1,function ## fname ## ication <uint32> ,UnsignedInteger32Bit,UnsignedInteger32Bit,UnsignedInteger32Bit) \
		REGISTER_PCODE_FUNCTION(name,int32  ,2,1,function ## fname ## ication <int32>  ,SignedInteger32Bit,SignedInteger32Bit,SignedInteger32Bit)       \
		REGISTER_PCODE_FUNCTION(name,uint16 ,2,1,function ## fname ## ication <uint16> ,UnsignedInteger16Bit,UnsignedInteger16Bit,UnsignedInteger16Bit) \
		REGISTER_PCODE_FUNCTION(name,int16  ,2,1,function ## fname ## ication <int16>  ,SignedInteger16Bit,SignedInteger16Bit,SignedInteger16Bit)       \
		REGISTER_PCODE_FUNCTION(name,uint8  ,2,1,function ## fname ## ication <uint8>  ,UnsignedInteger8Bit,UnsignedInteger8Bit,UnsignedInteger8Bit)    \
		REGISTER_PCODE_FUNCTION(name,int8   ,2,1,function ## fname ## ication <int8>   ,SignedInteger8Bit,SignedInteger8Bit,SignedInteger8Bit)

REGISTER_OPERATOR(ADD, + ,Addition)
REGISTER_OPERATOR(SUB, - ,Subtract)
REGISTER_OPERATOR(MUL, * ,Multipl)
REGISTER_OPERATOR(DIV, / ,Division)


#define REGISTER_1_FUNCTION(name,fname)	    							\
		template <typename T> void function ## fname ## ication (Context &context){ \
			T x,res;													     		\
			context.Pop(x);                                                         \
			res = fname (x);                                                        \
			context.Push(res);                                                      \
		}                                                                           \
		REGISTER_PCODE_FUNCTION(name,float32,1,1,function ## fname ## ication <float32>,Float32Bit,Float32Bit)  \
		REGISTER_PCODE_FUNCTION(name,float64,1,1,function ## fname ## ication <float64>,Float64Bit,Float64Bit)

REGISTER_1_FUNCTION(SIN,sin)
REGISTER_1_FUNCTION(COS,cos)
REGISTER_1_FUNCTION(TAN,tan)
REGISTER_1_FUNCTION(EXP,exp)
REGISTER_1_FUNCTION(LOG,log)
REGISTER_1_FUNCTION(LOG10,log10)

#define REGISTER_2_FUNCTION(name,fname)	    							\
		template <typename T> void function ## fname ## ication (Context &context){ \
			T x1,x2,res;	     										     		\
			context.Pop(x1);                                                        \
			context.Pop(x2);                                                        \
			res = fname (x1,x2);                                                    \
			context.Push(res);                                                      \
		}                                                                           \
		REGISTER_PCODE_FUNCTION(name,float32,2,1,function ## fname ## ication <float32>,Float32Bit,Float32Bit,Float32Bit)  \
		REGISTER_PCODE_FUNCTION(name,float64,2,1,function ## fname ## ication <float64>,Float64Bit,Float64Bit,Float64Bit)

REGISTER_2_FUNCTION(POW,pow)

#define REGISTER_COMPARE_OPERATOR(name,oper,fname)	    						    \
		template <typename T> void function ## fname ## ication (Context &context){ \
			T x1,x2;													     		\
			bool ret;													     		\
			context.Pop(x1);                                                        \
			context.Pop(x2);                                                        \
			ret = x1 oper x2;                                                       \
			context.Push(ret);                                                      \
		}                                                                           \
		REGISTER_PCODE_FUNCTION(name,float64,2,1,function ## fname ## ication <float64>,Float64Bit,Float64Bit,UnsignedInteger8Bit)  \
		REGISTER_PCODE_FUNCTION(name,float32,2,1,function ## fname ## ication <float32>,Float32Bit,Float32Bit,UnsignedInteger8Bit)  \
		REGISTER_PCODE_FUNCTION(name,uint64 ,2,1,function ## fname ## ication <uint64> ,UnsignedInteger64Bit,UnsignedInteger64Bit,UnsignedInteger8Bit) \
		REGISTER_PCODE_FUNCTION(name,int64  ,2,1,function ## fname ## ication <int64>  ,SignedInteger64Bit,SignedInteger64Bit,UnsignedInteger8Bit)       \
		REGISTER_PCODE_FUNCTION(name,uint32 ,2,1,function ## fname ## ication <uint32> ,UnsignedInteger32Bit,UnsignedInteger32Bit,UnsignedInteger8Bit) \
		REGISTER_PCODE_FUNCTION(name,int32  ,2,1,function ## fname ## ication <int32>  ,SignedInteger32Bit,SignedInteger32Bit,UnsignedInteger8Bit)       \
		REGISTER_PCODE_FUNCTION(name,uint16 ,2,1,function ## fname ## ication <uint16> ,UnsignedInteger16Bit,UnsignedInteger16Bit,UnsignedInteger8Bit) \
		REGISTER_PCODE_FUNCTION(name,int16  ,2,1,function ## fname ## ication <int16>  ,SignedInteger16Bit,SignedInteger16Bit,UnsignedInteger8Bit)       \
		REGISTER_PCODE_FUNCTION(name,uint8  ,2,1,function ## fname ## ication <uint8>  ,UnsignedInteger8Bit,UnsignedInteger8Bit,UnsignedInteger8Bit)    \
		REGISTER_PCODE_FUNCTION(name,int8   ,2,1,function ## fname ## ication <int8>   ,SignedInteger8Bit,SignedInteger8Bit,UnsignedInteger8Bit)

REGISTER_COMPARE_OPERATOR(EQ, == ,Equal)
REGISTER_COMPARE_OPERATOR(NEQ, != ,Different)
REGISTER_COMPARE_OPERATOR(GT, > ,Greater)
REGISTER_COMPARE_OPERATOR(LT, < ,Smaller)
REGISTER_COMPARE_OPERATOR(GTE, >= ,Great)
REGISTER_COMPARE_OPERATOR(LTE, <= ,Small)

#define REGISTER_LOGICAL_OPERATOR(name,oper,fname)	    						    \
		void function ## fname ## ication (Context &context){                       \
			bool x1,x2,ret;													        \
			context.Pop(x1);                                                        \
			context.Pop(x2);                                                        \
			ret = x1 oper x2;                                                       \
			context.Push(ret);                                                      \
		}                                                                           \
      	REGISTER_PCODE_FUNCTION(name,boolean,2,1,function ## fname ## ication,UnsignedInteger8Bit,UnsignedInteger8Bit,UnsignedInteger8Bit)

REGISTER_LOGICAL_OPERATOR(AND, && ,And)
REGISTER_LOGICAL_OPERATOR(OR, || ,Or)
REGISTER_LOGICAL_OPERATOR(XOR, ^ ,xor)



} //PseudoCode
} //MARTe