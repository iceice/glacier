#include "base/logging.h"


int main(int argc, char const *argv[])
{
    LOG_TRACE << "trace";
    LOG_DEBUG << "debug";
    LOG_INFO << "Hello";
    LOG_WARN << "World";
    LOG_ERROR << "Error";
    LOG_INFO << sizeof(Logger);
    LOG_INFO << sizeof(LogStream);
    LOG_INFO << sizeof(Fmt);
    LOG_INFO << sizeof(LogStream::Buffer);
    return 0;
}
