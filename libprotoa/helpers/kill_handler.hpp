#ifndef __PROTOA_KILLHANDLER_H__
#define __PROTOA_KILLHANDLER_H__

#include <functional>

namespace protoa
{
    namespace kill_handler
    {
        void reg(std::function<void()> f);
    }
}

#endif //__PROTOA_KILLHANDLER_H__
