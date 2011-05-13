/*
 * transportRegistry.h
 */

#ifndef TRANSPORTREGISTRY_H
#define TRANSPORTREGISTRY_H

#include <map>
#include <vector>
#include <iostream>

#include <osiSock.h>

#include <lock.h>
#include <pvType.h>
#include <epicsException.h>
#include <remote.h>
#include "inetAddressUtil.h"
#include <sharedPtr.h>

namespace epics { namespace pvAccess {

class TransportRegistry {
public:
    typedef std::tr1::shared_ptr<TransportRegistry> shared_pointer;
    typedef std::tr1::shared_ptr<const TransportRegistry> const_shared_pointer;

    typedef std::vector<Transport::shared_pointer> transportVector_t;

    TransportRegistry();
    virtual ~TransportRegistry();

    void put(Transport::shared_pointer const & transport);
    Transport::shared_pointer get(epics::pvData::String type, const osiSockAddr* address, const epics::pvData::int16 priority);
    std::auto_ptr<transportVector_t> get(epics::pvData::String type, const osiSockAddr* address);
    Transport::shared_pointer remove(Transport::shared_pointer const & transport);
    void clear();
    epics::pvData::int32 numberOfActiveTransports();
    std::auto_ptr<transportVector_t> toArray(epics::pvData::String type);
    std::auto_ptr<transportVector_t> toArray();

private:
    //TODO if unordered map is used instead of map we can use sockAddrAreIdentical routine from osiSock.h
    // NOTE: pointers are used to osiSockAddr (to save memory), since it guaranteed that their reference is valid as long as Transport
    typedef std::map<const epics::pvData::int16,Transport::shared_pointer> prioritiesMap_t;
    typedef std::tr1::shared_ptr<prioritiesMap_t> prioritiesMapSharedPtr_t;
    typedef std::map<const osiSockAddr*,prioritiesMapSharedPtr_t,comp_osiSockAddrPtr> transportsMap_t;

    transportsMap_t _transports;
    int32 _transportCount;
    epics::pvData::Mutex _mutex;
};

}}

#endif  /* INTROSPECTIONREGISTRY_H */
