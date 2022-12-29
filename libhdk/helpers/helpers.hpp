#ifndef __HDK_HELPERS_HPP__
#define __HDK_HELPERS_HPP__

#define HDK_UNUSED(v) (void)v;

#include <thread>
#include <string>
#include <algorithm>
#include <iostream>
#include <sstream>
#include <cstdio>
#include <iomanip>
#include <cstdint>

namespace hdk
{

    namespace helpers
    {

        inline int this_thread_id()
        {
            auto thread_id = std::this_thread::get_id();
            return *static_cast<unsigned int*>(static_cast<void*>(&thread_id));
        }

        inline std::string to_hex(const std::string& str)
        {
            std::stringstream os;
            std::string retval;

            for (std::string::size_type i = 0; i < str.length(); ++i)
                os << std::hex << std::uppercase << std::setfill('0') << std::setw(2) << static_cast<unsigned int>(static_cast<unsigned char>(str[i]));

            return os.str();
        }

    } // namespace helpers

} // namespace hdk

#endif // __HDK_HELPERS_HPP__
