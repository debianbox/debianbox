#include "channel/impl/socket_poller_zmq.hpp"
#include "channel/impl/socket_helpers_zmq.hpp"
#include "channel/impl/socket_info_zmq.hpp"
#include "channel/channel.hpp"

#include "brocker/brocker_manager.hpp"

#include <iostream>
#include <atomic>
#include <unordered_map>
#include <uuid/uuid.h>

#include "zmq_addon.hpp"

#define HDK_POLL_ITEM_SEND   (0u)
#define HDK_POLL_ITEM_CTRL   (1u)
#define HDK_POLL_ITEMS       (2u)

constexpr int c_connection_packet_size = 2;
constexpr int c_send_data_packet_size  = 6;

using namespace std::chrono_literals;

namespace hdk
{
    void print_zmq_msg(const std::vector<zmq::message_t>& zmq_msg_vec)
    {
        for(auto&& msg : zmq_msg_vec)
            std::cout << msg << "\n";
    }

    socket_poller_zmq::socket_poller_zmq()
    {
        startup();
    }

    void socket_poller_zmq::thread_poll()
    {
        try
        {
            std::unique_lock<std::mutex> lk(m_mutex_poll);

            m_send_socket =  zmq::socket_t(m_context, zmq::socket_type::router);
            m_send_socket.bind("inproc://send_socket");

            m_ctrl_socket =  zmq::socket_t(m_context, zmq::socket_type::router);
            m_ctrl_socket.bind("inproc://ctrl_socket");

            zmq::pollitem_t item;
            item.socket = m_send_socket;
            item.events = ZMQ_POLLIN;
            item.fd = 0;
            item.revents = 0;

            m_poll_items.push_back(item);

            item.socket = m_ctrl_socket;
            m_poll_items.push_back(item);

            lk.unlock();
            m_cv_poll.notify_one();

            while(1)
            {                
                zmq::poll(m_poll_items);
                {
                    // Socket IPC control
                    if(m_poll_items[HDK_POLL_ITEM_CTRL].revents & ZMQ_POLLIN)
                    {
                        parse_ctrl_data(m_poll_items[HDK_POLL_ITEM_CTRL].socket);
                        m_poll_items[HDK_POLL_ITEM_CTRL].revents = 0;
                    }

                    // Socket IPC send
                    if(m_poll_items[HDK_POLL_ITEM_SEND].revents & ZMQ_POLLIN)
                    {
                        parse_send_data(m_poll_items[HDK_POLL_ITEM_SEND].socket);
                        m_poll_items[HDK_POLL_ITEM_SEND].revents = 0;
                    }

                    // Check recv data
                    for(auto i = HDK_POLL_ITEMS; i < m_poll_items.size() - 1; i += 2)
                    {                        
                        // data
                        if(m_poll_items[i].revents & ZMQ_POLLIN)
                        {
                            parse_recv_data(m_poll_items[i].socket);
                            m_poll_items[i].revents = 0;
                        }

                        // events
                        if(m_poll_items[i + 1].revents & ZMQ_POLLIN)
                        {
                            parse_event_data(m_poll_items[i + 1].socket, m_poll_items[i].socket);
                            m_poll_items[i + 1].revents = 0;
                        }
                    }
                }
            }
        }
        catch(zmq::error_t err)
        {
            std::cout << err.what() << "\n";
            std::cout << "zmq poll thread exit\n";
        }
    }

    void socket_poller_zmq::parse_socket_create(std::vector<zmq::message_t> &msg)
    {        
        std::string sender = msg[0].to_string();
        std::string channel_name = msg[2].to_string();

        zmq::socket_type sock_type = *(zmq::socket_type*)msg[3].data();

        std::condition_variable* cv = reinterpret_cast<std::condition_variable*>( *(std::uintptr_t*)msg[4].data() );
        std::mutex* mtx = reinterpret_cast<std::mutex*>( *(std::uintptr_t*)msg[5].data() );

        // lock channel interface
        std::unique_lock<std::mutex> lk(*mtx);

        // create zmq::socket
        m_sockets.push_back(std::make_unique<socket_info_zmq>(channel_name, sock_type));
        socket_info_zmq* zmq_socket_info = m_sockets.rbegin()->get();

        // send data to channel interface
        std::uintptr_t sock_ptr = reinterpret_cast<std::uintptr_t>(zmq_socket_info);

        zmq::multipart_t mmsg;
        mmsg.addstr(sender);
        mmsg.addtyp(sock_ptr);
        mmsg.send(m_ctrl_socket, (int)zmq::send_flags::dontwait);

        // unlock channel interface
        lk.unlock();
        cv->notify_one();
    }

