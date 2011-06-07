/*
 * serverChannelImpl.cpp
 */

#include <pv/serverChannelImpl.h>

using namespace epics::pvData;

namespace epics { namespace pvAccess {

ServerChannelImpl::ServerChannelImpl(Channel::shared_pointer const & channel, pvAccessID cid, pvAccessID sid, epics::pvData::PVField::shared_pointer const & securityToken):
			_channel(channel),
			_cid(cid),
			_sid(sid),
			_destroyed(false)
{
	if (!channel.get())
	{
		THROW_BASE_EXCEPTION("non-null channel required");
	}
}

Channel::shared_pointer ServerChannelImpl::getChannel()
{
	return _channel;
}

pvAccessID ServerChannelImpl::getCID() const
{
	return _cid;
}

pvAccessID ServerChannelImpl::getSID() const
{
	return _sid;
}

int16 ServerChannelImpl::getAccessRights()
{
	//TODO implement
	return 0;
}

void ServerChannelImpl::registerRequest(const pvAccessID id, Destroyable::shared_pointer const & request)
{
	Lock guard(_mutex);
	_requests[id] = request;
}

void ServerChannelImpl::unregisterRequest(const pvAccessID id)
{
	Lock guard(_mutex);
	_iter = _requests.find(id);
	if(_iter != _requests.end())
	{
		_requests.erase(_iter);
	}
}

Destroyable::shared_pointer ServerChannelImpl::getRequest(const pvAccessID id)
{
	_iter = _requests.find(id);
	if(_iter != _requests.end())
	{
		return _iter->second;
	}
	return Destroyable::shared_pointer();
}

void ServerChannelImpl::destroy()
{
	Lock guard(_mutex);
	if (_destroyed) return;

	_destroyed = true;

	// destroy all requests
	destroyAllRequests();

    // ... and the channel
    // TODO try catch
	_channel->destroy();
}

ServerChannelImpl::~ServerChannelImpl()
{
    destroy();
}

void ServerChannelImpl::printInfo()
{
	printInfo(stdout);
}

void ServerChannelImpl::printInfo(FILE *fd)
{
	fprintf(fd,"CLASS        : %s\n", typeid(*this).name());
	fprintf(fd,"CHANNEL      : %s\n", typeid(*_channel).name());
}

void ServerChannelImpl::destroyAllRequests()
{
	Lock guard(_mutex);

	// resource allocation optimization
	if (_requests.size() == 0)
		return;

	while(_requests.size() != 0)
	{
		_iter = _requests.begin();
		_iter->second->destroy();
	}

	_requests.clear();
}

}
}
