/*
 * testBeaconEmitter.cpp
 */

#include "remote.h"
#include "blockingUDP.h"
#include "beaconEmitter.h"
#include "inetAddressUtil.h"

#include <osiSock.h>

#include <iostream>
#include <cstdio>

using namespace epics::pvAccess;
using namespace epics::pvData;

class DummyResponseHandler : public ResponseHandler
{
public:
    DummyResponseHandler(Context* ctx) : ResponseHandler() {}

    virtual void handleResponse(osiSockAddr* responseFrom,
            Transport* transport, int8 version, int8 command, int payloadSize,
            ByteBuffer* payloadBuffer)
    {
    	cout << "DummyResponseHandler::handleResponse" << endl;
    }
};


class ContextImpl : public Context {
public:
    ContextImpl() :
        _tr(new TransportRegistry()), _timer(new Timer("server thread",
                lowPriority)), _conf(new SystemConfigurationImpl()) {
    }
    virtual ~ContextImpl() {
        delete _tr;
        delete _timer;
    }
    virtual Timer* getTimer() { return _timer; }
    virtual TransportRegistry* getTransportRegistry() { return _tr; }
    virtual Channel* getChannel(epics::pvAccess::pvAccessID) { return 0; }
    virtual Transport* getSearchTransport() { return 0; }
    virtual Configuration* getConfiguration() { return _conf; }

private:
    TransportRegistry* _tr;
    Timer* _timer;
    Configuration* _conf;
};


void testBeaconEmitter()
{
    ContextImpl ctx;
    DummyResponseHandler drh(&ctx);
/*    SOCKET mysocket;
    if ((mysocket = socket (AF_INET, SOCK_DGRAM, 0)) == -1)
    {
    	assert(false);
    }
    InetAddrVector* broadcastAddresses = getBroadcastAddresses(mysocket);*/


    InetAddrVector* broadcastAddresses = new InetAddrVector;
    osiSockAddr* addr = new osiSockAddr;
    addr->ia.sin_family = AF_INET;
    addr->ia.sin_port = htons(5067);
    if(inet_aton("255.255.255.255",&addr->ia.sin_addr)==0)
    {
    	assert(false);
    }
    broadcastAddresses->push_back(addr);
    BlockingUDPConnector connector(true, broadcastAddresses, true);

    osiSockAddr bindAddr;
    bindAddr.ia.sin_family = AF_INET;
    bindAddr.ia.sin_port = htons(5066);
    bindAddr.ia.sin_addr.s_addr = htonl(INADDR_ANY);
    Transport* transport = connector.connect(NULL, &drh, bindAddr, 1, 50);

    cout<<"Sending beacons"<<endl;
    BeaconEmitter beaconEmitter(transport, transport->getRemoteAddress());
    beaconEmitter.start();

    while(1) sleep(1);

    delete transport;
}

int main(int argc, char *argv[])
{
	testBeaconEmitter();
    return (0);
}