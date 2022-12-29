#include "brocker/brocker_manager.hpp"

#include "asio/io_context.hpp"
#include "asio/dispatch.hpp"

namespace hdk
{
    thread_local hdk::brocker::sender_info s_sender_info;

    void brocker_manager::remove(brocker *broker_)
    {
        std::unique_lock lock(m_reg_brockers_mutex); // write lock
        for(auto map_it = m_reg_brokers_map.begin(); map_it != m_reg_brokers_map.end(); )
        {
            for(auto vec_it = map_it->second.begin(); vec_it != map_it->second.end(); )
            {
                if(*vec_it == broker_)
                {
                    vec_it = map_it->second.erase(vec_it);
                    if(map_it->second.empty())
                        map_it = m_reg_brokers_map.erase(map_it);
                    else
                        ++map_it;
                }
                else
                    ++vec_it;
            }
        }
    }

    void brocker_manager::reg(brocker *broker_, uint32_t type_id)
    {
        std::unique_lock lock(m_reg_brockers_mutex); // write lock
        __reg(broker_, type_id);
    }

    void brocker_manager::unreg(brocker *broker_, uint32_t type_id)
    {
        std::unique_lock lock(m_reg_brockers_mutex); // write lock
        __unreg(broker_, type_id);
    }

    void brocker_manager::clone(brocker *brocker_from, brocker *brocker_to)
    {
        std::unique_lock lock(m_reg_brockers_mutex); // write lock
        for(auto map_it = m_reg_brokers_map.begin(); map_it != m_reg_brokers_map.end(); ++map_it)
        {
            auto& brockers = map_it->second;
            if(std::find(brockers.begin(), brockers.end(), brocker_from) != std::end(brockers))
            {
                brockers.push_back(brocker_to);
                continue;
            }
        }
    }

    void brocker_manager::post(uint32_t type_id, const byte_array &data, const std::string& filter,
                                      const std::string &sender_name, const std::string &sender_address, bool check_reg)
    {
        std::shared_lock lock(m_reg_brockers_mutex); // read lock
        auto it_brocker = m_reg_brokers_map.find(type_id);
        if(it_brocker != m_reg_brokers_map.end())
        {
            const std::list<brocker*> &brokers = it_brocker->second;
            std::for_each(brokers.cbegin(), brokers.cend(),
                          [&filter, &type_id, &data, &sender_name, &sender_address](brocker* broker_)
                          {
                              if(broker_->__check_filter(filter))
                              {
                                  asio::dispatch(broker_->context(),
                                                 [broker_, type_id, data, sender_name, sender_address, filter]()
                                                 {
                                                     __sender_info( {sender_name, filter, sender_address} );
                                                     broker_->__call(type_id, data);
                                                 });
                              }
                          });
        }
        else
        {
            if(check_reg)
                std::cout << "libhdk: Unknown type_id [" << std::hex << type_id << "]\n";
        }
    }

    brocker::sender_info brocker_manager::sender()
    {
        return s_sender_info;
    }
    
    void brocker_manager::__reg(brocker* brocker_, uint32_t type_id)
    {
        std::list<brocker*> &brockers = m_reg_brokers_map[type_id];

        if(std::find(brockers.begin(), brockers.end(), brocker_) != std::end(brockers))
            return;

        brockers.push_back(brocker_);
    }

    void brocker_manager::__unreg(brocker *brocker_, uint32_t type_id)
    {
        if(m_reg_brokers_map.find(type_id) != m_reg_brokers_map.end())
        {
            std::list<brocker*> &brockers = m_reg_brokers_map[type_id];
            for(auto it = brockers.begin(); it != brockers.end(); ++it)
            {
                if(*it == brocker_)
                {
                    brockers.erase(it);
                    if(!brockers.size()) // all brockers were removed
                        m_reg_brokers_map.erase(type_id);

                    return;
                }
            }
        }
    }

    void brocker_manager::__sender_info(brocker::sender_info &&info)
    {
        s_sender_info = std::move(info);
    }

} // namespace hdk

