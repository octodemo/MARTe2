/**
 * @file ClassMethodsRegistryItem.h
 * @brief Header file for class ClassMethodsRegistryItem
 * @date 07/04/2016
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
 *
 * @details This header file contains the declaration of the class ClassMethodsRegistryItem
 * with all of its public, protected and private members. It may also include
 * definitions for inline methods which need to be visible to the compiler.
 */

#ifndef CLASSMETHODSREGISTRYITEM_H_
#define CLASSMETHODSREGISTRYITEM_H_

/*---------------------------------------------------------------------------*/
/*                        Standard header includes                           */
/*---------------------------------------------------------------------------*/

/*---------------------------------------------------------------------------*/
/*                        Project header includes                            */
/*---------------------------------------------------------------------------*/

#include "LinkedListable.h"
#include "ClassMethodInterfaceMapper.h"
#include "CString.h"

/*---------------------------------------------------------------------------*/
/*                          Forward declarations                             */
/*---------------------------------------------------------------------------*/

namespace MARTe {
class ClassRegistryItem;
}

/*---------------------------------------------------------------------------*/
/*                           Class declaration                               */
/*---------------------------------------------------------------------------*/

namespace MARTe {

/**
 * @brief A list of a class callable methods.
 */
class DLL_API ClassMethodsRegistryItem: public LinkedListable {

public:

    /**
     * @brief Constructor.
     * @details Passing a list of class methods, the ClassMethodInterfaceMapper array will be automatically built
     * and passed as the constructor input. The constructor adds this object in ClassRegistryItem passed in input.
     * All the methods have to belong to the class we want to register, otherwise at the moment of calling the function
     * the CallFunction(*) will return ErrorManagement::UnsupportedFeature.
     * @param[in] cri is the ClassRegistryItem where this object has to be added to.
     * @param[in] functionTable_in is the list of ClassMethodInterfaceMapper, each one capable to store and call one
     * class method.
     * @param[in] functionNames_in is a string containing all the class methods names. The syntax for this string
     * has to be "ClassName::FunctionName1, ClassName::FunctionName2, ...". ClassName it is not forced to be the same
     * name of the class to be registered because can be also the name of an ancestor of this class.
     * Example (registering virtual function):\n
     * Assume we have the inheritance chain A<--B<--C and the virtual function A::f() reimplemented in B::f() but not in C.
     * Registering A::f() in the ClassRegistryItem of the class C, means that when calling CallFunction(C* x) the implementation will be
     * the one defined in B, namely B::f() will be called because of the polimorphism.
     * Example (registering non virtual function):\n
     * Assume we have the inheritance chain A<--B<--C and the non-virtual function A::f() implemented also in B::f() but not in C.
     * Registering A::f() in the ClassRegistryItem of the class C, means that when calling CallFunction(C* x) the implementation will be
     * the one defined in A, namely A::f(). Registering B::f() in the ClassRegistryItem of C, the implementation will be the one defined
     * in B::f() also passing a pointer to C in the CallFunction. At last, registering C::f(), CallFunction(C* x) will return UnsupportedFeature
     * because C::f() has not been defined.
     */
    ClassMethodsRegistryItem(ClassRegistryItem * const cri,
                             ClassMethodInterfaceMapper * const functionTable_In,
                             const char8 * const functionNames_In);

    /**
     * @brief Destructor.
     */
    virtual ~ClassMethodsRegistryItem();

    /**
     * @brief Calls the function with one argument.
     * @param[in] context is the object which must call the class method.
     * @param[in] name is the name of the class method to be called. The name has to be
     * only the name of the function without any extra mangling.
     * @param[in,out] is the class method argument.
     * @return ErrorManagement::UnsupportedFeature if \a name does not match with any of the names in the
     * function names list or if the class method does not belong to \a context. ErrorManagement::FatalError
     * will be returned if the class method returns false, ErrorManagemenr::NoError if it returns true.
     */
    ErrorManagement::ErrorType CallFunction(Object * const context,
                                            const char8 * const name);

