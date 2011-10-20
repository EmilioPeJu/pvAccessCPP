/**
 * Copyright - See the COPYRIGHT that is included with this distribution.
 * pvAccessCPP is distributed subject to a Software License Agreement found
 * in file LICENSE that is included with this distribution.
 */

#ifndef BEACONSERVERSTATUSPROVIDER_H
#define BEACONSERVERSTATUSPROVIDER_H

#include <pv/pvData.h>
//#include <pv/serverContext.h>
#include <pv/sharedPtr.h>

namespace epics {
namespace pvAccess {

    class ServerContext;

	/**
	 * BeaconServerStatusProvider
	 */
	class BeaconServerStatusProvider
	{
	public:
        typedef std::tr1::shared_ptr<BeaconServerStatusProvider> shared_pointer;
        typedef std::tr1::shared_ptr<const BeaconServerStatusProvider> const_shared_pointer;

		/**
		 * Gets server status data.
		 */
		virtual epics::pvData::PVField::shared_pointer getServerStatusData() = 0;
	};

	/**
	 * DefaultBeaconServerStatusProvider
	 */
	class DefaultBeaconServerStatusProvider : public BeaconServerStatusProvider
	{
	public:
		/**
		 * Constructor.
		 * @param context CA context.
		 */
//		DefaultBeaconServerStatusProvider(ServerContext::shared_pointer const & context);
		DefaultBeaconServerStatusProvider(std::tr1::shared_ptr<ServerContext> const & context);
		/**
		 * Destructor.
		 */
		virtual ~DefaultBeaconServerStatusProvider();

		virtual epics::pvData::PVField::shared_pointer getServerStatusData();

	private:
		/**
		 * Initialize
		 */
		void initialize();


	private:
		epics::pvData::PVStructure::shared_pointer _status;
		std::tr1::shared_ptr<ServerContext> _context;
        //ServerContext::shared_pointer _context;
	};

}}

#endif  /* BEACONSERVERSTATUSPROVIDER_H */
