/**
 * @file ObjectBuilderTest.cpp
 * @brief Source file for class ObjectBuilderTest
 * @date 17/giu/2016
 * @author pc
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
 * the class ObjectBuilderTest (public, protected, and private). Be aware that some 
 * methods, such as those inline could be defined on the header file, instead.
 */

/*---------------------------------------------------------------------------*/
/*                         Standard header includes                          */
/*---------------------------------------------------------------------------*/

/*---------------------------------------------------------------------------*/
/*                         Project header includes                           */
/*---------------------------------------------------------------------------*/
#include "ClassRegistryItemT.h"
#include "ObjectBuilderTest.h"
#include "ObjectTestHelper.h"
#include "StringHelper.h"
/*---------------------------------------------------------------------------*/
/*                           Static definitions                              */
/*---------------------------------------------------------------------------*/
class NotBuildableAgainObj: public Object{
public:
    CLASS_REGISTER_DECLARATION();
};

CLASS_REGISTER(NotBuildableAgainObj,"1.0");
/*---------------------------------------------------------------------------*/
/*                           Method definitions                              */
/*---------------------------------------------------------------------------*/

bool ObjectBuilderTest::TestConstructor() {
    ObjectBuilderT<NotBuildableAgainObj> build;
    NotBuildableAgainObj *obj = (NotBuildableAgainObj *)build.Build(GlobalObjectsDatabase::Instance()->GetStandardHeap());
    if (obj == NULL) {
        return false;
    }
    delete obj;
    return true;
}

bool ObjectBuilderTest::TestBuild() {
    return TestConstructor();
}
