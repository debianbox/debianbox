#ifndef __HDK_CHANNEL_IMPL_LOCAL_HPP__
#define __HDK_CHANNEL_IMPL_LOCAL_HPP__

#include "marshall/marshal.hpp"
#include "channel/impl/channel_impl.hpp"

namespace hdk
{

    class channel_impl_local : public channel_impl
    {
    public:
        channel_impl_local(const std::string& channel_name);
        ~channel_impl_local();

        void open() override;
        void close() override;
        void send(uint32_t type_id, const byte_array &data, const std::string &reciever_id = "") override;
        bool connected() override;
    };

} // namespace hdk

#endif // __HDK_CHANNEL_IMPL_LOCAL_HPP__
