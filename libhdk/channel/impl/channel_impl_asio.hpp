#ifndef __HDK_CHANNEL_IMPL_ASIO_HPP__
#define __HDK_CHANNEL_IMPL_ASIO_HPP__

#include "asio/io_context.hpp"

#include "marshall/marshal.hpp"
#include "channel/impl/channel_impl.hpp"

namespace hdk
{
    class channel_impl_asio : public channel_impl
    {
    public:
        channel_impl_asio(asio::io_context& ctx, const std::string& channel_name, const std::string& address, int socket_type);
        ~channel_impl_asio();

        void open() override;
        void close() override;
        void send(uint32_t type_id, const byte_array& data, const std::string& reciever_id = "") override;
        bool connected() override;

    private:
        asio::io_context& m_context;
    };

} // namespace hdk

#endif // __HDK_CHANNEL_IMPL_ASIO_HPP__
