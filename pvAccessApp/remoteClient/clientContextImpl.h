/*
 * clientContext.h
 *
 *  Created on: Dec 21, 2010
 *      Author: msekoran
 */

#ifndef CLIENTCONTEXTIMPL_H_
#define CLIENTCONTEXTIMPL_H_

#include <pvAccess.h>
#include <remote.h>
#include <channelSearchManager.h>

namespace epics {
    namespace pvAccess {
        
        class BeaconHandler;
        class ClientContextImpl;
        
        class ChannelImpl :
                public Channel ,
                public TransportClient,
                public TransportSender,
                public BaseSearchInstance
        {
        public:
            virtual pvAccessID getChannelID() = 0;
            virtual void destroyChannel(bool force) = 0;
            virtual void connectionCompleted(pvAccessID sid/*,  rights*/) = 0;
            virtual void createChannelFailed() = 0;
            virtual ClientContextImpl* getContext() = 0;

            virtual pvAccessID getServerChannelID() = 0;
            virtual void registerResponseRequest(ResponseRequest* responseRequest) = 0;
            virtual void unregisterResponseRequest(ResponseRequest* responseRequest) = 0;
            virtual Transport* checkAndGetTransport() = 0;
            virtual Transport* getTransport() = 0;

            static Status* channelDestroyed;
            static Status* channelDisconnected;

        };
        
        class ClientContextImpl : public ClientContext, public Context
        {
        public:
            virtual ChannelSearchManager* getChannelSearchManager() = 0;
            virtual void checkChannelName(String& name) = 0;

            virtual void registerChannel(ChannelImpl* channel) = 0;
            virtual void unregisterChannel(ChannelImpl* channel) = 0;

            virtual void destroyChannel(ChannelImpl* channel, bool force) = 0;
            virtual ChannelImpl* createChannelInternal(String name, ChannelRequester* requester, short priority, InetAddrVector* addresses) = 0;

            virtual ResponseRequest* getResponseRequest(pvAccessID ioid) = 0;
            virtual pvAccessID registerResponseRequest(ResponseRequest* request) = 0;
            virtual ResponseRequest* unregisterResponseRequest(ResponseRequest* request) = 0;


            virtual Transport* getTransport(TransportClient* client, osiSockAddr* serverAddress, int16 minorRevision, int16 priority) = 0;

            virtual void beaconAnomalyNotify() = 0;

            virtual BeaconHandler* getBeaconHandler(osiSockAddr* responseFrom) = 0;

        };
        
        extern ClientContextImpl* createClientContextImpl();

    }
}

#endif /* CLIENTCONTEXTIMPL_H_ */
