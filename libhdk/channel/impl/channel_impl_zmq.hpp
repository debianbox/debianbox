#ifndef __HDK_CHANNEL_IMPL_ZMQ_HPP__
#define __HDK_CHANNEL_IMPL_ZMQ_HPP__

#include <unordered_map>
#include <string>
#include <functional>
#include <shared_mutex>

#include "zmq.hpp"

#include "marshall/marshal.hpp"
#include "channel/impl/channel_impl.hpp"

namespace hdk
{
    class channel_impl_zmq : public channel_impl
    {
    public:
        void open() override;
        void close() override;
        void send(uint32_t type_id, const byte_array& data, const std::string& reciever_id = "") override;

        bool connected() override;

        channel_impl_zmq(const std::string& channel_name, const std::string& address, zmq::socket_type socket_type);
        ~channel_impl_zmq();

    private:
        std::string m_address;

        zmq::socket_t m_send_socket;
        zmq::socket_t m_ctrl_socket;

        zmq::socket_type m_socket_type;

        std::vector<std::string> m_connected_peers;
        std::uintptr_t m_zmq_socket_info_ptr;

        void __create_socket_sync();
        void __open_socket_sync();
        void __close_socket_sync();
        void __remove_socket_sync();
    };

} // namespace hdk

#endif // __HDK_CHANNEL_IMPL_ZMQ_HPP__

