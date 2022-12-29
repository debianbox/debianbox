#ifndef __HDK_MARSHAL_H__
#define __HDK_MARSHAL_H__

#include <iostream>
#include <string>
#include <vector>

#include "asio/streambuf.hpp"

namespace hdk
{
    using byte_array = std::vector<uint8_t>;

    inline byte_array to_byte_array(const asio::streambuf& stream_buf)
    {
        byte_array ret;
        ret.resize(stream_buf.size());
        memcpy(ret.data(), stream_buf.data().data(), stream_buf.size());
        return ret;
    }

    inline void from_byte_array(asio::streambuf& stream_buf, const byte_array& byte_array_)
    {
        stream_buf.prepare(byte_array_.size());
        memcpy((void*)stream_buf.data().data(), byte_array_.data(), byte_array_.size());
        stream_buf.commit(byte_array_.size());
    }

    namespace _PRIVATE
    {
        template<typename ...Args>
        inline void _final(const Args...) {}

        template <typename Arg>
        inline bool _marshal(std::ostream& os, const Arg& arg)
        {            
            os.write((char*)&arg, sizeof(Arg));
            return true;
        }

        template <typename Arg>
        inline bool _demarshal(std::istream& is, const Arg& arg)
        {
            is.read((char*)&arg, sizeof(Arg));
            return true;
        }

        template<>
        inline bool _marshal<std::string>(std::ostream& os, const std::string& value)
        {
            _marshal(os, value.size());
            os.write(value.c_str(), value.size());
            return true;
        }

        template <>
        inline bool _demarshal<std::string>(std::istream& is, const std::string& value)
        {
            size_t size = 0;
            _demarshal(is, size);
            ((std::string&)value).resize(size);
            is.read((char*)value.data(), size);
            return true;
        }

        template<typename Arg>
        inline bool _marshal(std::ostream& os, const std::vector<Arg>& v)
        {
            _marshal(os, v.size());
            std::for_each(v.cbegin(), v.cend(), [&os](const Arg& value) { _marshal(os, value); });
            return true;
        }

        template <typename Arg>
        inline bool _demarshal(std::istream& is, const std::vector<Arg>& v)
        {
            size_t size = 0;
            _demarshal(is, size);
            ((std::vector<Arg>&)v).reserve(size);
            for(size_t i = 0; i < size; ++i)
            {
                Arg value;
                _demarshal(is, value);
                ((std::vector<Arg>&)v).push_back(value);
            }
            return true;
        }

        template<typename ArgKey, typename ArgValue>
        inline bool _marshal(std::ostream& os, const std::map<ArgKey, ArgValue>& m)
        {
            _marshal(os, m.size());
            std::for_each(m.cbegin(), m.cend(), [&os](auto& map_value)
                          {
                              _marshal(os, map_value.first);
                              _marshal(os, map_value.second);
                          });

            return true;
        }

        template<typename ArgKey, typename ArgValue>
        inline bool _demarshal(std::istream& is, const std::map<ArgKey, ArgValue>& m)
        {
            size_t size = 0;
            std::map<ArgKey, ArgValue>& mm = (std::map<ArgKey, ArgValue>&)m;
            _demarshal(is, size);

            for(size_t i = 0; i < size; ++i)
            {
                ArgKey key;
                ArgValue value;
                _demarshal(is, key);
                _demarshal(is, value);
                mm.insert( {key, value} );
            }
            return true;
        }

        template<typename ...Args>
        inline void marshal(std::ostream& os, const Args&... args)
        {
            _PRIVATE::_final( _PRIVATE::_marshal(os, args) ... );
        }

        template<typename ...Args>
        inline void demarshal(std::istream& is, const Args&... args)
        {
            _PRIVATE::_final( _PRIVATE::_demarshal(is, args) ... );
        }
    }
} // namespace hdk
#endif // __HDK_MARSHAL_HPP__
