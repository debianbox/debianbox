#include "channel/channel.hpp"

#include "brocker/brocker_manager.hpp"
#include "channel/impl/channel_impl.hpp"
#include "channel/impl/channel_impl_zmq.hpp"
#include "channel/impl/channel_impl_asio.hpp"
#include "channel/impl/channel_impl_local.hpp"

#include <map>

namespace hdk
{

    namespace _PRIVATE
    {
        std::map<std::string, std::weak_ptr<channel_impl>> g_channels;
    }

    channel::channel(const channel &pch)
    {
        m_impl = pch.m_impl;
    }

    channel::channel(channel&& pch)
    {
        m_impl = pch.m_impl;
        pch.m_impl.reset();
    }

    channel::~channel()
    {
        if(m_impl.use_count() == 1)
            _PRIVATE::g_channels.erase(m_impl->name());
    }

    channel &channel::operator =(const channel &pch)
    {
        m_impl = pch.m_impl;
        return *this;
    }

    channel& channel::operator =(channel&& pch)
    {
        m_impl = pch.m_impl;
        pch.m_impl.reset();
        return *this;
    }

    channel channel::make_client_zmq(const std::string &address, const std::string &channel_name)
    {
        if(_PRIVATE::g_channels.find(channel_name) != _PRIVATE::g_channels.end())
        {
            std::cout << "Channel with name " << channel_name << " already exists!\n";
            return channel{};
        }

        channel new_channel;
        new_channel.m_impl = std::make_shared<channel_impl_zmq>(channel_name, address, zmq::socket_type::dealer);
        _PRIVATE::g_channels.insert( {channel_name, new_channel.m_impl} );
        return new_channel;
    }

    channel channel::make_server_zmq(const std::string &address, const std::string &channel_name)
    {
        if(_PRIVATE::g_channels.find(channel_name) != _PRIVATE::g_channels.end())
        {
            std::cout << "Channel with name " << channel_name << " already exists!\n";
            return channel{};
        }

        channel new_channel;
        new_channel.m_impl = std::make_shared<channel_impl_zmq>(channel_name, address, zmq::socket_type::router);
        _PRIVATE::g_channels.insert( {channel_name, new_channel.m_impl} );
        return new_channel;
    }

    channel channel::make_local(const std::string &channel_name)
    {
        if(_PRIVATE::g_channels.find(channel_name) != _PRIVATE::g_channels.end())
        {
            std::cout << "Channel with name " << channel_name << " already exists!\n";
            return channel{};
        }

        channel new_channel;
        new_channel.m_impl = std::make_shared<channel_impl_local>(channel_name);
        _PRIVATE::g_channels.insert( {channel_name, new_channel.m_impl} );
        return new_channel;
    }

    channel channel::make_client_asio(asio::io_context &ctx, const std::string &address, const std::string &channel_name)
    {
        channel_impl_asio* p;
    }

    channel channel::make_server_asio(asio::io_context &ctx, const std::string &address, const std::string &channel_name)
    {
        channel_impl_asio* p;
    }

    channel channel::get(const std::string &channel_name)
    {
        if(_PRIVATE::g_channels.find(channel_name) != _PRIVATE::g_channels.end())
        {
            channel new_channel;
            new_channel.m_impl = _PRIVATE::g_channels[channel_name].lock();
            return new_channel;
        }
        else
            return channel{};
    }

    brocker::sender_info channel::sender()
    {
        return hdk::brocker_manager::instance()->sender();
    }

    void channel::open()
    {
        if(m_impl.get())
            m_impl->open();
    }

    void channel::close()
    {
        if(m_impl.get())
            m_impl->close();
    }

    bool channel::connected() const
    {
        if(m_impl.get())
            return m_impl->connected();

        return false;
    }

    bool channel::valid() const
    {
        return m_impl ? true : false;
    }

} // namespace hdk
