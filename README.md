# About
This is repository for my lightweight C++ asynchronous library for embedded Linux devices or Linux services on x86/ARM boxes, based on **libasio** and **zeromq**.
I hope someone will help to improve this lib.)
# Credits
This library is using non-boost version of **asio** from think-async.com, and **cppzmq**.

# Purpose
Purpose of creating is simple, it's good to have an extremely lightwight library (ideally header only) for different projects related to embedded Linux devices for non critical automatization etc.

# Other
This library is intended to be **C++14 compliant** (most of projects I work on are supposed to use GCC 6 or Clang-9 compilers)

# Components
1. *async timers* - with interface similiar to Qt's **QTimer**
2. *async objects* - for calling member functions in designated thread asynchronously or synchronously
3. *async database queries* - for dealing with databases (for now SQlite) asynchronously or synchronously
4. *async networking* - easy to implement network communication with **macro-based** serialization and callbacks. Can use **zeromq** as transport.
5. *async serial port* - just wrapper over **libasio** with callbacks
6. *async file descriptor polling* - for device drivers
7. *additional helper functions and classes* - kill handlers, checksums, algorithms, stacktrace, *etc*. 
