/**
 * @file ToStringConversionFactory.cpp
 * @brief Header file for class AnyType
 * @date 11 Nov 2017
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
#include "TypeConversionFactoryI.h"
#include "TypeConversionManager.h"
#include "IOBuffer.h"
#include "IOBufferPrivate.h"
#include "StreamString.h"

namespace MARTe{


/*********************************************************************************************************/
/*                                                                                                       */
/*                                write WRAPPER OF IoBuffer                                              */
/*                                                                                                       */
/*********************************************************************************************************/

/**
 * @brief provides a generic interface between IOBuffer and arrays of character streams
 */
class IOBufferWrapper: public IOBuffer{

public:
	/**
	 * @brief interface to this stream
	 */
	virtual void Wrap(void *ptr)=0;

	/**
	 * @brief flush the IOBuffer
	 */
	bool Flush(){
		return NoMoreSpaceToWrite();
	}

	/**
	 * @brief constructor
	 */
	IOBufferWrapper(){
	    SetBufferReferencedMemory(&buffer[0],sizeof(buffer),0);
	}

	/**
	 * @brief switch to next stream
	 */
	virtual bool Next(){
		Flush();
	}

private:
	/**
	 * @brief buffer for the IOBuffer
	 */
	char buffer[64];
};

/**
 * @brief connects IOBuffer to a single Stream
 */
class IOBufferWrapperStream: public IOBufferWrapper{
public:
	/**
	 * @brief constructor
	 */
	IOBufferWrapperStream(): IOBufferWrapper(){
		stream = NULL;
	}

	/**
	 * @brief interfaces to the Stream
	 */
	virtual void Wrap(void *ptr){
		stream = reinterpret_cast<StreamI *>(ptr);
	}

	/**
	 * @brief flush the IOBuffer
	 */
	bool Flush(){
		return NoMoreSpaceToWrite();
	}

	/**
	 * @brief switch to next stream
	 */
	virtual bool Next(){
		Flush();
		// do not know how to skip to next object
		return false;
	}
protected:
	/** the stream */
	StreamI *stream;

	/**
	 * @brief dumps the IOBuffer to the Stream
	 * */
	virtual bool NoMoreSpaceToWrite() {
	    bool retval = false;
		if (stream != NULL) {
	        // no buffering!
	        if (Buffer() != NULL) {

	            // how much was written?
	            uint32 writeSize = UsedSize();
	            if (writeSize == 0u) {
	                retval = true;
	            }
	            // write
	            else {
	                if (stream->Write(Buffer(), writeSize)) {
	                    retval = true;
	                    Empty();
	                }
	                else {
	                    REPORT_ERROR(ErrorManagement::FatalError, "StreamToIOBuffer: Failed Write");
	                }
	            }
	        }
	    }
		return retval;
	}
};

/**
 * @brief connects IOBuffer to a SStrings
 */
class IOBufferWrapperSString: public IOBufferWrapperStream{
public:

	IOBufferWrapperSString():IOBufferWrapperStream(){
		ss = NULL_PTR(StreamString*);
	}

	virtual void Wrap(void *ptr){
		ss = reinterpret_cast<StreamString *>(ptr);
		stream = ss;
	}

	/**
	 * @brief switch to next stream
	 */
	virtual bool Next(){
		Flush();
		ss++;
		stream = ss;
		return true;
	}
protected:
	StreamString *ss;
};

/**
 * @brief connects IOBuffer to a DynamicCString
 */
class IOBufferDynStringWrapper: public IOBufferWrapper{
public:

	IOBufferDynStringWrapper(): IOBufferWrapper(){
		string = NULL;
	}

	virtual void Wrap(void *ptr){
		string = reinterpret_cast<DynamicCString *>(ptr);
	}

	/**
	 * @brief switch to next stream
	 */
	virtual bool Next(){
		Flush();
		string++;
		return true;
	}
protected:

