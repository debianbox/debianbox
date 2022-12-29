#ifndef __HDK_BROCKER_MANAGER_HPP__
#define __HDK_BROCKER_MANAGER_HPP__

#include <unordered_map>
#include <shared_mutex>

#include "singleton.hpp"
#include "brocker/brocker.hpp"

namespace hdk
{

    class brocker_manager
    {
        public:
            HDK_SINGLETON(brocker_manager)

            void remove(brocker* broker_);

            void reg(brocker* broker_, uint32_t type_id);
            void unreg(brocker* broker_, uint32_t type_id);
            void clone(brocker* brocker_from, brocker *brocker_to);

            void post(uint32_t type_id, const hdk::byte_array& data, const std::string &filter = "",
                      const std::string& sender_name = "", const std::string& sender_address = "", bool check_reg = true);

            brocker::sender_info sender();

        private:
            brocker_manager() = default;

            mutable std::shared_mutex m_reg_brockers_mutex;
            std::unordered_map<uint32_t, std::list<brocker*>> m_reg_brokers_map;

            void __reg(brocker* brocker_, uint32_t type_id);
            void __unreg(brocker* brocker_, uint32_t type_id);

            static void __sender_info(brocker::sender_info&& info);
    };

} // namespace hdk

#endif // __HDK_BROCKER_MANAGER_HPP__
