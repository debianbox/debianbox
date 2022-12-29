#include "kill_handler.hpp"

#include <signal.h>

#include <cstdlib>

#include "helpers/helpers.hpp"

static void __kill_signal_handler(int nSig, siginfo_t* sigInfo, void* ptr_context);
static void __atexit_handler();

namespace hdk
{
    namespace kill_handler
    {
        namespace _PRIVATE
        {
            std::function<void()> g_kill_handler_func;
        }

        void reg(std::function<void()> f)
        {
            _PRIVATE::g_kill_handler_func = f;

            struct sigaction sa;
            {
                sa.sa_flags = SA_SIGINFO;
                sa.sa_sigaction = __kill_signal_handler;
                sigemptyset( &sa.sa_mask );
            }

            // User
            sigaction( SIGINT,  &sa, NULL );
            sigaction( SIGQUIT, &sa, NULL );
            sigaction( SIGTSTP, &sa, NULL );
            // Service
            sigaction( SIGKILL, &sa, NULL );
            sigaction( SIGTERM, &sa, NULL );

            std::atexit(__atexit_handler);
        }
    }
} // namespace hdk

static void __atexit_handler()
{
    hdk::kill_handler::_PRIVATE::g_kill_handler_func();
}

static void __kill_signal_handler(int nSig, siginfo_t* sigInfo, void* ptr_context)
{
    HDK_UNUSED(nSig)
    HDK_UNUSED(sigInfo)
    HDK_UNUSED(ptr_context)

    exit(0);
}
