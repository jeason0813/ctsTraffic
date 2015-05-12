/*

Copyright (c) Microsoft Corporation
All rights reserved.

Licensed under the Apache License, Version 2.0 (the ""License""); you may not use this file except in compliance with the License. You may obtain a copy of the License at http://www.apache.org/licenses/LICENSE-2.0

THIS CODE IS PROVIDED ON AN  *AS IS* BASIS, WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, EITHER EXPRESS OR IMPLIED, INCLUDING WITHOUT LIMITATION ANY IMPLIED WARRANTIES OR CONDITIONS OF TITLE, FITNESS FOR A PARTICULAR PURPOSE, MERCHANTABLITY OR NON-INFRINGEMENT.

See the Apache Version 2.0 License for specific language governing permissions and limitations under the License.

*/

#pragma once

// cpp headers
#include <exception>
// os headers
#include <windows.h>
#include <Objbase.h>
#include <Wbemidl.h>
// ctl headers
#include "ctWmiErrorInfo.hpp"
#include "ctException.hpp"


namespace ctl {

class ctWmiException : public ctException
{
public:
    ///
    /// constructors
    ///
    ctWmiException() throw() : ctException(), className(NULL), errorInfo()
    {
    }

    explicit ctWmiException(HRESULT _ulCode)
    : ctException(_ulCode),
      className(NULL),
      errorInfo()
    {
    }
    explicit ctWmiException(HRESULT _ulCode, _In_ const IWbemClassObject* _classObject)
    : ctException(_ulCode),
      className(NULL),
      errorInfo()
    {
        get_className(_classObject);
    }

    explicit ctWmiException(_In_ LPCWSTR _wszMessage, bool _bMessageCopy  = true) throw()
    : ctException(_wszMessage, _bMessageCopy),
      className(NULL),
      errorInfo()
    {
    }
    explicit ctWmiException(_In_ LPCWSTR _wszMessage, _In_ const IWbemClassObject* _classObject, bool _bMessageCopy  = true) throw()
    : ctException(_wszMessage, _bMessageCopy),
      className(NULL),
      errorInfo()
    {
        get_className(_classObject);
    }

    explicit ctWmiException(HRESULT _ulCode, _In_ LPCWSTR _wszMessage, bool _bMessageCopy  = true) throw()
    : ctException(_ulCode, _wszMessage, _bMessageCopy),
      className(NULL),
      errorInfo()
    {
    }
    explicit ctWmiException(HRESULT _ulCode, _In_ const IWbemClassObject* _classObject, _In_ LPCWSTR _wszMessage, bool _bMessageCopy  = true) throw()
    : ctException(_ulCode, _wszMessage, _bMessageCopy),
      className(NULL),
      errorInfo()
    {
        get_className(_classObject);
    }

    explicit ctWmiException(HRESULT _ulCode, _In_ LPCWSTR _wszMessage, _In_ LPCWSTR _wszLocation, bool _bBothStringCopy  = true) throw()
    : ctException(_ulCode, _wszMessage, _wszLocation, _bBothStringCopy),
      className(NULL),
      errorInfo()
    {
    }
    explicit ctWmiException(HRESULT _ulCode, _In_ const IWbemClassObject* _classObject, _In_ LPCWSTR _wszMessage, _In_ LPCWSTR _wszLocation, bool _bBothStringCopy  = true) throw()
    : ctException(_ulCode, _wszMessage, _wszLocation, _bBothStringCopy),
      className(NULL),
      errorInfo()
    {
        get_className(_classObject);
    }
    ///
    /// Copy c'tor
    ///
    ctWmiException(_In_ const ctWmiException& e) throw()
    : ctException(e),
      className(NULL),
      errorInfo(e.errorInfo)
    {
        // must do a deep copy, not just copy the ptr
        // if this fails, we'll just have NULL instead of a class name
        // - not failing this just because we can't get extra info
        if (e.className != NULL) {
            className = ::SysAllocString(e.className);
        }
    }

    ///
    /// destructor
    ///
    virtual ~ctWmiException() throw()
    {
        // SysFreeString handles NULL
        ::SysFreeString(className);
    }

    // public accessors
    virtual const wchar_t* class_w() const throw()
    {
        return (className == NULL) ? L"" : className;
    }

    virtual ctWmiErrorInfo error_info() const throw()
    {
        // copy c'tor of ctWmiErrorInfo is no-throw
        // - so is safe to return a copy here
        return errorInfo;
    }

private:
    // using a raw BSTR to ensure no method will throw
    BSTR className;
    ctWmiErrorInfo errorInfo;

    void get_className(_In_ const IWbemClassObject* _classObject) throw()
    {
        //
        // protect against a null IWbemClassObject pointer
        //
        if(_classObject != NULL)
        {
            assert(NULL == className);
        
            VARIANT variant;
            ::VariantInit(&variant);
            // the method should allow to be called from const() methods
            // - forced to const-cast to make this const-correct
            HRESULT hr = const_cast<IWbemClassObject*>(_classObject)->Get(
                L"__CLASS", 0, &variant, 0, 0
                );
            if (SUCCEEDED(hr)) {
                // copy the BSTR from the VARIANT
                // - do NOT free the VARIANT
                this->className = variant.bstrVal;
            }
        }
    }
};

} // namespace

#endif