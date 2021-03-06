/*

Copyright (c) Microsoft Corporation
All rights reserved.

Licensed under the Apache License, Version 2.0 (the ""License""); you may not use this file except in compliance with the License. You may obtain a copy of the License at http://www.apache.org/licenses/LICENSE-2.0

THIS CODE IS PROVIDED ON AN  *AS IS* BASIS, WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, EITHER EXPRESS OR IMPLIED, INCLUDING WITHOUT LIMITATION ANY IMPLIED WARRANTIES OR CONDITIONS OF TITLE, FITNESS FOR A PARTICULAR PURPOSE, MERCHANTABLITY OR NON-INFRINGEMENT.

See the Apache Version 2.0 License for specific language governing permissions and limitations under the License.

*/

#pragma once

// cpp headers
#include <string>
#include <vector>
#include <tuple>

// os headers
#include <Windows.h>

// project headers
#include "ctException.hpp"
#include "ctString.hpp"
#include "ctMath.hpp"


namespace ctsPerf {
    namespace details {
        ////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
        ///
        /// write( ... )
        /// - overloads for ULONG and ULONGLONG data types
        /// - overloads for 1, 2, or 3 data points
        ///
        ////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
        inline std::wstring write(ULONGLONG _first_value)
        {
            return ctl::ctString::format_string(L",%llu", _first_value);
        }
        inline std::wstring write(ULONG _first_value)
        {
            return ctl::ctString::format_string(L",%lu", _first_value);
        }
        inline std::wstring write(double _first_value)
        {
            return ctl::ctString::format_string(L",%f", _first_value);
        }
        inline std::wstring write(ULONGLONG _first_value, ULONGLONG _second_value)
        {
            return ctl::ctString::format_string(L",%llu,%llu", _first_value, _second_value);
        }
        inline std::wstring write(ULONG _first_value, ULONG _second_value)
        {
            return ctl::ctString::format_string(L",%lu,%lu", _first_value, _second_value);
        }
        inline std::wstring write(double _first_value, double _second_value)
        {
            return ctl::ctString::format_string(L",%f,%f", _first_value, _second_value);
        }
        inline std::wstring write(ULONGLONG _first_value, ULONGLONG _second_value, ULONGLONG _third_value)
        {
            return ctl::ctString::format_string(L",%llu,%llu,%llu", _first_value, _second_value, _third_value);
        }
        inline std::wstring write(ULONG _first_value, ULONG _second_value, ULONG _third_value)
        {
            return ctl::ctString::format_string(L",%lu,%lu,%lu", _first_value, _second_value, _third_value);
        }
        inline std::wstring write(double _first_value, double _second_value, double _third_value)
        {
            return ctl::ctString::format_string(L",%f,%f,%f", _first_value, _second_value, _third_value);
        }
    }

    class ctsWriteDetails {
    private:
        void start_row(_In_ LPCWSTR _class_name, _In_ LPCWSTR _counter_name);
        void end_row();

        HANDLE file_handle = INVALID_HANDLE_VALUE;

    public:
        ctsWriteDetails(_In_ LPCWSTR _file_name, bool _mean_only);
        ~ctsWriteDetails() NOEXCEPT;
        ctsWriteDetails(const ctsWriteDetails&) = delete;
        ctsWriteDetails& operator=(const ctsWriteDetails&) = delete;

        //
        // The vector *will* be sorted before being returned (this is why it's non-const).
        //
        template <typename T>
        void write_details(_In_ LPCWSTR _class_name, _In_ LPCWSTR _counter_name, std::vector<T>& _data)
        {
            if (_data.empty()) {
                return;
            }

            start_row(_class_name, _counter_name);

            if (!_data.empty()) {
                // sort the data for IQR calculations
                sort(_data.begin(), _data.end());

                auto std_tuple = ctSampledStandardDeviation(_data.begin(), _data.end());
                auto interquartile_tuple = ctInterquartileRange(_data.begin(), _data.end());

                std::wstring formatted_data(details::write(static_cast<DWORD>(_data.size())));  // TotalCount
                formatted_data += details::write(*_data.begin(), *_data.rbegin()); // Min,Max
                formatted_data += details::write(std::get<0>(std_tuple), std::get<1>(std_tuple), std::get<2>(std_tuple)); // -1Std,Mean,+1Std
                formatted_data += details::write(std::get<0>(interquartile_tuple), std::get<1>(interquartile_tuple), std::get<2>(interquartile_tuple)); // -1IQR,Median,+1IQR

                DWORD length = static_cast<DWORD>(formatted_data.length() * sizeof(wchar_t));
                DWORD written;
                if (!::WriteFile(file_handle, formatted_data.c_str(), length, &written, NULL)) {
                    throw ctl::ctException(::GetLastError(), L"WriteFile", false);
                }
            }

            end_row();
        }

        template <typename T>
        void write_difference(_In_ LPCWSTR _class_name, _In_ LPCWSTR _counter_name, const std::vector<T>& _data)
        {
            if (_data.size() < 3) {
                return;
            }

            start_row(_class_name, _counter_name);
            // [0] == count
            // [1] == first
            // [2] == last
            std::wstring difference = details::write(_data[0], _data[2] - _data[1]);

            DWORD length = static_cast<DWORD>(difference.length() * sizeof(wchar_t));
            DWORD written;
            if (!::WriteFile(file_handle, difference.c_str(), length, &written, NULL)) {
                throw ctl::ctException(::GetLastError(), L"WriteFile", false);
            }

            end_row();
        }

        template <typename T>
        void write_mean(_In_ LPCWSTR _class_name, _In_ LPCWSTR _counter_name, const std::vector<T>& _data)
        {
            if (_data.size() < 4) {
                return;
            }

            start_row(_class_name, _counter_name);
            // assumes vector is formatted as:
            // [0] == count
            // [1] == min
            // [2] == max
            // [3] == mean
            std::wstring mean_string = details::write(_data[0], _data[1]);
            mean_string += details::write(_data[2], _data[3]);

            DWORD length = static_cast<DWORD>(mean_string.length() * sizeof(wchar_t));
            DWORD written;
            if (!::WriteFile(file_handle, mean_string.c_str(), length, &written, NULL)) {
                throw ctl::ctException(::GetLastError(), L"WriteFile", false);
            }

            end_row();
        }
    };
}