#ifndef __HDK_MARSHALL_STREAM_HPP__
#define __HDK_MARSHALL_STREAM_HPP__

#include <iostream>

#include "marshall/marshall_type_info.hpp"

namespace hdk
{
    class marshall_stream
    {
    public:
        marshall_stream(void* proto_struct, const hdk::_PRIVATE::basic_type_info* type_info) :
            m_proto_struct(proto_struct),
            m_type_info(type_info)
        {
            m_type_info->type_id(m_proto_struct);
        }

        void marshal(std::ostream& os) const
        {
            m_type_info->marshal(os, m_proto_struct);
        }
        void demarshal(std::istream& is) const
        {
            m_type_info->demarshal(is, m_proto_struct);
        }

    private:
        const hdk::_PRIVATE::basic_type_info* m_type_info = nullptr;
        void* m_proto_struct = nullptr;
    };
}

inline std::ostream& operator <<(std::ostream& os, const hdk::marshall_stream& ps)
{
    ps.marshal(os);
    return os;
}

inline std::istream& operator >>(std::istream& is, const hdk::marshall_stream& ps)
{
    ps.demarshal(is);
    return is;
}

#endif // __HDK_MARSHALL_STREAM_HPP__
