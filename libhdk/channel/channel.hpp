#ifndef __HDK_CHANNEL_HPP__
#define __HDK_CHANNEL_HPP__

#include <map>
#include <string>
#include <memory>

#include "asio/io_context.hpp"

#include "brocker/brocker.hpp"
#include "channel/impl/channel_impl.hpp"
#include "marshall/marshall_macro.hpp"
#include "marshall/marshal.hpp"

namespace hdk
{
    struct recv_connection_status
    {
        bool connected;
        HDK_MARSHAL(connected)
    };
}

namespace hdk
{
    class channel
    {
    public:
        channel() = default;
        channel(const channel& pch);
        channel(channel&& pch);
        ~channel();

        channel& operator =(const channel& pch);
        channel& operator =(channel&& pch);

        static channel make_client_zmq(const std::string& address, const std::string& channel_name);
        static channel make_server_zmq(const std::string& address, const std::string& channel_name);

        static channel make_local(const std::string& channel_name);

        static channel make_client_asio(asio::io_context& ctx, const std::string& address, const std::string& channel_name);
        static channel make_server_asio(asio::io_context& ctx, const std::string& address, const std::string& channel_name);

        static channel get(const std::string& channel_name);

        static hdk::brocker::sender_info sender();

        template<typename T>
        static void send_current(const T& msg)
        {
            auto info = sender();
            get(info.name).send(msg, info.id);
        }

        template<typename T>
        void send(const T& msg, const std::string& receiver_id = "")
        {
            asio::streambuf out_buffer;
            std::iostream out_stream(&out_buffer);
            out_stream << msg;

            m_impl->send(msg.get_type_id(), hdk::to_byte_array(out_buffer), receiver_id);
        }

        void open();
        void close();
        bool connected() const;
        bool valid() const;

    private:
        std::shared_ptr<hdk::channel_impl> m_impl;
    };

} // namespace hdk

#endif // __HDK_CHANNEL_HPP__
