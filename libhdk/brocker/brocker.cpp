#include "brocker/brocker.hpp"
#include "brocker/brocker_manager.hpp"

namespace hdk
{

    brocker::brocker(asio::io_context& ctx) :  m_io_context(ctx)
    {
    }

    brocker& brocker::filter(std::initializer_list<std::string> filters)
    {
        m_filters = filters;
        return *this;
    }

    brocker& brocker::filter(const std::vector<std::string> &filters)
    {
        m_filters = filters;
        return *this;
    }

    brocker& brocker::filter(const std::string &filter)
    {
        m_filters.clear();
        m_filters.push_back(filter);
        return *this;
    }

    brocker::~brocker()
    {
        brocker_manager::instance()->remove(this);
    }

    brocker::brocker(const brocker &brocker_): m_io_context(brocker_.m_io_context)
    {
        m_calls = brocker_.m_calls;
        m_filters = brocker_.m_filters;
        brocker_manager::instance()->clone(const_cast<brocker*>(&brocker_), this);
    }


    asio::io_context& brocker::context()
    {
        return m_io_context;
    }

    void brocker::post(uint32_t type_id, const byte_array &data, const std::string& filter,
                              const std::string &sender_name, const std::string &sender_address)
    {
        brocker_manager::instance()->post(type_id, data, filter, sender_name, sender_address);
    }

    void brocker::__call(uint32_t type_id, const byte_array &data) const
    {
        if(m_calls.find(type_id) != m_calls.end())
            m_calls.at(type_id)->call(data);
    }

    void brocker::__reg(uint32_t type_id)
    {
        brocker_manager::instance()->reg(this, type_id);
    }

    void brocker::__unreg(uint32_t type_id)
    {
        brocker_manager::instance()->unreg(this, type_id);
    }

    bool brocker::__check_filter(const std::string &filter) const
    {
        if(m_filters.empty())
            return true;

        return std::find(m_filters.cbegin(), m_filters.cend(), filter) != std::end(m_filters);
    }

} // namespace hdk
