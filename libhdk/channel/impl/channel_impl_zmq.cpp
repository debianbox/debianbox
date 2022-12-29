#include "channel/impl/channel_impl_zmq.hpp"
#include "channel/impl/socket_poller_zmq.hpp"

#include <string>
#include <iostream>

#include "zmq_addon.hpp"

namespace hdk
{
    channel_impl_zmq::channel_impl_zmq(const std::string& channel_name, const std::string& address, zmq::socket_type socket_type) :
    m_socket_type(socket_type), m_address(address)
    {
        name(channel_name);

        m_ctrl_socket = zmq::socket_t(socket_poller_zmq::instance()->zmq_context(), zmq::socket_type::dealer);
        m_ctrl_socket.connect("inproc://ctrl_socket");

        m_send_socket = zmq::socket_t(socket_poller_zmq::instance()->zmq_context(), zmq::socket_type::dealer);
        m_send_socket.connect("inproc://send_socket");

        __create_socket_sync();
    }

    channel_impl_zmq::~channel_impl_zmq()
    {
        __remove_socket_sync();
    }

    void channel_impl_zmq::__create_socket_sync()
    {
        std::mutex mtx;
        std::condition_variable cv;

        std::uintptr_t cv_ptr = reinterpret_cast<std::uintptr_t>(&cv);
        std::uintptr_t mutex_ptr = reinterpret_cast<std::uintptr_t>(&mtx);

        zmq::multipart_t mmsg;
        mmsg.addtyp(socket_poller_zmq::create_socket);
        mmsg.addstr(m_channel_name);
        mmsg.addtyp(m_socket_type);
        mmsg.addtyp(cv_ptr);
        mmsg.addtyp(mutex_ptr);
        mmsg.send(m_ctrl_socket, (int)zmq::send_flags::dontwait);

        std::unique_lock<std::mutex> lk(mtx);
        cv.wait(lk);

        zmq::message_t recv_msg;
        auto val = m_ctrl_socket.recv(recv_msg, zmq::recv_flags::none);
        if(val.has_value())
        {
            std::uintptr_t server_socket_ptr = *(std::uintptr_t*)recv_msg.data();
            m_zmq_socket_info_ptr = server_socket_ptr;
        }
    }

    void channel_impl_zmq::__open_socket_sync()
    {
        std::mutex mtx;
        std::condition_variable cv;

        std::uintptr_t cv_ptr = reinterpret_cast<std::uintptr_t>(&cv);
        std::uintptr_t mutex_ptr = reinterpret_cast<std::uintptr_t>(&mtx);

        zmq::multipart_t mmsg;
        mmsg.addtyp(socket_poller_zmq::open_socket);
        mmsg.addstr(m_address);
        mmsg.addtyp(m_zmq_socket_info_ptr);
        mmsg.addtyp(cv_ptr);
        mmsg.addtyp(mutex_ptr);
        mmsg.send(m_ctrl_socket, (int)zmq::send_flags::dontwait);

        std::unique_lock<std::mutex> lk(mtx);
        cv.wait(lk);
    }

    void channel_impl_zmq::__close_socket_sync()
    {
        std::mutex mtx;
        std::condition_variable cv;

        std::uintptr_t cv_ptr = reinterpret_cast<std::uintptr_t>(&cv);
        std::uintptr_t mutex_ptr = reinterpret_cast<std::uintptr_t>(&mtx);

        zmq::multipart_t mmsg;
        mmsg.addtyp(socket_poller_zmq::close_socket);
        mmsg.addtyp(m_zmq_socket_info_ptr);
        mmsg.addtyp(cv_ptr);
        mmsg.addtyp(mutex_ptr);
        mmsg.send(m_ctrl_socket, (int)zmq::send_flags::dontwait);

        std::unique_lock<std::mutex> lk(mtx);
        cv.wait(lk);
    }

    void channel_impl_zmq::__remove_socket_sync()
    {
        std::mutex mtx;
        std::condition_variable cv;

        std::uintptr_t cv_ptr = reinterpret_cast<std::uintptr_t>(&cv);
        std::uintptr_t mutex_ptr = reinterpret_cast<std::uintptr_t>(&mtx);

        zmq::multipart_t mmsg;
        mmsg.addtyp(socket_poller_zmq::remove_socket);
        mmsg.addtyp(m_zmq_socket_info_ptr);
        mmsg.addtyp(cv_ptr);
        mmsg.addtyp(mutex_ptr);
        mmsg.send(m_ctrl_socket, (int)zmq::send_flags::dontwait);

        std::unique_lock<std::mutex> lk(mtx);
        cv.wait(lk);
    }

    void channel_impl_zmq::open()
    {
        __open_socket_sync();
    }

    void channel_impl_zmq::close()
    {
        __close_socket_sync();
    }

    void channel_impl_zmq::send(uint32_t type_id, const byte_array &data, const std::string &peer)
    {
        zmq::multipart_t mmsg;

        mmsg.addtyp(socket_poller_zmq::socket_send_data);
        mmsg.addtyp(m_zmq_socket_info_ptr);
        mmsg.addstr(peer);
        mmsg.addtyp(type_id);
        mmsg.addmem(data.data(), data.size());

        mmsg.send(m_send_socket, (int)zmq::send_flags::dontwait);
    }

    bool channel_impl_zmq::connected()
    {
        return reinterpret_cast<socket_info_zmq*>(m_zmq_socket_info_ptr)->connected;
    }

} // namespace hdk

