#include <EBApplication.hpp>
#include <socket/raw/EBICMP.hpp>
#include <EBUtils.hpp>

#include "MultiPing.hpp"

MultiPing::MultiPing()
{
    if( EBApplication::getArgumentCount() < 3 )
    {
        std::cout << "MultiPing.exe [first ip] [last ip]" << std::endl;
        EBEventLoop::getInstance()->exit();
        return;
    }

    EBString firstAddress = EBApplication::getArgument(1);
    EBString lastAddress = EBApplication::getArgument(2);

    firstIp = EBUtils::ipToInt(firstAddress);
    lastIp = EBUtils::ipToInt(lastAddress);
    currentIp = firstIp;

    timer.timeout.connect(this, &MultiPing::timeout);
    timer.start(50);
}

EB_SLOT(MultiPing::timeout)
{
    ping(EBUtils::intToIp(currentIp));
    currentIp++;
    if( currentIp > lastIp )
    {
        timer.stop();
    }
}

void MultiPing::ping(const EBString& hostname)
{
    EBPtr<EBICMP> ping = EBCreate<EBICMP>();
    ping->setDestination(hostname);
    ping->finished.connect(this, &MultiPing::finished);
    ping->error.connect(this, &MultiPing::error);
    ping->ping();

    pings.append(ping);

    counter++;
}

EB_SLOT_WITH_ARGS(MultiPing::finished, EBICMP::ICMP_RESULT result)
{
    if( result.status == 0 )
        std::cout << result.host << " " << result.roundtrip << " ms" << std::endl;
    counter--;

    EBPtr<EBICMP> icmp = sender.cast<EBICMP>();

    if( !counter && currentIp > lastIp )
        EBEventLoop::getInstance()->exit();
}

EB_SLOT(MultiPing::error)
{
    EBPtr<EBICMP> icmp = sender.cast<EBICMP>();
    //std::cout << pings.getSize() << " ERROR  " << icmp->getDestination() << "   " << icmp->getLastError() << std::endl;
    counter--;

    if( !counter && currentIp > lastIp )
        EBEventLoop::getInstance()->exit();

}
