#ifndef __HDK_MACRO_HPP__
#define __HDK_MACRO_HPP__


#include <iostream>
#include <atomic>

#include "checksum.hpp"

#include "marshall/marshal.hpp"
#include "marshall/marshall_type_info.hpp"
#include "marshall/marshall_stream.hpp"

namespace hdk
{
    namespace _PRIVATE
    {
        template<uint32_t CRC>
        inline constexpr uint32_t get_static_crc32() { return CRC; }
    }
}

#define __GLUE_VA_ARGS(X...) #X

#define __HDK_MARSHAL_IMPL(...) \
    void marshal(std::ostream& os) const { hdk::_PRIVATE::marshal(os, __VA_ARGS__); } \
    void demarshal(std::istream& is)     { hdk::_PRIVATE::demarshal(is, __VA_ARGS__); } \
    uint32_t get_type_id() const \
    {\
        static uint32_t s_type_id = hdk::get_type_id(hdk::type_name(*this), \
                                       hdk::_PRIVATE::get_static_crc32<hdk::checksum::crc32_static(__GLUE_VA_ARGS(__VA_ARGS__))>()); \
        return s_type_id; \
    }\
    const hdk::_PRIVATE::basic_type_info *__typeInfo() const { return hdk::init_type_info(this); } \
    operator const hdk::marshall_stream() const { return hdk::marshall_stream((void*)this, __typeInfo()); }

#define HDK_MARSHAL(...) __HDK_MARSHAL_IMPL(__VA_ARGS__)

#endif // __HDK_MACRO_HPP__
