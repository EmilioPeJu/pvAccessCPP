/**
 * Copyright - See the COPYRIGHT that is included with this distribution.
 * pvAccessCPP is distributed subject to a Software License Agreement found
 * in file LICENSE that is included with this distribution.
 */
 
#ifndef SIMPLECHANNELSEARCHMANAGERIMPL_H
#define SIMPLECHANNELSEARCHMANAGERIMPL_H

#include <pv/channelSearchManager.h>

#include <pv/lock.h>
#include <pv/byteBuffer.h>
#include <pv/timer.h>

namespace epics {
namespace pvAccess {


class MockTransportSendControl: public TransportSendControl
{
public:
	void endMessage() {}
	void flush(bool lastMessageCompleted) {}
	void setRecipient(const osiSockAddr& sendTo) {}
	void startMessage(epics::pvData::int8 command, int ensureCapacity) {}
	void ensureBuffer(int size) {}
	void alignBuffer(int alignment) {}
	void flushSerializeBuffer() {}
    void cachedSerialize(
    const std::tr1::shared_ptr<const epics::pvData::Field>& field, epics::pvData::ByteBuffer* buffer)
    {
        // no cache
        field->serialize(buffer, this);
    }
};


class SimpleChannelSearchManagerImpl : public ChannelSearchManager, public epics::pvData::TimerCallback
{
    public:
    POINTER_DEFINITIONS(SimpleChannelSearchManagerImpl);
    
    /**
	 * Constructor.
	 * @param context
	 */
	SimpleChannelSearchManagerImpl(Context::shared_pointer const & context);
    /**
	 * Constructor.
	 * @param context
	 */
	virtual ~SimpleChannelSearchManagerImpl();
	/**
	 * Cancel.
	 */
	void cancel();
	/**
	 * Get number of registered channels.
	 * @return number of registered channels.
	 */
	int32_t registeredCount();
	/**
	 * Register channel.
	 * @param channel to register.
	 */
	void registerSearchInstance(SearchInstance::shared_pointer const & channel);
	/**
	 * Unregister channel.
	 * @param channel to unregister.
	 */
	void unregisterSearchInstance(SearchInstance::shared_pointer const & channel);
	/**
	 * Search response from server (channel found).
	 * @param cid	client channel ID.
	 * @param seqNo	search sequence number.
	 * @param minorRevision	server minor CA revision.
	 * @param serverAddress	server address.
	 */
	void searchResponse(pvAccessID cid, int32_t seqNo, int8_t minorRevision, osiSockAddr* serverAddress);
	/**
	 * Beacon anomaly detected.
	 * Boost searching of all channels.
	 */
	void beaconAnomalyNotify();
    
    /// Timer callback.	
    void callback();
    
    /// Timer stooped callback.
    void timerStopped();

    private:
    
    bool generateSearchRequestMessage(SearchInstance::shared_pointer const & channel, bool allowNewFrame, bool flush);
    
    static bool generateSearchRequestMessage(SearchInstance::shared_pointer const & channel,
        epics::pvData::ByteBuffer* byteBuffer, TransportSendControl* control);

    void boost();
    
    void initializeSendBuffer();
    void flushSendBuffer();
    
    static bool isPowerOfTwo(int32_t x);

	/**
	 * Context.
	 */
	Context::weak_pointer m_context;

	/**
	 * Canceled flag.
	 */
	AtomicBoolean m_canceled;

	/**
	 * Search (datagram) sequence number.
	 */
	int32_t m_sequenceNumber;
	
   	/**
	 * Send byte buffer (frame)
	 */
	epics::pvData::ByteBuffer m_sendBuffer;
	
    /**
     * Set of registered channels.
     */
    std::map<pvAccessID,SearchInstance::shared_pointer> m_channels;
    
	/**
	 * Timer node.
	 * (sync on requestPendingChannels)
	 */
	epics::pvData::TimerNode m_timerNode;

    /**
     * Time of last frame send.
     */
    int64_t m_lastTimeSent;
    
    /**
     * Mock transport send control
     */
    MockTransportSendControl m_mockTransportSendControl;
    
   	/**
	 * This instance mutex.
	 */
	epics::pvData::Mutex m_channelMutex;
    
   	/**
	 * User value lock.
	 */
	epics::pvData::Mutex m_userValueMutex;

   	/**
	 * m_channels mutex.
	 */
	epics::pvData::Mutex m_mutex;

    static const int DATA_COUNT_POSITION;
    static const int PAYLOAD_POSITION;

    static const double ATOMIC_PERIOD;
    static const int PERIOD_JITTER_MS;
    
	static const int BOOST_VALUE;
	static const int MAX_COUNT_VALUE;
	static const int MAX_FALLBACK_COUNT_VALUE;

	static const int MAX_FRAMES_AT_ONCE;
	static const int DELAY_BETWEEN_FRAMES_MS;

};

}
}

#endif  /* SIMPLECHANNELSEARCHMANAGERIMPL_H */
