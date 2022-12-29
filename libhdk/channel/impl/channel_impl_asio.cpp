#include "channel/impl/channel_impl_asio.hpp"

namespace hdk
{
    channel_impl_asio::channel_impl_asio(asio::io_context& ctx, const std::string& channel_name, const std::string& address, int socket_type) :
        m_context(ctx)
    {
        name(channel_name);
    }

    channel_impl_asio::~channel_impl_asio()
    {

    }

    void channel_impl_asio::open()
    {

    }

    void channel_impl_asio::close()
    {

    }

    void channel_impl_asio::send(uint32_t type_id, const byte_array &data, const std::string &peer)
    {

    }

    bool channel_impl_asio::connected()
    {

    }
} // namespace hdk
