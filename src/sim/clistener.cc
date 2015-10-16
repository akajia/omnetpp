//=========================================================================
//  CLISTENER.CC - part of
//
//                  OMNeT++/OMNEST
//           Discrete System Simulation in C++
//
//=========================================================================

/*--------------------------------------------------------------*
  Copyright (C) 1992-2015 Andras Varga
  Copyright (C) 2006-2015 OpenSim Ltd.

  This file is distributed WITHOUT ANY WARRANTY. See the file
  `license' for details on this and other legal matters.
*--------------------------------------------------------------*/

#include "omnetpp/clistener.h"
#include "omnetpp/ccomponent.h"
#include "omnetpp/cmodule.h"
#include "omnetpp/cchannel.h"
#include "omnetpp/csimulation.h"

namespace omnetpp {

struct Subscription
{
    cComponent *component;
    simsignal_t signalID;
};
typedef std::vector<Subscription> SubscriptionList;

static void findListenerOccurences(cComponent *component, cIListener *listener, SubscriptionList& result)
{
    std::vector<simsignal_t> signals = component->getLocalListenedSignals();
    for (unsigned int i = 0; i < signals.size(); i++) {
        simsignal_t signalID = signals[i];
        if (component->isSubscribed(signalID, listener)) {
            result.push_back(Subscription());
            result.back().component = component;
            result.back().signalID = signalID;
        }
    }

    if (component->isModule()) {
        cModule *module = (cModule *)component;
        for (cModule::SubmoduleIterator it(module); !it.end(); ++it)
            findListenerOccurences(*it, listener, result);
        for (cModule::ChannelIterator it(module); !it.end(); ++it)
            findListenerOccurences(*it, listener, result);
    }
}

//---

cIListener::cIListener()
{
    subscribeCount = 0;
}

cIListener::~cIListener()
{
    if (subscribeCount) {
        // note: throwing an exception would is risky here: it would typically
        // cause other exceptions, and eventually crash
        if (subscribeCount < 0) {
            getEnvir()->printfmsg(
                    "cListener destructor: internal error: negative subscription "
                    "count (%d) in listener at address %p", subscribeCount, this);
            return;
        }

        // subscribecount > 0:
        getEnvir()->printfmsg(
                "cListener destructor: listener at address %p is still added to "
                "%d listener list(s). This will likely result in a crash: "
                "Listeners must be fully unsubscribed before deletion. "
                "Trying to determine components where this listener is subscribed...",
                this, subscribeCount);

        // print components and signals where this listener is subscribed
        std::stringstream out;
        SubscriptionList list;
        findListenerOccurences(getSimulation()->getSystemModule(), this, list);
        for (int i = 0; i < (int)list.size(); i++) {
            out << "- signal \"" << cComponent::getSignalName(list[i].signalID) << "\" (id=" << list[i].signalID << ") ";
            out << "at (" << list[i].component->getClassName() << ")" << list[i].component->getFullPath() << "\n";
        }
        getEnvir()->printfmsg("Subscriptions for listener at address %p:\n%s", this, out.str().c_str());
    }
}

//---

void cListener::unsupportedType(simsignal_t signalID, const char *dataType)
{
    const char *signalName = cComponent::getSignalName(signalID);
    throw cRuntimeError("%s: Unsupported signal data type %s for signal %s (id=%d)",
            opp_typename(typeid(*this)), dataType, signalName, (int)signalID);
}

void cListener::receiveSignal(cComponent *, simsignal_t signalID, bool)
{
    unsupportedType(signalID, "bool");
}

void cListener::receiveSignal(cComponent *, simsignal_t signalID, long)
{
    unsupportedType(signalID, "long");
}

void cListener::receiveSignal(cComponent *, simsignal_t signalID, unsigned long)
{
    unsupportedType(signalID, "unsigned long");
}

void cListener::receiveSignal(cComponent *, simsignal_t signalID, double)
{
    unsupportedType(signalID, "double");
}

void cListener::receiveSignal(cComponent *, simsignal_t signalID, const SimTime&)
{
    unsupportedType(signalID, "simtime_t");
}

void cListener::receiveSignal(cComponent *, simsignal_t signalID, const char *)
{
    unsupportedType(signalID, "const char *");
}

void cListener::receiveSignal(cComponent *, simsignal_t signalID, cObject *)
{
    unsupportedType(signalID, "cObject *");
}

}  // namespace omnetpp

