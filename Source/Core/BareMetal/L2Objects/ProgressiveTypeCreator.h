/**
 * @file ProgressiveTypeCreator.h
 * @brief Header file for class AnyType
 * @date 5 Jan 2018
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

#ifndef PROGRESSIVETYPECREATOR_H_
#define PROGRESSIVETYPECREATOR_H_


/*---------------------------------------------------------------------------*/
/*                        Standard header includes                           */
/*---------------------------------------------------------------------------*/

/*---------------------------------------------------------------------------*/
/*                        Project header includes                            */
/*---------------------------------------------------------------------------*/

#include "ErrorManagement.h"
#include "TypeDescriptor.h"
#include "TypeConversionOperatorI.h"
#include "StaticListHolder.h"
#include "TypeCharacteristics.h"
#include "ReferenceT.h"
#include "MemoryPage.h"


/*---------------------------------------------------------------------------*/
/*                          Forward declarations                             */
/*---------------------------------------------------------------------------*/

/*---------------------------------------------------------------------------*/
/*                           Class declaration                               */
/*---------------------------------------------------------------------------*/



namespace MARTe{

class MemoryPage;
/**
 * @brief a growable container of uint32
 */
class SizeStack: protected StaticListHolder{

public:
	/**
	 *
	 */
	inline SizeStack();
	/**
	 *
	 */
	inline bool Push(uint32 size);
	/**
	 *
	 */
	inline uint32 numberOfElements();
	/**
	 *
	 */
	inline uint32 operator[](uint32 index);

	/**
	 *
	 */
	inline void Clean();

};


/**
 * @brief A tool to create AnyObjectI that contains scalar/vectors/arrays of fixed size objects
 */
class DLL_API ProgressiveTypeCreator{

public:
	/**
	 * @brief Creates the object selecting the type to convert to and the default PageSize
	 */
	ProgressiveTypeCreator(uint32 pageSizeIn );

	/**
	 * @deletes the object and any memory allocated in the pages
	 */
	virtual ~ProgressiveTypeCreator();

	/**
	 * @brief first call - sets the output type
	 */
	ErrorManagement::ErrorType Start(TypeDescriptor typeIn);

	/**
	 * @brief frees memory and readies the system for Start(). works on any state
	 */
	void Clean();

	/**
	 * @adds an element. It will convert the string to the specified type. Callable after Start()
	 */
	ErrorManagement::ErrorType AddElement(CCString typeStringRepresentation);

	/**
	 * @brief Marks the end of a row of elements. Callable after Start()
	 */
	ErrorManagement::ErrorType EndVector();

	/**
	 * @brief Marks the end of the object construction. Callable after Start()
	 */
	ErrorManagement::ErrorType End();

	/**
	 * @brief Allows retrieving the Object that has been built.
	 * Can only be done after End has been called. Callable after End()
	 */
	ErrorManagement::ErrorType GetReference(Reference &x);

	/**
	 * @brief returns Actual page size after growth etc...
	 */
	inline uint32 DefaultPageSize();

protected:

	enum PTCState {
		// no data yet
		started 			= 0x0000,
		// one data
		scalar 				= 0x0001,
		// more than one
		vector 				= 0x0002,
		// end of first vector reached
		vectorEnd 			= 0x0003,
		// nth vector data in. all vectors same size so far
		matrixRow 	    	= 0x0004,
		// end of nth vector. all vectors same size
		matrixRowEnd    	= 0x0005,
		// nth vector data in. at least one vector has different size
		sparseMatrixRow		= 0x0006,
		// end of nth vector
		sparseMatrixRE 		= 0x0007,
		// finished as a scalar
		finishedS 			= 0x00E0,
		// finished as a vector
		finishedV 			= 0x00E1,
		// finished as a matrix
		finishedM 			= 0x00E2,
		// finished as a sparse matrix
		finishedSM 			= 0x00E3,
		//
		notStarted          = 0x00F0,
		// some error encountered
		error 				= 0x00FF,
		//
		mask				= 0x00F0
	};

	ErrorManagement::ErrorType CheckMemoryFixedSizeEl(bool newRow);

	ErrorManagement::ErrorType CompleteFixedSizeEl(uint32 &auxSize,void *&auxPtr);

	ErrorManagement::ErrorType CheckMemoryStringEl(uint32 neededSize);

	ErrorManagement::ErrorType CompleteStringEl(void *&dataPtr,uint32 &auxSize,void *&auxPtr);