	/**
	 * @brief dumps the IOBuffer to the Stream
	 * */
	virtual bool NoMoreSpaceToWrite() {
	    bool retval = false;
		if (string != NULL) {
	        // no buffering!
	        if (Buffer() != NULL) {

	            // how much was written?
	            uint32 writeSize = UsedSize();
	            if (writeSize == 0u) {
	                retval = true;
	            }
	            // write
	            else {
	                if (string->AppendN(Buffer(), writeSize)) {
	                    retval = true;
	                    Empty();
	                }
	                else {
	                    REPORT_ERROR(ErrorManagement::FatalError, "IOBufferDynStringWrapper: Failed Write");
	                }
	            }
	        }
	    }
		return retval;
	}
private:
	/**
	 * @brief pointer to array of DynamicCString
	 */
	DynamicCString *string;
};


/*********************************************************************************************************/
/*                                                                                                       */
/*                                TYPE CONVERSION OPERATORS                                              */
/*                                                                                                       */
/*********************************************************************************************************/


/**
 * @brief copies to strings
 */
class StringTCO: public TypeConversionOperatorI{
public:

	/**
	 * @brief constructor
	 */
	StringTCO(IOBufferWrapper *writerIn): writer(writerIn){
	}

	/**
	 * @brief destructor
	 */
	virtual  ~StringTCO(){
		delete writer;
	}
protected:
	/*
	 * @brief the writer mechanism
	 */
	IOBufferWrapper *writer;

};

/**
 * @brief copies integer to strings
 */
template <typename integerType>
class IntegerToStringTCO: public StringTCO{

public:

	/**
	 * @brief constructor
	 */
	IntegerToStringTCO(IOBufferWrapper *writerIn): StringTCO(writerIn){
	}

	/**
	 * @brief destructor
	 */
	virtual  ~IntegerToStringTCO(){}

	/**
	 * @brief data conversion method
	 */
	virtual ErrorManagement::ErrorType Convert(uint8 *dest, const uint8 *source,uint32 numberOfElements,const FormatDescriptor &fd) const{
		ErrorManagement::ErrorType  ok;
		if (writer != NULL){
			writer->Wrap(dest);
			uint32 ix = 0;
			const integerType *pIn = (reinterpret_cast<const integerType *>(source));
			ok.fatalError = IntegerToStream(*writer,*pIn++,fd);
			for (ix = 1;(ix<numberOfElements) && ok;ix++){
				ok.illegalOperation=writer->Next();
				if (ok){
					ok.fatalError = IntegerToStream(*writer,*pIn++,fd);
				}
			}
			writer->Flush();
		} else {
			ok.internalSetupError = true;
		}

		return ok;
	}

	/**
	 * @brief data conversion method
	 */
	virtual ErrorManagement::ErrorType Convert(uint8 *dest, const uint8 *source,uint32 numberOfElements) const{
		return Convert(dest,source,numberOfElements,format);
	}


};

/**
 * @brief copies bitset integers to strings
 */
class BitSetToStringTCO: public StringTCO{

public:

	/**
	 * @brief constructor
	 */
	BitSetToStringTCO(IOBufferWrapper *writerIn,uint8 byteSizeIn,uint8 numberBitShiftIn,uint8 numberBitSizeIn,bool isSignedIn): StringTCO(writerIn){
		numberBitShift = numberBitShiftIn;
		numberBitSize  = numberBitSizeIn;
		byteSize       = byteSizeIn;
		isSigned       = isSignedIn;
	}

	/**
	 * @brief destructor
	 */
	virtual  ~BitSetToStringTCO(){}

