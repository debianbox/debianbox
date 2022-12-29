#ifndef __HDK_BROCKER_HPP__
#define __HDK_BROCKER_HPP__

#include <unordered_map>

#include "asio/io_context.hpp"

#include "helpers/helpers.hpp"
#include "brocker/brocker_call.hpp"

namespace hdk
{
    class brocker_manager;

    class brocker
    {
    public:
        friend class brocker_manager;

        struct sender_info
        {
            sender_info() {}
            sender_info(const std::string id_, const std::string name_, const std::string address_) : id(id_), name(name_), address(address_) {}
            ~sender_info() = default;

            sender_info(const sender_info&) = default;
            sender_info(sender_info&&) = default;
            sender_info& operator=(const sender_info&) = default;
            sender_info& operator=(sender_info&&) = default;


            std::string id;
            std::string name;
            std::string address;
        };

        brocker(asio::io_context &ctx);
        ~brocker();

        brocker(const brocker& brocker_);

        brocker(brocker&&) = delete;
        brocker& operator=(brocker&&) = delete;

        brocker& filter(std::initializer_list<std::string> filters);
        brocker& filter(const std::vector<std::string>& filters);
        brocker& filter(const std::string& filter);

        asio::io_context& context();

        static void post(uint32_t type_id, const hdk::byte_array& data, const std::string &filter = "",
                         const std::string& sender_name = "", const std::string& sender_address = "");

        template<typename OBJ, typename ARG>
        brocker& reg(OBJ* object_ptr, void(OBJ::*memfunc_ptr)(ARG))
        {
            typename std::decay<ARG>::type msg;
            uint32_t type_id = msg.get_type_id();

            m_calls.insert({type_id,
                            new brocker_call<ARG>([memfunc_ptr, object_ptr](ARG arg)
                                                {
                                                    (object_ptr->*memfunc_ptr)(std::forward<ARG>(arg));
                                                })
                           });

            __reg(type_id);
            return *this;
        }

        template<typename ARG>
        brocker& reg(void(*func_ptr)(ARG))
        {
            typename std::decay<ARG>::type msg;
            uint32_t type_id = msg.get_type_id();

            m_calls.insert({type_id,
                            new brocker_call<ARG>([func_ptr](ARG arg)
                                                {
                                                    (*func_ptr)(arg);
                                                })
                           });

            __reg(type_id);
            return *this;
        }

        template<typename ARG>
        brocker& reg(std::function<void(ARG)> f)
        {
            typename std::decay<ARG>::type msg;
            uint32_t type_id = msg.get_type_id();

            m_calls.insert({type_id,
                new brocker_call<ARG>(f)
            });

            __reg(type_id);
            return *this;
        }

        template<typename ARG>
        void unreg(void(*func_ptr)(ARG))
        {
            HDK_UNUSED(func_ptr)
            typename std::decay<ARG>::type msg;
            uint32_t type_id = msg.get_type_id();
            m_calls.erase(type_id);
            __unreg(type_id);
        }

        template<typename OBJ,typename ARG>
        void unreg(void(OBJ::*memfunc_ptr)(ARG))
        {
            HDK_UNUSED(memfunc_ptr)
            typename std::decay<ARG>::type msg;
            uint32_t type_id = msg.get_type_id();
            m_calls.erase(type_id);
            __unreg(type_id);
        }

    private:
        std::unordered_map<uint32_t, _PRIVATE::basic_brocker_call*> m_calls;
        std::vector<std::string> m_filters;

        asio::io_context& m_io_context;

        void __call(uint32_t type_id, const byte_array &data) const;
        void __reg(uint32_t type_id);
        void __unreg(uint32_t type_id);
        bool __check_filter(const std::string& filter) const;
    };

} // namespace hdk

#endif // __HDK_BROCKER_HPP__
