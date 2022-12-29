#ifndef __BROCKER_CALL_HPP__
#define __BR0CKER_CALL_HPP__

#include "asio/streambuf.hpp"

#include <functional>
#include <type_traits>

#include "marshall/marshal.hpp"

namespace hdk
{

    namespace _PRIVATE
    {
        class basic_brocker_call
        {
        public:
            basic_brocker_call() = default;
            virtual ~basic_brocker_call() = default;
            virtual void call(const hdk::byte_array& data) const = 0;
        };
    }

    template<typename T>
    class brocker_call : public _PRIVATE::basic_brocker_call
    {
    public:
        brocker_call (std::function<void(T)> callback)
            : m_callback(callback) {}

        virtual ~brocker_call() = default;

        virtual void call(const hdk::byte_array& data) const
        {
            asio::streambuf input_buffer;
            hdk::from_byte_array(input_buffer, data);
            std::iostream input_stream(&input_buffer);

            typename std::decay<T>::type struct_data;
            input_stream >> struct_data;

            m_callback(struct_data);
        }

    private:
        std::function<void(T)> m_callback;

    };

} // namespace hdk

#endif // __BROCKER_CALL_HPP__