	/**
	 * @brief data conversion method
	 */
	virtual ErrorManagement::ErrorType Convert(uint8 *dest, const uint8 *source,uint32 numberOfElements,const FormatDescriptor &td) const{
		ErrorManagement::ErrorType  ok;
		writer->Wrap(dest);

		ok.fatalError = BitSetToStream(*writer,reinterpret_cast<uint32 const * >(source),numberBitShift,numberBitSize,isSigned,td);
		for (uint32 ix = 1; (ix < numberOfElements) && ok;ix++){
			ok.illegalOperation=writer->Next();
			source += byteSize;
			if (ok){
				ok.fatalError = BitSetToStream(*writer,reinterpret_cast<uint32 const * >(source),numberBitShift,numberBitSize,isSigned,td);
			}
		}
		writer->Flush();

		return ok;
	}

	/**
	 * @brief data conversion method
	 */
	virtual ErrorManagement::ErrorType Convert(uint8 *dest, const uint8 *source,uint32 numberOfElements) const{
		return Convert(dest,source,numberOfElements,format);
	}

private:
	/**
	 * @brief overall occupation of number in bytes
	 */
	uint8 byteSize;
	/**
	 * @brief bitshift of number
	 */
	uint8 numberBitShift;
	/**
	 * @brief bitsize of number
	 */
	uint8 numberBitSize;
	/**
	 * @brief signedness of number
	 */
	bool  isSigned;
};

/**
 * @brief copies bitset integers to strings
 */
class PointerToStringTCO: public StringTCO{

public:

	/**
	 * @brief constructor
	 */
	PointerToStringTCO(IOBufferWrapper *writerIn): StringTCO(writerIn){
	}

	/**
	 * @brief destructor
	 */
	virtual  ~PointerToStringTCO(){}

	/**
	 * @brief data conversion method
	 */
	virtual ErrorManagement::ErrorType Convert(uint8 *dest, const uint8 *source,uint32 numberOfElements) const{
		ErrorManagement::ErrorType  ok;
		writer->Wrap(dest);

		const void **src = reinterpret_cast<const void ** >(source);
		ok.fatalError = PointerToStream(*writer,*src);
		for (uint32 ix = 1; (ix < numberOfElements) && ok;ix++){
			ok.illegalOperation=writer->Next();
			src++;
			if (ok){
				ok.fatalError = PointerToStream(*writer,*src);
			}
		}

		writer->Flush();

		return ok;
	}

};

/**
 * @brief copies floats to strings
 */
template <typename floatType>
class FloatToStringTCO: public StringTCO{

public:

	/**
	 * @brief constructor
	 */
	FloatToStringTCO(IOBufferWrapper *writerIn): StringTCO(writerIn){
	}

	/**
	 * @brief destructor
	 */
	virtual  ~FloatToStringTCO(){}

	/**
	 * @brief data conversion method
	 */
	virtual ErrorManagement::ErrorType Convert(uint8 *dest, const uint8 *source,uint32 numberOfElements,const FormatDescriptor &fd) const{
		ErrorManagement::ErrorType  ok;
		writer->Wrap(dest);

		const floatType *src = reinterpret_cast<const floatType *>(source);
		ok.fatalError = FloatToStream(*writer,*src,fd);
		for (uint32 ix = 1; (ix < numberOfElements) && ok;ix++){
			ok.illegalOperation=writer->Next();
			src++;
			if (ok){
				ok.fatalError = FloatToStream(*writer,*src,fd);
			}
		}
		writer->Flush();

		return ok;
	}

	/**
	 * @brief data conversion method
	 */
	virtual ErrorManagement::ErrorType Convert(uint8 *dest, const uint8 *source,uint32 numberOfElements) const{
		return Convert(dest,source,numberOfElements,format);
	}
};

/**
 * @brief copies integer to strings
 */
class CCStringToStringTCO: public StringTCO{

public:

	/**
	 * @brief constructor
	 */
	CCStringToStringTCO(IOBufferWrapper *writerIn): StringTCO(writerIn){
	}

	/**
	 * @brief destructor
	 */
	virtual  ~CCStringToStringTCO(){}

