/**
 * @file ToFormattedStreamsConversionFactory.h
 * @brief Header file for class AnyType
 * @date Jul 9, 2020
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




/*---------------------------------------------------------------------------*/
/*                        Standard header includes                           */
/*---------------------------------------------------------------------------*/

/*---------------------------------------------------------------------------*/
/*                        Project header includes                            */
/*---------------------------------------------------------------------------*/

/*---------------------------------------------------------------------------*/
/*                          Forward declarations                             */
/*---------------------------------------------------------------------------*/

/*---------------------------------------------------------------------------*/
/*                           Class declaration                               */
/*---------------------------------------------------------------------------*/

#define DLL_API

#include "TypeConversionFactoryI.h"
#include "TypeConversionManager.h"
#include "IOBufferWrappers.h"
#include "StreamString.h"
#include "CompositeErrorManagement.h"


namespace MARTe{



/*---------------------------------------------------------------------------*/
/*                        Inline method definitions                          */
/*---------------------------------------------------------------------------*/


/*********************************************************************************************************/
/*                                                                                                       */
/*                                TYPE CONVERSION OPERATORS                                              */
/*                                                                                                       */
/*********************************************************************************************************/

/**
 * @brief copies to strings
 */
class StringTCO;
/**
 * @brief copies integer to strings
 */
template <typename integerType>
class IntegerToStringTCO;
/**
 * @brief copies integer to strings
 */
class CharToStringTCO;
/**
 * @brief copies bitset integers to strings
 */
class BitSetToStringTCO;
/**
 * @brief copies bitset integers to strings
 */
class PointerToStringTCO;
/**
 * @brief copies floats to strings
 */
template <typename floatType>
class FloatToStringTCO;
/**
 * @brief copies integer to strings
 */
class CCStringToStringTCO;
/**
 * @brief copies stream to strings
 */
class StreamToStringTCO;
/**
 * @brief copies integer to strings
 */
class SStringToStringTCO;

/**
 * @brief copies to strings
 */
class StringTCO: public TypeConversionOperatorI{
public:

    /**
     * @brief constructor
     */
    StringTCO(IOBufferWrapper *writerIn);
    /**
     * @brief destructor
     */
    virtual  ~StringTCO();
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
    IntegerToStringTCO(IOBufferWrapper *writerIn);

    /**
     * @brief destructor
     */
    virtual  ~IntegerToStringTCO();

    /**
     * @brief data conversion method
     */
    virtual ErrorManagement::ErrorType Convert(uint8 *dest, const uint8 *source,uint32 numberOfElements,const FormatDescriptor &fd) const;

    /**
     * @brief data conversion method
     */
    virtual ErrorManagement::ErrorType Convert(uint8 *dest, const uint8 *source,uint32 numberOfElements) const;
};

/**
 * @brief copies integer to strings
 */
class CharToStringTCO: public StringTCO{

public:

    /**
     * @brief constructor
     */
    CharToStringTCO(IOBufferWrapper *writerIn);

    /**
     * @brief destructor
     */
    virtual  ~CharToStringTCO();

    /**
     * @brief data conversion method
     */
    virtual ErrorManagement::ErrorType Convert(uint8 *dest, const uint8 *source,uint32 numberOfElements,const FormatDescriptor &fd) const;

    /**
     * @brief data conversion method
     */
    virtual ErrorManagement::ErrorType Convert(uint8 *dest, const uint8 *source,uint32 numberOfElements) const;
};

/**
 * @brief copies bitset integers to strings
 */
class BitSetToStringTCO: public StringTCO{

public:

    /**
     * @brief constructor
     */
    BitSetToStringTCO(IOBufferWrapper *writerIn,TypeDescriptor td,bool isSignedIn);

    /**
     * @brief destructor
     */
    virtual  ~BitSetToStringTCO();

    /**
     * @brief data conversion method
     */
    virtual ErrorManagement::ErrorType Convert(uint8 *dest, const uint8 *source,uint32 numberOfElements,const FormatDescriptor &td) const;