    void socket_poller_zmq::parse_socket_open(std::vector<zmq::message_t>& msg)
    {
        std::condition_variable* cv = reinterpret_cast<std::condition_variable*>( *(std::uintptr_t*)msg[4].data() );
        std::mutex* mtx = reinterpret_cast<std::mutex*>( *(std::uintptr_t*)msg[5].data() );

        // lock channel interface
        std::unique_lock<std::mutex> lk(*mtx);

        std::string address = msg[2].to_string();
        socket_info_zmq* zmq_socket_info = reinterpret_cast<socket_info_zmq*>( *(std::uintptr_t*)msg[3].data() );

        create_socket_info_zmq(zmq_socket_info);

        if(zmq_socket_info->socket_type == zmq::socket_type::router)
            zmq_socket_info->zmq_socket.bind(address);
        else
            zmq_socket_info->zmq_socket.connect(address);

        // unlock channel interface
        lk.unlock();
        cv->notify_one();
    }

    void socket_poller_zmq::parse_socket_close(std::vector<zmq::message_t>& msg)
    {
        std::condition_variable* cv = reinterpret_cast<std::condition_variable*>( *(std::uintptr_t*)msg[3].data() );
        std::mutex* mtx = reinterpret_cast<std::mutex*>( *(std::uintptr_t*)msg[4].data() );

        // lock channel interface
        std::unique_lock<std::mutex> lk(*mtx);

        socket_info_zmq* zmq_socket_info = reinterpret_cast<socket_info_zmq*>( *(std::uintptr_t*)msg[2].data() );
        remove_socket_info_zmq(zmq_socket_info);

        // unlock channel interface
        lk.unlock();
        cv->notify_one();
    }

    void socket_poller_zmq::parse_socket_remove(std::vector<zmq::message_t> &msg)
    {
        std::condition_variable* cv = reinterpret_cast<std::condition_variable*>( *(std::uintptr_t*)msg[3].data() );
        std::mutex* mtx = reinterpret_cast<std::mutex*>( *(std::uintptr_t*)msg[4].data() );

        // lock channel interface
        std::unique_lock<std::mutex> lk(*mtx);

        socket_info_zmq* zmq_socket_info = reinterpret_cast<socket_info_zmq*>( *(std::uintptr_t*)msg[2].data() );

        if(zmq_socket_info->zmq_socket.handle())
            remove_socket_info_zmq(zmq_socket_info);

        // remove zmq_socket_info
        auto socket_it = std::find_if(m_sockets.begin(), m_sockets.end(),
                       [&zmq_socket_info](const auto& sock_info)
        {
            return sock_info.get() == zmq_socket_info;
        });


        m_sockets.erase(socket_it);

        // unlock channel interface
        lk.unlock();
        cv->notify_one();
    }