	/**
	 * @brief data conversion method
	 */
	virtual ErrorManagement::ErrorType Convert(uint8 *dest, const uint8 *source,uint32 numberOfElements,const FormatDescriptor &fd) const{
		ErrorManagement::ErrorType  ok;
		writer->Wrap(dest);

		const CCString *src = reinterpret_cast<const CCString *>(source);
		ok.fatalError = PrintCCString(*writer,*src,fd);
		for (uint32 ix = 1; (ix < numberOfElements) && ok;ix++){
			ok.illegalOperation=writer->Next();
			src++;
			if (ok){
				ok.fatalError = PrintCCString(*writer,*src,fd);
			}
		}

		writer->Flush();

		return ok;
	}

	/**
	 * @brief data conversion method
	 */
	virtual ErrorManagement::ErrorType Convert(uint8 *dest, const uint8 *source,uint32 numberOfElements) const{
		return Convert(dest,source,numberOfElements,format);
	}
};

/**
 * @brief copies stream to strings
 */
class StreamToStringTCO: public StringTCO{

public:

	/**
	 * @brief constructor
	 */
	StreamToStringTCO(IOBufferWrapper *writerIn): StringTCO(writerIn){
	}

	/**
	 * @brief destructor
	 */
	virtual  ~StreamToStringTCO(){}

	/**
	 * @brief data conversion method
	 */
	virtual ErrorManagement::ErrorType Convert(uint8 *dest, const uint8 *source,uint32 numberOfElements,const FormatDescriptor &fd) const{
		ErrorManagement::ErrorType  ok;

		ok.unsupportedFeature = (numberOfElements!= 1);

		if (ok){
			writer->Wrap(dest);

			uint8 *srcc = const_cast<uint8 *>(source);
			StreamString *src = reinterpret_cast<StreamString *>(srcc);

			ok.fatalError = PrintStream(*writer,src,fd);
			for (uint32 ix = 1; (ix < numberOfElements) && ok;ix++){
				ok.illegalOperation=writer->Next();
				src++;
				if (ok){
					ok.fatalError = PrintStream(*writer,src,fd);
				}
			}

			writer->Flush();
		}

		return ok;
	}

	/**
	 * @brief data conversion method
	 */
	virtual ErrorManagement::ErrorType Convert(uint8 *dest, const uint8 *source,uint32 numberOfElements) const{
		return Convert(dest,source,numberOfElements,format);
	}

};

/**
 * @brief copies integer to strings
 */
class SStringToStringTCO: public StringTCO{

public:

	/**
	 * @brief constructor
	 */
	SStringToStringTCO(IOBufferWrapper *writerIn): StringTCO(writerIn){
	}

	/**
	 * @brief destructor
	 */
	virtual  ~SStringToStringTCO(){}

	/**
	 * @brief data conversion method
	 */
	virtual ErrorManagement::ErrorType Convert(uint8 *dest, const uint8 *source,uint32 numberOfElements,const FormatDescriptor &fd) const{
		ErrorManagement::ErrorType  ok;
		writer->Wrap(dest);

		uint8 *sourceD = const_cast<uint8 *>(source);
		StreamString *ss = reinterpret_cast<StreamString *>(sourceD);
		ok.fatalError = PrintStream(*writer,ss,fd);

		writer->Flush();

		return ok;
	}

	/**
	 * @brief data conversion method
	 */
	virtual ErrorManagement::ErrorType Convert(uint8 *dest, const uint8 *source,uint32 numberOfElements) const{
		return Convert(dest,source,numberOfElements,format);
	}

};

/*********************************************************************************************************/
/*                                                                                                       */
/*                                FACTORY                                                                */
/*                                                                                                       */
/*********************************************************************************************************/


class ToStringConversionFactory: TypeConversionFactoryI{

public:

	/**
 	 * @brief Default constructor. NOOP.
 	 */
	ToStringConversionFactory();

    /**
     * @brief Default destructor.
     */
    virtual ~ToStringConversionFactory();

