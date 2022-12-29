#ifndef __HDK_MARSHALL_TYPE_INFO_HPP__
#define __HDK_MARSHALL_TYPE_INFO_HPP__

#ifdef __GNUG__
# include <cxxabi.h>
#endif

#include "checksum.hpp"

namespace hdk
{
    namespace _PRIVATE
    {
        class basic_type_info
        {
        public:
            basic_type_info() = default;
            virtual ~basic_type_info() = default;

            virtual void marshal(std::ostream& os, const void* struct_data) const = 0 ;
            virtual void demarshal(std::istream& is, void* struct_data) const = 0;
            virtual uint32_t type_id(void* struct_data) const = 0;
        };
    }

    template<typename T>
    class marshall_type_info : public _PRIVATE::basic_type_info
    {
    public:
        marshall_type_info() = default;
        virtual ~marshall_type_info() = default;
        virtual void marshal(std::ostream& os, const void* struct_data) const { ((T*)struct_data)->marshal(os); }
        virtual void demarshal(std::istream& is, void* struct_data)    const { ((T*)struct_data)->demarshal(is); }
        virtual uint32_t type_id(void* struct_data) const { return ((T*)struct_data)->get_type_id(); }
    };

    template<typename T>
    inline const _PRIVATE::basic_type_info* init_type_info(const T*)
    {
        static marshall_type_info<T> s_type_info;
        return &s_type_info;
    }

    template<typename T>
    const char* type_name(const T&)
    {
        static const char* name = typeid(T).name();
#ifdef __GNUG__
        int status = -4;
        static const char* s_type_name = abi::__cxa_demangle(name, NULL, NULL, &status);
        return (s_type_name == nullptr) ? name : s_type_name;
#else
        return name;
#endif
    }

    inline uint32_t get_type_id(const char* type_name, uint32_t prev_crc32)
    {
        return hdk::checksum::crc32(type_name, strlen(type_name), prev_crc32);
    }

    const char* function_name();

} // namespace hdk

//  std::unique_ptr<char, void(*)(void*)> res {abi::__cxa_demangle(name, NULL, NULL, &status), std::free};


#endif // __HDK_MARSHALL_TYPE_INFO_HPP__
