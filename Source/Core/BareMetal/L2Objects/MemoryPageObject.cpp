/**
 * @file MemoryPageObject.cpp
 * @brief Header file for class AnyType
 * @date 3 Feb 2018
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

#include "MemoryPageObject.h"

namespace MARTe{

/**
 * constructor
 */
MemoryPageObject::MemoryPageObject(){	}

/**
 * destructor
 */
MemoryPageObject::~MemoryPageObject(){	}

void MemoryPageObject::Setup(TypeDescriptor dataDescriptorIn,CCString modifiers,const void* const dataPointerIn,MemoryPage &pagesToTransfer){
	anyType.Setup(dataDescriptorIn,modifiers,dataPointerIn);

	mp.Copy(pagesToTransfer);
}

/**
 * @brief The only interface provided by an AnyObject is the ability to provide its data via an AnyType.
 * @return a valid AnyType that describes the content of this object and allows read only access to its content
 */
MemoryPageObject::operator AnyType(){
	return anyType;
}


CLASS_REGISTER(MemoryPageObject, "1.0")




} //MARTe