    void socket_poller_zmq::create_socket_info_zmq(socket_info_zmq *zmq_socket_info)
    {
        if(!zmq_socket_info)
            return;

        zmq_socket_info->zmq_socket = zmq::socket_t(m_context, zmq_socket_info->socket_type);
        zmq_socket_info->fd = zmq_socket_info->zmq_socket.get(zmq::sockopt::fd);

        zmq_socket_info->zmq_socket.set(zmq::sockopt::immediate,         1);
        zmq_socket_info->zmq_socket.set(zmq::sockopt::heartbeat_ivl,     1000);
        zmq_socket_info->zmq_socket.set(zmq::sockopt::heartbeat_timeout, 10000);
        zmq_socket_info->zmq_socket.set(zmq::sockopt::heartbeat_ttl,     10000);
        zmq_socket_info->zmq_socket.set(zmq::sockopt::handshake_ivl,     5000);
        zmq_socket_info->zmq_socket.set(zmq::sockopt::linger,            0);
        zmq_socket_info->zmq_socket.set(zmq::sockopt::rcvtimeo,          1000);

        if(zmq_socket_info->socket_type == zmq::socket_type::router)
            zmq_socket_info->zmq_socket.set(zmq::sockopt::router_mandatory, 1);
        else
        {
            uuid_t routing_id;
            uuid_generate(routing_id);
            zmq_socket_info->zmq_socket.set(zmq::sockopt::routing_id, reinterpret_cast<const char*>(routing_id));
        }

        // create monitor socket
        std::string monitor_address = "inproc://" + zmq_socket_info->channel_name + "_monitor";
        int rc = zmq_socket_monitor(zmq_socket_info->zmq_socket.handle(), monitor_address.c_str(),
                                    ZMQ_EVENT_CONNECTED | ZMQ_EVENT_DISCONNECTED);
        if (rc != 0)
        {
            std::cout << "monitor socket: " << monitor_address << " error: " << rc << "\n";
            throw error_t();
        }

        zmq_socket_info->zmq_monitor = zmq::socket_t(m_context, zmq::socket_type::pair);
        zmq_socket_info->zmq_monitor.connect(monitor_address);

        // add sockets to poll vector
        zmq::pollitem_t item;
        item.socket = zmq_socket_info->zmq_socket;
        item.events = ZMQ_POLLIN;
        item.fd = 0;
        item.revents = 0;
        m_poll_items.push_back(item);

        item.socket = zmq_socket_info->zmq_monitor;
        m_poll_items.push_back(item);
    }

    void socket_poller_zmq::remove_socket_info_zmq(socket_info_zmq *zmq_socket_info)
    {
        if(!zmq_socket_info)
            return;

        // remove from zmq_poll_item vector
        auto poll_item_it = std::find_if(m_poll_items.begin(), m_poll_items.end(),
                                         [&zmq_socket_info](const zmq_pollitem_t& poll_item)
                                         {
                                             return poll_item.socket == zmq_socket_info->zmq_socket.handle();
                                         });

        if(poll_item_it != m_poll_items.end())
            m_poll_items.erase(poll_item_it, poll_item_it + 2);

        // stop zmq_monitor
        zmq_socket_monitor(zmq_socket_info->zmq_socket.handle(), ZMQ_NULLPTR, 0);

        zmq_socket_info->zmq_monitor.close();
        zmq_socket_info->zmq_socket.close();
        zmq_socket_info->connected = false;
        zmq_socket_info->peers.clear();
    }

    void socket_poller_zmq::parse_ctrl_data(void *sock_handle)
    {
        std::vector<zmq::message_t> zmq_msg_vec;
        auto res = hdk::helpers::recv_multipart(sock_handle, std::back_inserter(zmq_msg_vec), zmq::recv_flags::dontwait);
        if(res.has_value())
        {
            if(zmq_msg_vec.size() < 2)
            {
                std::cout << "ctrl packet size < 2\n";
                return;
            }
            socket_poller_cmd_t cmd = *(socket_poller_cmd_t*)zmq_msg_vec[1].data();

            switch (cmd)
            {
            case create_socket:
                parse_socket_create(zmq_msg_vec);
                break;
            case open_socket:
                parse_socket_open(zmq_msg_vec);
                break;
            case close_socket:
                parse_socket_close(zmq_msg_vec);
                break;
            case remove_socket:
                parse_socket_remove(zmq_msg_vec);
                break;
            default:
                break;
            }
        }
    }

