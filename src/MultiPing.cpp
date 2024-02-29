#include <EBApplication.hpp>
#include <socket/raw/EBICMP.hpp>
#include <EBUtils.hpp>

#include <json/EBJson.hpp>
#include <xml/EBXml.hpp>

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

    if( isXmlResult() )
    {
        xmlResult.setVersion("1.0");
        xmlResult.setRootElement(EBCreate<EBXmlElement>(EBXmlElement::TYPE::ELEMENT, "MultiPing", ""));
    }

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

bool MultiPing::isJsonResult()
{
    return EBApplication::containsArgument("--json") || EBApplication::containsArgument("-j");
}

bool MultiPing::isXmlResult()
{
    return EBApplication::containsArgument("--xml") || EBApplication::containsArgument("-x");
}

void MultiPing::checkForFinished()
{
    counter--;
    if( !counter && currentIp > lastIp )
    {
        if( isJsonResult() )
        {
            std::cout << jsonResult.dump() << std::endl;
        }        
        if( isXmlResult() )
        {
            std::cout << xmlResult.dump() << std::endl;
        }        
        EBEventLoop::getInstance()->exit();
    }
}

EB_SLOT_WITH_ARGS(MultiPing::finished, EBICMP::ICMP_RESULT result)
{
    if( isJsonResult() )
    {
        EBJsonObject obj;
        obj.set("host", result.host);
        obj.set("result", result.resultString);
        obj.set("rtt", result.roundtrip);
        obj.set("status", result.status);
        jsonResult.append(obj);
    }
    else if( isXmlResult() )
    {
        EBPtr<EBXmlElement> element = EBCreate<EBXmlElement>(EBXmlElement::TYPE::ELEMENT, "icmp", "");
        element->addAttribute(EBCreate<EBXmlElement>(EBXmlElement::TYPE::ATTRIBUTE, "host", result.host));
        element->addAttribute(EBCreate<EBXmlElement>(EBXmlElement::TYPE::ATTRIBUTE, "result", result.resultString.trim()));;
        element->addAttribute(EBCreate<EBXmlElement>(EBXmlElement::TYPE::ATTRIBUTE, "rtt", EBUtils::intToStr(result.roundtrip)));;
        element->addAttribute(EBCreate<EBXmlElement>(EBXmlElement::TYPE::ATTRIBUTE, "status", EBUtils::intToStr(result.status)));
        xmlResult.getRootElement()->addChild(element);
    }
    else
    {
        if( result.status == 0 )
        {
            std::cout << result.host << " " << result.roundtrip << " ms" << std::endl;
        }
    }

    checkForFinished();
}

EB_SLOT(MultiPing::error)
{
    EBPtr<EBICMP> icmp = sender.cast<EBICMP>();

    if( EBApplication::containsArgument("--json") || EBApplication::containsArgument("-j") )
    {
        EBJsonObject obj;
        obj.set("host", icmp->getDestination().getHost());
        obj.set("result", icmp->getLastError());
        obj.set("rtt", -1);
        obj.set("status", -1);
        jsonResult.append(obj);
    }
    else if( isXmlResult() )
    {
        EBPtr<EBXmlElement> element = EBCreate<EBXmlElement>(EBXmlElement::TYPE::ELEMENT, "icmp", "");
        element->addAttribute(EBCreate<EBXmlElement>(EBXmlElement::TYPE::ATTRIBUTE, "host",icmp->getDestination().getHost()));
        element->addAttribute(EBCreate<EBXmlElement>(EBXmlElement::TYPE::ATTRIBUTE, "result", icmp->getLastError()));
        element->addAttribute(EBCreate<EBXmlElement>(EBXmlElement::TYPE::ATTRIBUTE, "rtt", "-1"));
        element->addAttribute(EBCreate<EBXmlElement>(EBXmlElement::TYPE::ATTRIBUTE, "status", "-1"));
        xmlResult.getRootElement()->addChild(element);
    }

    checkForFinished();
}
