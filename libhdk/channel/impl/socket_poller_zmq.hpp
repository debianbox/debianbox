#ifndef __HDK_SOCKET_POLLER_ZMQ_HPP__
#define __HDK_SOCKET_POLLER_ZMQ_HPP__

#include <vector>
#include <thread>
#include <mutex>

#include "zmq.hpp"

#include "singleton.hpp"
#include "channel/impl/socket_info_zmq.hpp"

namespace hdk
{
    class socket_poller_zmq
    {
    public:
        HDK_SINGLETON(socket_poller_zmq)

        void startup();
        void shutdown();

        zmq::context_t& zmq_context();

        enum socket_poller_cmd_t : uint32_t
        {
            create_socket,
            open_socket,
            close_socket,
            remove_socket,
            socket_connected,
            socket_send_data,
            socket_recv_data,
        };

    private:
        socket_poller_zmq();

        zmq::context_t m_context;

        std::thread m_poller_thread;

        std::mutex m_mutex_poll;
        std::condition_variable m_cv_poll;

        std::list<std::unique_ptr<socket_info_zmq>> m_sockets;

        std::vector<zmq::pollitem_t> m_poll_items;

        zmq::socket_t m_send_socket;
        zmq::socket_t m_ctrl_socket;

        void thread_poll();

        void parse_socket_create(std::vector<zmq::message_t>& msg);
        void parse_socket_open(std::vector<zmq::message_t>& msg);
        void parse_socket_close(std::vector<zmq::message_t>& msg);
        void parse_socket_remove(std::vector<zmq::message_t>& msg);

        void create_socket_info_zmq(socket_info_zmq* zmq_socket_info);
        void remove_socket_info_zmq(socket_info_zmq* zmq_socket_info);

        void parse_ctrl_data(void* sock_handle);
        void parse_send_data(void* sock_handle);
        void parse_recv_data(void* sock_handle);
        void parse_event_data(void *sock_handle_event, void* sock_handle);

        void remove_peer_by_fd(socket_info_zmq* zmq_socket_info, int fd);

        void connection_state(socket_info_zmq* zmq_socket_info, bool state);
        bool connection_state(socket_info_zmq* zmq_socket_info) const;

        socket_info_zmq* get_zmq_socket_info(void* sock_handle);

        void send_connection_state_msg(const std::string channel_name, bool state, const std::string peer_id, const std::string peer_address);
    };

} // namespace hdk

#endif // __HDK_SOCKET_POLLER_ZMQ_HPP__