    /**
     * @brief data conversion method
     */
    virtual ErrorManagement::ErrorType Convert(uint8 *dest, const uint8 *source,uint32 numberOfElements) const;

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
    PointerToStringTCO(IOBufferWrapper *writerIn);

    /**
     * @brief destructor
     */
    virtual  ~PointerToStringTCO();

    /**
     * @brief data conversion method
     */
    virtual ErrorManagement::ErrorType Convert(uint8 *dest, const uint8 *source,uint32 numberOfElements) const;
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
    FloatToStringTCO(IOBufferWrapper *writerIn);

    /**
     * @brief destructor
     */
    virtual  ~FloatToStringTCO();

    /**
     * @brief data conversion method
     */
    virtual ErrorManagement::ErrorType Convert(uint8 *dest, const uint8 *source,uint32 numberOfElements,const FormatDescriptor &fd) const;

    /**
     * @brief data conversion method
     */
    virtual ErrorManagement::ErrorType Convert(uint8 *dest, const uint8 *source,uint32 numberOfElements) const;
};

/**
 * @brief copies integer to strings
 */
class CCStringToStringTCO: public StringTCO{

public:

    /**
     * @brief constructor
     */
    CCStringToStringTCO(IOBufferWrapper *writerIn);

    /**
     * @brief destructor
     */
    virtual  ~CCStringToStringTCO();

    /**
     * @brief data conversion method
     */
    virtual ErrorManagement::ErrorType Convert(uint8 *dest, const uint8 *source,uint32 numberOfElements,const FormatDescriptor &fd) const;

    /**
     * @brief data conversion method
     */
    virtual ErrorManagement::ErrorType Convert(uint8 *dest, const uint8 *source,uint32 numberOfElements) const;
};

/**
 * @brief copies stream to strings
 */
class StreamToStringTCO: public StringTCO{

public:

    /**
     * @brief constructor
     */
    StreamToStringTCO(IOBufferWrapper *writerIn);

    /**
     * @brief destructor
     */
    virtual  ~StreamToStringTCO();

    /**
     * @brief data conversion method
     */
    virtual ErrorManagement::ErrorType Convert(uint8 *dest, const uint8 *source,uint32 numberOfElements,const FormatDescriptor &fd) const;

    /**
     * @brief data conversion method
     */
    virtual ErrorManagement::ErrorType Convert(uint8 *dest, const uint8 *source,uint32 numberOfElements) const;

};

/**
 * @brief copies integer to strings
 */
class SStringToStringTCO: public StringTCO{

public:

    /**
     * @brief constructor
     */
    SStringToStringTCO(IOBufferWrapper *writerIn);

    /**
     * @brief destructor
     */
    virtual  ~SStringToStringTCO();

    /**
     * @brief data conversion method
     */
    virtual ErrorManagement::ErrorType Convert(uint8 *dest, const uint8 *source,uint32 numberOfElements,const FormatDescriptor &fd) const;