    /**
     * @brief Calls the function with one argument.
     * @param[in] context is the object which must call the class method.
     * @param[in] name is the name of the class method to be called. The name has to be
     * only the name of the function without any extra mangling.
     * @param[in,out] is the class method argument.
     * @return ErrorManagement::UnsupportedFeature if \a name does not match with any of the names in the
     * function names list or if the class method does not belong to \a context. ErrorManagement::FatalError
     * will be returned if the class method returns false, ErrorManagemenr::NoError if it returns true.
     * @tparam argType is the type of the argument to pass to the class method.
     */
    template<typename argType>
    ErrorManagement::ErrorType CallFunction(Object * const context,
                                            const char8 * const name,
                                            argType ref);

private:

    /**
     * @brief Finds the class method name in input in the class method names list.
     * @param[in] name is the name of the class method (without any mangling)
     * to be searched in the names list.
     * @return the index of the class method pointer in the table if \a name is found in the list,
     * -1 otherwise.
     */
    int32 FindFunction(const char8 * const name,
                       int32 minIndex);

    /**
     * The array of objects used to call the class methods.
     */
    ClassMethodInterfaceMapper * const functionTable;

    /**
     * The class method names list.
     */
    CCString functionNames;

};

}

/*---------------------------------------------------------------------------*/
/*                        Inline method definitions                          */
/*---------------------------------------------------------------------------*/

namespace MARTe {

template<typename argType>
ErrorManagement::ErrorType ClassMethodsRegistryItem::CallFunction(Object * context,
                                                                  const char8 *name,
                                                                  argType ref) {
    ErrorManagement::ErrorType returnValue;

    if (context == NULL) {
        returnValue.parametersError = true;
    }
    if (name == NULL) {
        returnValue.parametersError = true;
    }

    if (returnValue.NoError()) {
        ClassMethodInterfaceMapper * fmp = NULL_PTR(ClassMethodInterfaceMapper *);
        int32 minIndex = 0;
        int32 functionIndex = 0;
        while (functionIndex >= 0) {
            returnValue = true;
            functionIndex = FindFunction(name, minIndex);
            if (functionIndex >= 0) {
                fmp = &functionTable[functionIndex];
            }
            else {
                returnValue.unsupportedFeature = true;
            }

            if (returnValue.NoError()) {
                /*lint -e{613} .The NULL checking has been done before entering here*/
                returnValue = fmp->Call<argType>(context, ref);
                if (returnValue.unsupportedFeature == true) {
                    // allow function overload, try again to search!!
                    minIndex = functionIndex + 1;
                }
                else {
                    //the function has been executed.. exit
                    functionIndex = -1;
                }
            }
        }
    }

    return returnValue;
}

}

/**
 * @brief Macro used to register automatically a list of class methods in the ClassRegistryItem (and in the ClassRegistryDatabase).
 * @details Passing a list of class method pointers, this macro automatically creates the list of function names. To find the function
 * in the list it is only necessary that the function name (without mangling) is enclosed between a ':' and a ',' or '\0'. For instance
 * putting "(bool (*)())A::f1(),B::f2()" the find function will match the functions f1 and f2. It is possible to register two different
 * functions with the same name (if a class overloads a method).
 * @param[in] C is the class name.
 * @param[in] ... list of class methods.
 */
#define CLASS_METHOD_REGISTER(C,...)\
    static MARTe::ClassMethodInterfaceMapper C ## __ClassMethodsInterfaceMapper[] = {__VA_ARGS__}; \
    static /*const*/ MARTe::ClassMethodsRegistryItem C ## __ClassMethodsRegistryItem(C::GetClassRegistryItem_Static(),C ## __ClassMethodsInterfaceMapper,#__VA_ARGS__);

#endif /* CLASSMETHODSREGISTRYITEM_H_ */