    void socket_poller_zmq::parse_send_data(void *sock_handle)
    {
        std::vector<zmq::message_t> zmq_msg_vec;
        auto res = hdk::helpers::recv_multipart(sock_handle, std::back_inserter(zmq_msg_vec), zmq::recv_flags::dontwait);
        if(res.has_value())
        {
            if(zmq_msg_vec.size() != c_send_data_packet_size)
            {
                std::cout << "send packet size < " << c_send_data_packet_size << "\n";
                return;
            }

            if(*(socket_poller_cmd_t*)zmq_msg_vec[1].data() != socket_send_data)
                return;

            socket_info_zmq* zmq_socket_info = reinterpret_cast<socket_info_zmq*>( *(std::uintptr_t*)zmq_msg_vec[2].data() );

            auto send_peer = zmq_msg_vec[3].to_string();
            uint32_t type_id = *(uint32_t*)zmq_msg_vec[4].data();

            if(send_peer.empty())
            {
                if(zmq_socket_info->socket_type == zmq::socket_type::router) // send to all router peers
                {
                    std::for_each(zmq_socket_info->peers.begin(), zmq_socket_info->peers.end(),
                                  [&zmq_socket_info, &type_id, &zmq_msg_vec](const socket_info_zmq::peer_info& info)
                    {
                        zmq::multipart_t mmsg;
                        mmsg.addstr(info.peer_id);
                        mmsg.addtyp(socket_recv_data);
                        mmsg.addtyp(type_id);
                        mmsg.addmem(zmq_msg_vec[5].data(), zmq_msg_vec[5].size());
                        mmsg.send(zmq_socket_info->zmq_socket, (int)zmq::send_flags::dontwait);
                    });
                }
                else // just dealer client send routine
                {
                    zmq::multipart_t mmsg;
                    mmsg.addtyp(socket_recv_data);
                    mmsg.addtyp(type_id);
                    mmsg.addmem(zmq_msg_vec[5].data(), zmq_msg_vec[5].size());
                    mmsg.send(zmq_socket_info->zmq_socket, (int)zmq::send_flags::dontwait);
                }
            }
            else // send to selected peer
            {
                zmq::multipart_t mmsg;
                mmsg.addstr(send_peer);
                mmsg.addtyp(socket_recv_data);
                mmsg.addtyp(type_id);
                mmsg.addmem(zmq_msg_vec[5].data(), zmq_msg_vec[5].size());
                mmsg.send(zmq_socket_info->zmq_socket, (int)zmq::send_flags::dontwait);
            }
        }
    }

    void socket_poller_zmq::parse_recv_data(void *sock_handle)
    {
        std::vector<zmq::message_t> zmq_msg_vec;
        auto res = helpers::recv_multipart(sock_handle, std::back_inserter(zmq_msg_vec), zmq::recv_flags::dontwait);

        if(res.has_value())
        {
            if(zmq_msg_vec.size() < c_connection_packet_size) // connection notification message
            {
                std::cout << "recv packet size < " << c_connection_packet_size << "\n";
                return;
            }

            std::string peer_id;
            int msg_pos {1};

            if(helpers::get_zmq_socket_type(sock_handle) == ZMQ_DEALER) // dealer auto removes zmq_msg_vec[0] with peer id
            {
                msg_pos = 0;
                peer_id.push_back(0);
            }
            else // router has zmq_msg_vec[0] with peer id
                peer_id = zmq_msg_vec[0].to_string();

            int peer_fd = zmq_msg_get(zmq_msg_vec[1].handle(), ZMQ_SRCFD);
            std::string peer_addr = helpers::get_peer_address(peer_fd);

            auto zmq_sock_info = get_zmq_socket_info(sock_handle);

            // DEALER sends connection info to ROUTER
            if(*(socket_poller_cmd_t*)zmq_msg_vec[msg_pos].data() == socket_connected)
            {
                if(zmq_sock_info)
                {
                    zmq_sock_info->add(socket_info_zmq::peer_info(peer_id, peer_addr, peer_fd));
                    send_connection_state_msg(zmq_sock_info->channel_name, true, peer_id, peer_addr);
                }
            }

            // General data recv routine
            if(*(socket_poller_cmd_t*)zmq_msg_vec[msg_pos].data() == socket_recv_data)
            {
                uint32_t type_id = *(uint32_t*)zmq_msg_vec[msg_pos + 1].data();

                hdk::byte_array msg_data;
                msg_data.resize(zmq_msg_vec[msg_pos + 2].size());
                memcpy(msg_data.data(), (const char*)zmq_msg_vec[msg_pos + 2].data(), zmq_msg_vec[msg_pos + 2].size());

                if(zmq_sock_info)
                    brocker_manager::instance()->post(type_id, msg_data, zmq_sock_info->channel_name, peer_id, peer_addr);
            }
        }
    }

