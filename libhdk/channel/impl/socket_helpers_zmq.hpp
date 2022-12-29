#ifndef __HDK_SOCKET_HELPERS_ZMQ_HPP__
#define __HDK_SOCKET_HELPERS_ZMQ_HPP__

#include <zmq_addon.hpp>

#include <sys/socket.h>
#include <netinet/in.h>
#include <netinet/ip.h>
#include <arpa/inet.h>

namespace hdk
{
    namespace _PRIVATE
    {

        zmq::send_result_t __send(void* sock_handle, zmq::message_t &msg, zmq::send_flags flags)
        {
            int nbytes = zmq_msg_send(msg.handle(), sock_handle, static_cast<int>(flags));
            if (nbytes >= 0)
                return static_cast<size_t>(nbytes);
            if (zmq_errno() == EAGAIN)
                return {};
            throw error_t();
        }

        ZMQ_NODISCARD
        zmq::recv_result_t __recv(void* sock_handle, zmq::message_t &msg, zmq::recv_flags flags = zmq::recv_flags::none)
        {
            const int nbytes = zmq_msg_recv(msg.handle(), sock_handle, static_cast<int>(flags));
            if (nbytes >= 0)
            {
                assert(msg.size() == static_cast<size_t>(nbytes));
                return static_cast<size_t>(nbytes);
            }
            if (zmq_errno() == EAGAIN)
                return {};
            throw error_t();
        }

        template<bool CheckN, class OutputIt>
        zmq::recv_result_t __recv_multipart_n(void* sock_handle, OutputIt out, size_t n, zmq::recv_flags flags)
        {
            size_t msg_count = 0;
            zmq::message_t msg;
            while (true)
            {
                if ZMQ_CONSTEXPR_IF (CheckN)
                {
                    if (msg_count >= n)
                        throw std::runtime_error(
                            "Too many message parts in recv_multipart_n");
                }
                if (!__recv(sock_handle, msg, flags)) {
                    // zmq ensures atomic delivery of messages
                    assert(msg_count == 0);
                    return {};
                }
                ++msg_count;
                const bool more = msg.more();
                *out++ = std::move(msg);
                if (!more)
                    break;
            }
            return msg_count;
        }
    } // namespace _PRIVATE

    namespace helpers
    {
        // Recv multipart message from zmq socket
        template<class OutputIt>
        ZMQ_NODISCARD zmq::recv_result_t recv_multipart(void* sock_handle, OutputIt out, zmq::recv_flags flags = zmq::recv_flags::none)
        {
                return _PRIVATE::__recv_multipart_n<false>(sock_handle, std::move(out), 0, flags);
        }

        // Send multipart message to zmq socket
        bool send_multipart(void* sock_handle, zmq::multipart_t& mmsg, int flags = 0)
        {
                flags &= ~(ZMQ_SNDMORE);
                bool more = mmsg.size() > 0;
                while (more)
                {
                    zmq::message_t message = mmsg.pop();
                    more = mmsg.size() > 0;
                    if(!_PRIVATE::__send(sock_handle, message, static_cast<zmq::send_flags>((more ? ZMQ_SNDMORE : 0) | flags)))
                        return false;
                }
                mmsg.clear();
                return true;
        }

        std::string get_peer_address(int fd)
        {
            sockaddr_in addr;
            socklen_t asize = sizeof(addr);
            getpeername(fd, (sockaddr*)&addr, &asize);

            std::string retval = inet_ntoa(addr.sin_addr);
            retval += ":" + std::to_string(htons(addr.sin_port));
            return retval;
        }

        std::string get_address(int fd)
        {
            sockaddr_in addr;
            socklen_t asize = sizeof(addr);
            getsockname(fd, (sockaddr*)&addr, &asize);

            std::string retval = inet_ntoa(addr.sin_addr);
            retval += ":" + std::to_string(htons(addr.sin_port));
            return retval;
        }

        int get_zmq_socket_type(void* sock_handle)
        {
            int sock_type;
            size_t sock_type_size = sizeof(sock_type);
            int rc = zmq_getsockopt(sock_handle, ZMQ_TYPE, &sock_type, &sock_type_size);
            if (rc == -1 && zmq_errno() == ETERM)
                    return -1;

            return sock_type;
        }

        int get_zmq_socket_fd(void* sock_handle)
        {
            int sock_fd;
            size_t sock_fd_size = sizeof(sock_fd);
            int rc = zmq_getsockopt(sock_handle, ZMQ_FD, &sock_fd, &sock_fd_size);
            if (rc == -1 && zmq_errno() == ETERM)
                    return -1;

            return sock_fd;
        }

        std::string beautify_address(const std::string &addr)
        {
            std::string retval = addr;
            const char* substr = "tcp://";
            retval.erase(0, strlen(substr));
            return retval;
        }

        } // namespace helpers

} // namespace hdk

#endif // __HDK_SOCKET_HELPERS_ZMQ_HPP__