    /**
     * @brief data conversion method
     */
    virtual ErrorManagement::ErrorType Convert(uint8 *dest, const uint8 *source,uint32 numberOfElements) const;

};

/***************************************************************************************************************/
/*  IMPLEMENTATION */
/***************************************************************************************************************/

StringTCO::StringTCO(IOBufferWrapper *writerIn): writer(writerIn){
}

StringTCO::~StringTCO(){
    delete writer;
}

template <typename integerType>
IntegerToStringTCO<integerType>::IntegerToStringTCO(IOBufferWrapper *writerIn): StringTCO(writerIn){
}

template <typename integerType>
IntegerToStringTCO<integerType>::~IntegerToStringTCO(){}

template <typename integerType>
ErrorManagement::ErrorType IntegerToStringTCO<integerType>::Convert(uint8 *dest, const uint8 *source,uint32 numberOfElements,const FormatDescriptor &fd) const{
    ErrorManagement::ErrorType  ok;
    if (writer != NULL){
        writer->Wrap(dest);
        uint32 ix = 0;
        const integerType *pIn = (reinterpret_cast<const integerType *>(source));
        if (!IntegerToStream(*writer,*pIn++,fd)){
            ok.fatalError = true;
            REPORT_ERROR(ok,"IntegerToStream Failed");
        }
        for (ix = 1;(ix<numberOfElements) && ok;ix++){
            ok = writer->Next();
            if (!ok){
                COMPOSITE_REPORT_ERROR(ok,"switch to element ",ix," failed");
            }

            if (ok){
                if (!IntegerToStream(*writer,*pIn++,fd)){
                    ok.fatalError = true;
                    REPORT_ERROR(ok,"IntegerToStream Failed");
                }
            }
        }
        writer->Flush();
        if (ok){
            ok = writer->Check();
        }
    } else {
        ok.internalSetupError = true;
    }

    return ok;
}

template <typename integerType>
ErrorManagement::ErrorType IntegerToStringTCO<integerType>::Convert(uint8 *dest, const uint8 *source,uint32 numberOfElements) const{
    return Convert(dest,source,numberOfElements,format);
}

CharToStringTCO::CharToStringTCO(IOBufferWrapper *writerIn): StringTCO(writerIn){
}

CharToStringTCO::~CharToStringTCO(){}

ErrorManagement::ErrorType CharToStringTCO::Convert(uint8 *dest, const uint8 *source,uint32 numberOfElements,const FormatDescriptor &fd) const{
    ErrorManagement::ErrorType  ok;
    if (writer != NULL){
        writer->Wrap(dest);
        uint32 ix = 0;
        const char *pIn = (reinterpret_cast<const char *>(source));
        if (!writer->PutC(*pIn)){
            ok.fatalError = true;
            REPORT_ERROR(ok,"PutC Failed");
        }
        for (ix = 1;(ix<numberOfElements) && ok;ix++){
            ok = writer->Next();
            if (!ok){
                COMPOSITE_REPORT_ERROR(ok,"switch to element ",ix," failed");
            }

            if (ok){
                if (!writer->PutC(*pIn)){
                    ok.fatalError = true;
                    REPORT_ERROR(ok,"PutC Failed");
                }
            }
        }
        writer->Flush();
        if (ok){
            ok = writer->Check();
        }
    } else {
        ok.internalSetupError = true;
    }

    return ok;
}

ErrorManagement::ErrorType CharToStringTCO::Convert(uint8 *dest, const uint8 *source,uint32 numberOfElements) const{
    return Convert(dest,source,numberOfElements,format);
}

BitSetToStringTCO::BitSetToStringTCO(IOBufferWrapper *writerIn,TypeDescriptor td,bool isSignedIn): StringTCO(writerIn){
    numberBitSize  = static_cast<uint8>(td.GetNumericBitSize());
    numberBitShift = static_cast<uint8>(td.GetNumericBitOffset());
    byteSize       = td.StorageSize();
    isSigned       = isSignedIn;
}

BitSetToStringTCO::~BitSetToStringTCO(){}

ErrorManagement::ErrorType BitSetToStringTCO::Convert(uint8 *dest, const uint8 *source,uint32 numberOfElements,const FormatDescriptor &td) const{
    ErrorManagement::ErrorType  ok;
    writer->Wrap(dest);

    if (!BitSetToStream(*writer,reinterpret_cast<uint32 const * >(source),numberBitShift,numberBitSize,isSigned,td)){
        ok.fatalError = true;
        REPORT_ERROR(ok,"BitSetToStream Failed");
    }

    for (uint32 ix = 1; (ix < numberOfElements) && ok;ix++){
        ok = writer->Next();
        if (!ok){
            COMPOSITE_REPORT_ERROR(ok,"switch to element ",ix," failed");
        }

        source += byteSize;
        if (ok){
            if (!BitSetToStream(*writer,reinterpret_cast<uint32 const * >(source),numberBitShift,numberBitSize,isSigned,td)){
                ok.fatalError = true;
                REPORT_ERROR(ok,"BitSetToStream Failed");
            }
        }
    }
    writer->Flush();
    if (ok){
        ok = writer->Check();
    }

    return ok;
}

ErrorManagement::ErrorType BitSetToStringTCO::Convert(uint8 *dest, const uint8 *source,uint32 numberOfElements) const{
    return Convert(dest,source,numberOfElements,format);
}

PointerToStringTCO::PointerToStringTCO(IOBufferWrapper *writerIn): StringTCO(writerIn){ }

PointerToStringTCO::~PointerToStringTCO(){}

ErrorManagement::ErrorType PointerToStringTCO::Convert(uint8 *dest, const uint8 *source,uint32 numberOfElements) const{
    ErrorManagement::ErrorType  ok;
    writer->Wrap(dest);

    uint8 *source1 = const_cast<uint8 * >(source);
    const void **src = reinterpret_cast<const void ** >(source1);

    if (!PointerToStream(*writer,*src)){
        ok.fatalError = true;
    }
    for (uint32 ix = 1; (ix < numberOfElements) && ok;ix++){
        ok = writer->Next();
        if (!ok){
            COMPOSITE_REPORT_ERROR(ok,"switch to element ",ix," failed");
        }
        src++;
        if (ok){
            ok.fatalError = PointerToStream(*writer,*src);
        }
    }
    writer->Flush();
    if (ok){
        ok = writer->Check();
    }

    return ok;
}

template <typename floatType>
FloatToStringTCO<floatType>::FloatToStringTCO(IOBufferWrapper *writerIn): StringTCO(writerIn){
}

template <typename floatType>
FloatToStringTCO<floatType>::~FloatToStringTCO(){}

template <typename floatType>
ErrorManagement::ErrorType FloatToStringTCO<floatType>::Convert(uint8 *dest, const uint8 *source,uint32 numberOfElements,const FormatDescriptor &fd) const{
    ErrorManagement::ErrorType  ok;
    writer->Wrap(dest);

    const floatType *src = reinterpret_cast<const floatType *>(source);
    if (!FloatToStream(*writer,*src,fd)){
        ok.fatalError = true;
        REPORT_ERROR(ok,"FloatToStream Failed");
    }

    for (uint32 ix = 1; (ix < numberOfElements) && ok;ix++){
        ok = writer->Next();
        if (!ok){
            COMPOSITE_REPORT_ERROR(ok,"switch to element ",ix," failed");
        }
        src++;
        if (ok){
            if (!FloatToStream(*writer,*src,fd)){
                ok.fatalError = true;
                REPORT_ERROR(ok,"FloatToStream Failed");
            }
        }
    }
    writer->Flush();
    if (ok){
        ok = writer->Check();
    }

    return ok;
}

template <typename floatType>
ErrorManagement::ErrorType FloatToStringTCO<floatType>::Convert(uint8 *dest, const uint8 *source,uint32 numberOfElements) const{
    return Convert(dest,source,numberOfElements,format);
}


CCStringToStringTCO::CCStringToStringTCO(IOBufferWrapper *writerIn): StringTCO(writerIn){
}

CCStringToStringTCO::~CCStringToStringTCO(){}

ErrorManagement::ErrorType CCStringToStringTCO::Convert(uint8 *dest, const uint8 *source,uint32 numberOfElements,const FormatDescriptor &fd) const{
    ErrorManagement::ErrorType  ok;
    writer->Wrap(dest);

    const CCString *src = reinterpret_cast<const CCString *>(source);

//{ // TODO remove
//printf("@ %p string[%i]\n",src,numberOfElements);
//for (uint32 ix = 0; (ix < numberOfElements) && ok;ix++){
//  printf("%i @%p string \n",ix,src[ix].GetList());
//}
//}

    if (!PrintCCString(*writer,*src,fd)){
        ok.fatalError = true;
        REPORT_ERROR(ok,"PrintCCString Failed");
    }
    for (uint32 ix = 1; (ix < numberOfElements) && ok;ix++){
        ok = writer->Next();
        if (!ok){
            COMPOSITE_REPORT_ERROR(ok,"switch to element ",ix," failed");
        }
        src++;
        if (ok){
            if (!PrintCCString(*writer,*src,fd)){
                ok.fatalError = true;
                REPORT_ERROR(ok,"PrintCCString Failed");
            }
        }
    }
    writer->Flush();
    if (ok){
        ok = writer->Check();
    }

    return ok;
}

ErrorManagement::ErrorType CCStringToStringTCO::Convert(uint8 *dest, const uint8 *source,uint32 numberOfElements) const{
    return Convert(dest,source,numberOfElements,format);
}

StreamToStringTCO::StreamToStringTCO(IOBufferWrapper *writerIn): StringTCO(writerIn){
}

StreamToStringTCO::~StreamToStringTCO(){}

ErrorManagement::ErrorType StreamToStringTCO::Convert(uint8 *dest, const uint8 *source,uint32 numberOfElements,const FormatDescriptor &fd) const{
    ErrorManagement::ErrorType  ok;
//TODO manage more elements by using their size
    ok.unsupportedFeature = (numberOfElements!= 1);

    if (ok){
        writer->Wrap(dest);

        uint8 *srcc = const_cast<uint8 *>(source);
        StreamString *src = reinterpret_cast<StreamString *>(srcc);

        if (!PrintStream(*writer,src,fd)){
            ok.fatalError = true;
            REPORT_ERROR(ok,"PrintStream Failed");
        }
        for (uint32 ix = 1; (ix < numberOfElements) && ok;ix++){
            ok = writer->Next();
            if (!ok){
                COMPOSITE_REPORT_ERROR(ok,"switch to element ",ix," failed");
            }
            src++;
            if (ok){
                if (!PrintStream(*writer,src,fd)){
                    ok.fatalError = true;
                    REPORT_ERROR(ok,"PrintStream Failed");
                }
            }
        }
        writer->Flush();
        if (ok){
            ok = writer->Check();
        }
    }

    return ok;
}

ErrorManagement::ErrorType StreamToStringTCO::Convert(uint8 *dest, const uint8 *source,uint32 numberOfElements) const{
    return Convert(dest,source,numberOfElements,format);
}


SStringToStringTCO::SStringToStringTCO(IOBufferWrapper *writerIn): StringTCO(writerIn){
}

SStringToStringTCO::~SStringToStringTCO(){}

ErrorManagement::ErrorType SStringToStringTCO::Convert(uint8 *dest, const uint8 *source,uint32 numberOfElements,const FormatDescriptor &fd) const{
    ErrorManagement::ErrorType  ok;
    writer->Wrap(dest);

    uint8 *sourceD = const_cast<uint8 *>(source);
    StreamString *ss = reinterpret_cast<StreamString *>(sourceD);

    if (!PrintCCString(*writer,ss->Buffer(),fd)){
        ok.fatalError = true;
        REPORT_ERROR(ok,"PrintCCString Failed");
    }
    for (uint32 ix = 1; (ix < numberOfElements) && ok;ix++){
        ok = writer->Next();
        if (!ok){
            COMPOSITE_REPORT_ERROR(ok,"switch to element ",ix," failed");
        }
        ss++;
        if (ok){
            if (!PrintCCString(*writer,ss->Buffer(),fd)){
                ok.fatalError = true;
                REPORT_ERROR(ok,"PrintCCString Failed");
            }
        }
    }
    writer->Flush();
    if (ok){
        ok = writer->Check();
    }

    return ok;
}

ErrorManagement::ErrorType SStringToStringTCO::Convert(uint8 *dest, const uint8 *source,uint32 numberOfElements) const{
    return Convert(dest,source,numberOfElements,format);
}

/*********************************************************************************************************/
/*                                                                                                       */
/*                                FACTORY                                                                */
/*                                                                                                       */
/*********************************************************************************************************/


class ToFormattedStreamConversionFactory: public TypeConversionFactoryI{

public:

    /**
     * @brief Default constructor. NOOP.
     */
    ToFormattedStreamConversionFactory();

    /**
     * @brief Default destructor.
     */
    virtual ~ToFormattedStreamConversionFactory();

    /**
     * @brief allow access to optimal functor for data conversion
     *
     */
    TypeConversionOperatorI *GetOperator(const TypeDescriptor &destTd,const TypeDescriptor &sourceTd,bool isCompare);

private:

} toStringConversionFactory;

ToFormattedStreamConversionFactory::ToFormattedStreamConversionFactory(){
}

ToFormattedStreamConversionFactory::~ToFormattedStreamConversionFactory(){
}


TypeConversionOperatorI *ToFormattedStreamConversionFactory::GetOperator(const TypeDescriptor &destTd,const TypeDescriptor &sourceTd,bool isCompare){
    TypeConversionOperatorI *tco = NULL_PTR(TypeConversionOperatorI *);

    IOBufferWrapper *wrapper = NULL_PTR(IOBufferWrapper *);
    if (isCompare){
        if (destTd.SameAs(DynamicCharString) ||
            destTd.SameTypeAs(ConstCharString) ||
            destTd.SameTypeAs(CharString(0))){
            wrapper = new IOBufferCStringCompareWrapper();
        }
    } else {
        if (destTd.SameTypeAs(StreamIType(0))){
            wrapper = new IOBufferWrapperStream(destTd.StorageSize());
        } else
        if (destTd.SameAs(StreamStringType(sizeof(StreamString))) ){
            wrapper = new IOBufferWrapperSString();
        } else
        if (destTd.SameAs(DynamicCharString)){
            wrapper = new IOBufferDynStringWrapper();
        }
    }

    // this implies SString,Stream,DynamicCString and excludes ConstCharString
    if (wrapper != NULL){
        TD_FullType fullType = sourceTd.GetFullTypeCode();
//      uint32 basicTypeSize = sourceTd.basicTypeSize;
        uint32 storageSize = sourceTd.StorageSize();
        bool hasBitSize = sourceTd.IsBitType();
        bool isStructuredData = sourceTd.IsStructuredData();
        if (!isStructuredData){
            switch(fullType){
            case TDF_Char:{
                tco = new CharToStringTCO(wrapper);
            }break;
            case TDF_UnsignedInteger:{
                if (!hasBitSize){
                    switch(storageSize){
                    case 1:{
                        tco = new IntegerToStringTCO<uint8>(wrapper);
                    }break;
                    case 2:{
                        tco = new IntegerToStringTCO<uint16>(wrapper);
                    }break;
                    case 4:{
                        tco = new IntegerToStringTCO<uint32>(wrapper);
                    }break;
                    case 8:{
                        tco = new IntegerToStringTCO<uint64>(wrapper);
                    }break;

                    default:{
                    }
                    }
                } else {
                    tco = new BitSetToStringTCO(wrapper,sourceTd,false);
                }

            }break;
            case TDF_SignedInteger:{
                if (!hasBitSize){
                    switch(storageSize){
                    case 1:{
                        tco = new IntegerToStringTCO<int8>(wrapper);
                    }break;
                    case 2:{
                        tco = new IntegerToStringTCO<int16>(wrapper);
                    }break;
                    case 4:{
                        tco = new IntegerToStringTCO<int32>(wrapper);
                    }break;
                    case 8:{
                        tco = new IntegerToStringTCO<int64>(wrapper);
                    }break;
                    default:{
                    }
                    }
                } else {
                    tco = new BitSetToStringTCO(wrapper,sourceTd,true);
                }
            }break;
            case TDF_Float:{
                switch(storageSize){
                case 4:{
                    tco = new FloatToStringTCO<float>(wrapper);
                }break;
                case 8:{
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
            case TDF_CString:
            case TDF_CCString:{
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

INSTALL_STARTUP_MANAGER_INITIALISATION_ENTRY(ToFormattedStreamConversionFactory,("TCMService",emptyString),("TCMDataBase",emptyString))

ErrorManagement::ErrorType ToFormattedStreamConversionFactoryStartup::Init(){
    ErrorManagement::ErrorType ret;
    ret.initialisationError = !TypeConversionManager::Register(&toStringConversionFactory);

    return ret;
}

ErrorManagement::ErrorType ToFormattedStreamConversionFactoryStartup::Finish(){
    ErrorManagement::ErrorType ret;

    TypeConversionManager::Clean();

    return ret;
}

} // MARTe
