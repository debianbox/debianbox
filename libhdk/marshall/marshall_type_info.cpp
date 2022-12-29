#include "marshall/marshall_type_info.hpp"

namespace hdk
{

    const char* function_name()
    {
#if defined(__GNUG__)
        return __PRETTY_FUNCTION__;
#elif defined(_WIN32) && defined(_MSC_VER)
        return __FUNCSIG__;
#endif
    }

} // namespace hdk

