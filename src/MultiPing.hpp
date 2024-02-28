#include <EBString.hpp>
#include <EBTimer.hpp>
#include <socket/raw/EBICMP.hpp>

using namespace EBCpp;

class MultiPing : public EBObject<MultiPing>
{
public:
    MultiPing();

private:
    void ping(const EBString& hostname);
    EB_SLOT_WITH_ARGS(finished, EBICMP::ICMP_RESULT result);
    EB_SLOT(error);
    uint64_t counter = 0;
    uint64_t currentIp;
    uint64_t firstIp;
    uint64_t lastIp;

    EBTimer timer;

    EBList<EBPtr<EBICMP>> pings;

    EB_SLOT(timeout);
};
