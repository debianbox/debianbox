#include "channel/impl/channel_impl.hpp"

namespace hdk
{
    std::string channel_impl::name() const
    {
        return m_channel_name;
    }

    void channel_impl::name(const std::string &channel_name)
    {
        m_channel_name = channel_name;
    }

} // namespace hdk
