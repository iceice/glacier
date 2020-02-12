#include "src/inet_dddress.h"

#include "base/logging.h"

int main(int argc, char const *argv[])
{
    InetAddress addr0(1234);
    LOG_INFO << addr0.toIp();
    LOG_INFO << addr0.toIpPort();
    LOG_INFO << addr0.toPort();

    InetAddress addr1(4321, true);
    LOG_INFO << addr1.toIp();
    LOG_INFO << addr1.toIpPort();
    LOG_INFO << addr1.toPort();

    InetAddress addr2("1.2.3.4", 8888);
    LOG_INFO << addr2.toIp();
    LOG_INFO << addr2.toIpPort();
    LOG_INFO << addr2.toPort();

    InetAddress addr3("255.254.253.252", 65535);
    LOG_INFO << addr3.toIp();
    LOG_INFO << addr3.toIpPort();
    LOG_INFO << addr3.toPort();

    InetAddress addr(80);
    if (InetAddress::resolve("google.com", &addr))
    {
        LOG_INFO << "google.com resolved to " << addr.toIpPort();
    }
    else
    {
        LOG_ERROR << "Unable to resolve google.com";
    }

    return 0;
}
