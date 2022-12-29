#ifndef __HDK_SOCKET_INFO_ZMQ_HPP__
#define __HDK_SOCKET_INFO_ZMQ_HPP__

#include "zmq.hpp"

#include <vector>
#include <iostream>
#include <atomic>

namespace hdk
{

    struct socket_info_zmq
    {
        struct peer_info
        {
            peer_info() {}
            peer_info(const std::string& id, const std::string& addr, int fd) : peer_id(id), peer_address(addr), peer_fd(fd)
            {}

            bool operator==(const peer_info& other) { return peer_id == other.peer_id && peer_address == other.peer_address; }

            std::string peer_id;
            std::string peer_address;
            int peer_fd;
        };

        socket_info_zmq() : connected(false) {}
        socket_info_zmq(const std::string& name, zmq::socket_type sock_type)
            : channel_name(name), socket_type(sock_type), connected(false) {}

        inline void add(const peer_info& nfo)
        {
            if(!peers.size())
                connected = true;

            peers.push_back(nfo);
        }

        inline void remove(const peer_info& nfo) { peers.erase(std::find(peers.begin(), peers.end(), nfo)); }

        inline void remove_by_addr(const std::string& peer_addr)
        {
            auto it = std::find_if(peers.begin(), peers.end(),
                                   [&peer_addr]
                                   (const peer_info& nfo){ return nfo.peer_address == peer_addr; });

            if(it != peers.end())
                peers.erase(it);


            if(!peers.size())
                connected = false;
        }

        inline void remove_by_id(const std::string& peer_id)
        {
            auto it = std::find_if(peers.begin(), peers.end(),
                                   [&peer_id]
                                   (const peer_info& nfo){ return nfo.peer_id == peer_id; });
            if(it != peers.end())
                peers.erase(it);


            if(!peers.size())
                connected = false;
        }

        inline peer_info remove_by_fd(int peer_fd)
        {
            peer_info retval;

            auto it = std::find_if(peers.begin(), peers.end(),
                                   [&peer_fd] (const peer_info& nfo) { return nfo.peer_fd == peer_fd; });

            if(it != peers.end())
            {
                retval = *it;
                peers.erase(it);
            }

            if(!peers.size())
                connected = false;

            return retval;
        }

        std::string channel_name;
        zmq::socket_t zmq_socket;
        zmq::socket_t zmq_monitor;
        zmq::socket_type socket_type;
        std::vector<peer_info> peers;
        std::atomic<bool> connected {false};
        int fd;
    };

} // namespace hdk

#endif // __HDK_SOCKET_INFO_ZMQ_HPP__
