#ifndef __HDK_KILLHANDLER_H__
#define __HDK_KILLHANDLER_H__

#include <functional>

namespace hdk
{

    namespace kill_handler
    {
        void reg(std::function<void()> f);
    }

} // namespace hdk

#endif //__HDK_KILLHANDLER_HPP__