	ErrorManagement::ErrorType PageGrow(uint32 amount);

	/**
	 * @brief Check if the current segment has enough space to store another vector.
	 * If there is no need (neededSize== 0) or if the space is not large enough
	 * close the current memory page segment
	 */
	ErrorManagement::ErrorType CheckAndClosePage(uint32 neededSize);
	/**
	 * @brief If pageSize is 0 allocate a new page of desired size
	 */
	ErrorManagement::ErrorType CheckAndNewPage(uint32 neededSize);
	/**
	 * @brief Check if the current segment has enough space to store another vector.
	 * If there is no need (neededSize== 0) or if the space is not large enough
	 * close the current memory page segment and open a new one
	 */
//	ErrorManagement::ErrorType CheckAndRenewPage(uint32 neededSize,uint32 newPageSize);

	/**
	 * @brief Core of implementation of GetObject
	 * called by GetObject
	 * if data is fragmented into multiple pages, it assumes that the start of data is in the first cell of the top page
	 * Therefore requires memory flipping.
	 * Would not support fragmented table of string pointers. (not generated currently)
	 */
	ErrorManagement::ErrorType GetReferencePrivate(Reference &x, void *dataPtr, void *auxPtr,uint32 auxSize);

	/**
	 * status within the progressive creation
	 */
	PTCState status;

	/**
	 * Any of the finished states
	 */
	inline bool Finished();

	/**
	 * Any of the started states
	 */
	inline bool Started();

	/**
	 * The size of the first vector and of all other vectors if status = matrix
	 */
	uint32 vectorSize;

	/**
	 * Number of rows in matrix
	 */
	uint32 matrixRowSize;

	/**
	 * Size of vector currently being scanned
	 */
	uint32 currentVectorSize;

	/**
	 * vectorSize*matrixRowSize or something less for sparse matrices
	 */
	uint64 numberOfElements;

	/**
	 * The object containing the memory
	 */
	MemoryPage page;

	/**
	 * Reference pageSize specified by user
	 */
	uint32  defaultPageSize;
	/**
	 *Actual page size after growth etc...
	 */
	uint32 	pageSize;
	/**
	 * Space left within pageSize
	 */
	uint32 	sizeLeft;
	/**
	 * Size of one object in bytes
	 */
	uint32 	objectSize;
	/**
	 * how much of pageSize was used
	 */
	uint32 pageWritePos;
	/**
	 * converts from string to type
	 */
	const TypeConversionOperatorI *converter;
	/**
	 * holds the size of each vectors
	 */
	SizeStack sizeStack;
	/**
	 * the type of the data;
	 */
	TypeDescriptor type;


	bool isString;

	/**
	 * > 0 if data spread across more than one page.
	 */
	uint32 numberOfPages;
};


/*---------------------------------------------------------------------------*/
/*                        Inline method definitions                          */
/*---------------------------------------------------------------------------*/

SizeStack::SizeStack():StaticListHolder(sizeof(uint32),64,0,0){
}

bool SizeStack::Push(uint32 size){
	return Add(&size);
}

uint32 SizeStack::numberOfElements(){
	return GetSize();
}

uint32 SizeStack::operator[](uint32 index){
	uint32 value=0;
	Peek(index,&value);
	return value;
}

void SizeStack::Clean(){
	StaticListHolder::Clean();
}


/**
 * Any of the finished states or error
 */
bool ProgressiveTypeCreator::Finished(){
	const uint32 mask 			= static_cast<uint32>(PTCState::mask);
	const uint32 finishedMask 	= static_cast<uint32>(PTCState::finishedS);
	uint32 statusAsInt = static_cast<uint32>(status);
	return ((statusAsInt & mask) == finishedMask);
}

/**
 * Any of the start,
 */
bool ProgressiveTypeCreator::Started(){
	const uint32 mask 			= static_cast<uint32>(PTCState::mask);
	const uint32 startedMask 	= static_cast<uint32>(PTCState::started);
	uint32 statusAsInt = static_cast<uint32>(status);
	return ((statusAsInt & mask) == startedMask);
}

inline uint32 ProgressiveTypeCreator::DefaultPageSize(){
	return defaultPageSize;
}


} // MARTe

#endif /* SOURCE_CORE_BAREMETAL_L2OBJECTS_PROGRESSIVETYPECREATOR_H_ */