    void socket_poller_zmq::parse_event_data(void *sock_handle_event, void* sock_handle)
    {
        zmq::message_t event_msg;

        int rc = zmq_msg_recv(event_msg.handle(), sock_handle_event, 0);
        if (rc == -1 && zmq_errno() == ETERM)
            return;

        const char *data = static_cast<const char *>(event_msg.data());
        zmq_event_t zmq_event;
        memcpy(&zmq_event.event, data, sizeof(uint16_t));
        data += sizeof(uint16_t);
        memcpy(&zmq_event.value, data, sizeof(int32_t));

        zmq::message_t addr_msg;
        rc = zmq_msg_recv(addr_msg.handle(), sock_handle_event, 0);
        if (rc == -1 && zmq_errno() == ETERM)
            return;

        auto zmq_sock_info = get_zmq_socket_info(sock_handle);

        switch(zmq_event.event)
        {
        case ZMQ_EVENT_CONNECTED:
        {
            // for client only
            zmq::multipart_t mmsg;
            mmsg.addtyp(socket_connected);
            helpers::send_multipart(sock_handle, mmsg, (int)zmq::send_flags::none);             

            connection_state(zmq_sock_info, true);
            send_connection_state_msg(zmq_sock_info->channel_name, true, "", helpers::beautify_address(addr_msg.to_string()));

            break;
        }
        case ZMQ_EVENT_DISCONNECTED:
        {
            if(helpers::get_zmq_socket_type(sock_handle) == ZMQ_ROUTER)
                remove_peer_by_fd(zmq_sock_info, zmq_event.value); // server set connected flag when peers count reaches zero (inside remove_peer_by_fd())
            else
            {
                // for client only
                connection_state(zmq_sock_info, false);
                send_connection_state_msg(zmq_sock_info->channel_name, false, "", helpers::beautify_address(addr_msg.to_string()));
            }

            break;
        }
        default:
            break;
        }
    }

    void socket_poller_zmq::remove_peer_by_fd(socket_info_zmq *zmq_socket_info, int fd)
    {
        if(zmq_socket_info)
        {
            auto info = zmq_socket_info->remove_by_fd(fd);
            send_connection_state_msg(zmq_socket_info->channel_name, false, info.peer_id, info.peer_address);
        }
    }

    void socket_poller_zmq::connection_state(socket_info_zmq *zmq_socket_info, bool state)
    {
        if(zmq_socket_info)
            zmq_socket_info->connected = state;
    }

    bool socket_poller_zmq::connection_state(socket_info_zmq *zmq_socket_info) const
    {
        if(zmq_socket_info)
            return zmq_socket_info->connected;

        return false;
    }

    socket_info_zmq *socket_poller_zmq::get_zmq_socket_info(void *sock_handle)
    {
        auto sock_it = std::find_if(m_sockets.begin(), m_sockets.end(),
                                    [&sock_handle](const auto& sock_info)
                                    {
                                        return sock_info->zmq_socket.handle() == sock_handle;
                                    });

        if(sock_it != m_sockets.end())
            return sock_it->get();

        return nullptr;
    }

    void socket_poller_zmq::send_connection_state_msg(const std::string channel_name, bool state, const std::string peer_id, const std::string peer_address)
    {
        hdk::recv_connection_status msg;
        msg.connected = state;

        asio::streambuf out_buffer;
        std::iostream out_stream(&out_buffer);
        out_stream << msg;

        brocker_manager::instance()->post(msg.get_type_id(), hdk::to_byte_array(out_buffer),
                                                 channel_name, peer_id, peer_address, false);
    }

    void socket_poller_zmq::startup()
    {
        std::unique_lock<std::mutex> lk(m_mutex_poll);

        m_poller_thread = std::thread([this]
        {
            thread_poll();
        });

        m_cv_poll.wait(lk);
    }

    void socket_poller_zmq::shutdown()
    {
        m_context.shutdown();
        m_poller_thread.join();
    }

    zmq::context_t &socket_poller_zmq::zmq_context()
    {
        return m_context;
    }

} // namespace hdk
