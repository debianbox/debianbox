#include "channel/impl/channel_impl_local.hpp"

#include "brocker/brocker_manager.hpp"

namespace hdk
{

    channel_impl_local::channel_impl_local(const std::string& channel_name)
    {
        name(channel_name);
    }

    channel_impl_local::~channel_impl_local()
    {
    }

    void channel_impl_local::open()
    {
    }

    void channel_impl_local::close()
    {
    }

    void channel_impl_local::send(uint32_t type_id, const byte_array &data, const std::string &reciever_id)
    {
        HDK_UNUSED(reciever_id)
        brocker_manager::instance()->post(type_id, data, m_channel_name, "local", "local");
    }

    bool channel_impl_local::connected()
    {
        return true;
    }

} // namespace hdk
