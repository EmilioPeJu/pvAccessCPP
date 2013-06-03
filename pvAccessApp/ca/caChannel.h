/**
 * Copyright - See the COPYRIGHT that is included with this distribution.
 * pvAccessCPP is distributed subject to a Software License Agreement found
 * in file LICENSE that is included with this distribution.
 */

#ifndef CACHANNEL_H
#define CACHANNEL_H

#include <pv/pvAccess.h>

/* for CA */
#include <cadef.h>

namespace epics {
namespace pvAccess {

class CAChannel :
        public Channel,
        public std::tr1::enable_shared_from_this<CAChannel>
{

public:
    POINTER_DEFINITIONS(CAChannel);

    static CAChannel::shared_pointer create(ChannelProvider::shared_pointer const & channelProvider,
                                            epics::pvData::String const & channelName,
                                            short priority,
                                            ChannelRequester::shared_pointer const & channelRequester);

    virtual ~CAChannel();

    void connected();
    void disconnected();

    chid getChannelID();
    chtype getNativeType();
    unsigned getElementCount();

    epics::pvData::PVStructure::shared_pointer getPVStructure();

    /* --------------- epics::pvAccess::Channel --------------- */

    virtual std::tr1::shared_ptr<ChannelProvider> getProvider();
    virtual epics::pvData::String getRemoteAddress();
    virtual ConnectionState getConnectionState();
    virtual epics::pvData::String getChannelName();
    virtual std::tr1::shared_ptr<ChannelRequester> getChannelRequester();
    virtual bool isConnected();

    virtual void getField(GetFieldRequester::shared_pointer const & requester,epics::pvData::String const & subField);

    virtual AccessRights getAccessRights(epics::pvData::PVField::shared_pointer const & pvField);

    virtual ChannelProcess::shared_pointer createChannelProcess(
            ChannelProcessRequester::shared_pointer const & channelProcessRequester,
            epics::pvData::PVStructure::shared_pointer const & pvRequest);

    virtual ChannelGet::shared_pointer createChannelGet(
            ChannelGetRequester::shared_pointer const & channelGetRequester,
            epics::pvData::PVStructure::shared_pointer const & pvRequest);

    virtual ChannelPut::shared_pointer createChannelPut(
            ChannelPutRequester::shared_pointer const & channelPutRequester,
            epics::pvData::PVStructure::shared_pointer const & pvRequest);

    virtual ChannelPutGet::shared_pointer createChannelPutGet(
            ChannelPutGetRequester::shared_pointer const & channelPutGetRequester,
            epics::pvData::PVStructure::shared_pointer const & pvRequest);

    virtual ChannelRPC::shared_pointer createChannelRPC(
            ChannelRPCRequester::shared_pointer const & channelRPCRequester,
            epics::pvData::PVStructure::shared_pointer const & pvRequest);

    virtual epics::pvData::Monitor::shared_pointer createMonitor(
            epics::pvData::MonitorRequester::shared_pointer const & monitorRequester,
            epics::pvData::PVStructure::shared_pointer const & pvRequest);

    virtual ChannelArray::shared_pointer createChannelArray(
            ChannelArrayRequester::shared_pointer const & channelArrayRequester,
            epics::pvData::PVStructure::shared_pointer const & pvRequest);

    virtual void printInfo();

    virtual void printInfo(epics::pvData::StringBuilder out);

    /* --------------- epics::pvData::Requester --------------- */

    virtual epics::pvData::String getRequesterName();

    virtual void message(epics::pvData::String const & message, epics::pvData::MessageType messageType);

    /* --------------- epics::pvData::Destroyable --------------- */

    virtual void destroy();

private:

    CAChannel(ChannelProvider::shared_pointer const & channelProvider,
              ChannelRequester::shared_pointer const & channelRequester);
    void activate(epics::pvData::String const & channelName, short priority);

    // TODO weak_ptr usage?
    ChannelProvider::shared_pointer channelProvider;
    ChannelRequester::shared_pointer channelRequester;

    chid channelID;
    chtype channelType;
    unsigned elementCount;
    epics::pvData::PVStructure::shared_pointer pvStructure;
};



class CAChannelGet :
        public ChannelGet,
        public std::tr1::enable_shared_from_this<CAChannelGet>
{

public:
    POINTER_DEFINITIONS(CAChannelGet);

    static ChannelGet::shared_pointer create(CAChannel::shared_pointer const & channel,
                                             ChannelGetRequester::shared_pointer const & channelGetRequester,
                                             epics::pvData::PVStructure::shared_pointer const & pvRequest);

    virtual ~CAChannelGet();

    void getDone(struct event_handler_args &args);

    /* --------------- epics::pvAccess::ChannelGet --------------- */

    virtual void get(bool lastRequest);

    /* --------------- epics::pvData::Destroyable --------------- */

    virtual void destroy();

    /* --------------- epics::pvData::Lockable --------------- */

    virtual void lock();
    virtual void unlock();

private:

    CAChannelGet(CAChannel::shared_pointer const & _channel,
                 ChannelGetRequester::shared_pointer const & _channelGetRequester,
                 epics::pvData::PVStructure::shared_pointer const & pvRequest);
    void activate();

    // TODO weak_ptr usage?
    CAChannel::shared_pointer channel;
    ChannelGetRequester::shared_pointer channelGetRequester;
    chtype getType;

    epics::pvData::PVStructure::shared_pointer pvStructure;
    epics::pvData::BitSet::shared_pointer bitSet;
};



class CAChannelPut :
        public ChannelPut,
        public std::tr1::enable_shared_from_this<CAChannelPut>
{

public:
    POINTER_DEFINITIONS(CAChannelPut);

    static ChannelPut::shared_pointer create(CAChannel::shared_pointer const & channel,
                                             ChannelPutRequester::shared_pointer const & channelPutRequester,
                                             epics::pvData::PVStructure::shared_pointer const & pvRequest);

    virtual ~CAChannelPut();

    void putDone(struct event_handler_args &args);
    void getDone(struct event_handler_args &args);

    /* --------------- epics::pvAccess::ChannelPut --------------- */

    virtual void put(bool lastRequest);
    virtual void get();

    /* --------------- epics::pvData::Destroyable --------------- */

    virtual void destroy();

    /* --------------- epics::pvData::Lockable --------------- */

    virtual void lock();
    virtual void unlock();

private:

    CAChannelPut(CAChannel::shared_pointer const & _channel,
                 ChannelPutRequester::shared_pointer const & _channelPutRequester,
                 epics::pvData::PVStructure::shared_pointer const & pvRequest);
    void activate();

    // TODO weak_ptr usage?
    CAChannel::shared_pointer channel;
    ChannelPutRequester::shared_pointer channelPutRequester;
    chtype getType;

    epics::pvData::PVStructure::shared_pointer pvStructure;
    epics::pvData::BitSet::shared_pointer bitSet;
};

}}

#endif  /* CACHANNEL_H */
