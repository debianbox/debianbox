#ifndef __HDK_CHANNEL_IMPL_HPP__
#define __HDK_CHANNEL_IMPL_HPP__

#include "marshall/marshal.hpp"

namespace hdk
{
    
    class channel_impl
    {
    public:
        friend class channel;

        channel_impl() = default;
        virtual ~channel_impl() = default;

        std::string name() const;
        void name(const std::string& channel_name);

    protected:
        virtual void open() = 0;
        virtual void close() = 0;
        virtual void send(uint32_t type_id, const byte_array& data, const std::string& reciever_id = "") = 0;
        virtual bool connected() = 0;

        std::string m_channel_name;

    };

} // namespace hdk

#endif // __HDK_CHANNEL_IMPL_HPP__