	/**
     * @brief allow access to optimal functor for data conversion
	 *
	 */
	TypeConversionOperatorI *GetOperator(const TypeDescriptor &destTd,const TypeDescriptor &sourceTd);


private:

} sameTypeConversionFactory;

ToStringConversionFactory::ToStringConversionFactory(){
	TypeConversionManager::Instance().Register(this);
}

ToStringConversionFactory::~ToStringConversionFactory(){
}

TypeConversionOperatorI *ToStringConversionFactory::GetOperator(const TypeDescriptor &destTd,const TypeDescriptor &sourceTd){
	TypeConversionOperatorI *tco = NULL_PTR(TypeConversionOperatorI *);

	IOBufferWrapper *wrapper = NULL_PTR(IOBufferWrapper *);
	if (destTd == StreamType){
		wrapper = new IOBufferWrapperStream();
	} else
	if (destTd == StreamStringType){
		wrapper = new IOBufferWrapperSString();
	} else
	if (destTd == DynamicCharString){
		wrapper = new IOBufferDynStringWrapper();
	}

	// this implies SString,Stream,DynamicCString and excludes ConstCharString
	if (wrapper != NULL){
		if (!sourceTd.isStructuredData){
			switch(sourceTd.fullType){
			case TDF_UnsignedInteger:{
				if (!sourceTd.hasBitSize()){
					switch(sourceTd.objectSize){
					case Size8bit:{
						tco = new IntegerToStringTCO<uint8>(wrapper);
					}break;
					case Size16bit:{
						tco = new IntegerToStringTCO<uint16>(wrapper);
					}break;
					case Size32bit:{
						tco = new IntegerToStringTCO<uint32>(wrapper);
					}break;
					case Size64bit:{
						tco = new IntegerToStringTCO<uint64>(wrapper);
					}break;

					default:{
					}
					}
				} else {
					uint8 numberOfBits = sourceTd.numberOfBits;
					uint8 bitOffset = sourceTd.bitOffset;
					uint8 byteSize = SizeFromTDObjectSize(sourceTd.objectSize);
					tco = new BitSetToStringTCO(wrapper,byteSize,numberOfBits,bitOffset,false);
				}

			}break;
			case TDF_SignedInteger:{
				if (!sourceTd.hasBitSize()){
					switch(sourceTd.objectSize){
					case Size8bit:{
						tco = new IntegerToStringTCO<int8>(wrapper);
					}break;
					case Size16bit:{
						tco = new IntegerToStringTCO<int16>(wrapper);
					}break;
					case Size32bit:{
						tco = new IntegerToStringTCO<int32>(wrapper);
					}break;
					case Size64bit:{
						tco = new IntegerToStringTCO<int64>(wrapper);
					}break;
					default:{
					}
					}
				} else {
					uint8 numberOfBits = sourceTd.numberOfBits;
					uint8 bitOffset = sourceTd.bitOffset;
					uint8 byteSize = SizeFromTDObjectSize(sourceTd.objectSize);
					tco = new BitSetToStringTCO(wrapper,byteSize,numberOfBits,bitOffset,true);
				}
			}break;
			case TDF_Float:{
				switch(sourceTd.objectSize){
				case Size32bit:{
					tco = new FloatToStringTCO<float>(wrapper);
				}break;
				case Size64bit:{
					tco = new FloatToStringTCO<double>(wrapper);
				}break;
				default:{
				}
				}
			}break;
			case TDF_Pointer:{
				tco = new PointerToStringTCO(wrapper);
			}break;
			case TDF_DynamicCString:
			case TDF_CString:{
				tco = new CCStringToStringTCO(wrapper);
			}break;
			case TDF_SString:{
				tco = new SStringToStringTCO(wrapper);
			}break;
			case TDF_Stream:{
				tco = new StreamToStringTCO(wrapper);
			}break;

			default:{

			}
			}
		}
	}

	if ((tco == NULL) && (wrapper != NULL)){
		delete wrapper;
	}

	return tco;
}



} //MARTe