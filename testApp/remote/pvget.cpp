#include <iostream>
#include <pv/clientFactory.h>
#include <pv/pvAccess.h>

#include <stdio.h>
#include <epicsStdlib.h>
#include <epicsGetopt.h>
#include <epicsThread.h>
#include <pv/logger.h>
#include <pv/lock.h>

#include <vector>
#include <string>
#include <sstream>

#include <pv/event.h>
#include <epicsExit.h>

#include "pvutils.cpp"

using namespace std;
using namespace std::tr1;
using namespace epics::pvData;
using namespace epics::pvAccess;






#define DEFAULT_TIMEOUT 3.0
#define DEFAULT_REQUEST "field(value)"

double timeOut = DEFAULT_TIMEOUT;
string request(DEFAULT_REQUEST);

enum PrintMode { ValueOnlyMode, StructureMode, TerseMode };
PrintMode mode = ValueOnlyMode;

char fieldSeparator = ' ';

void usage (void)
{
    fprintf (stderr, "\nUsage: pvget [options] <PV name>...\n\n"
    "  -h: Help: Print this message\n"
    "options:\n"
    "  -r <pv request>:   Request, specifies what fields to return and options, default is '%s'\n"
    "  -w <sec>:          Wait time, specifies timeout, default is %f second(s)\n"
    "  -t:                Terse mode - print only value, without names\n"
    "  -m:                Monitor mode\n"
    "  -d:                Enable debug output\n"
    "  -F <ofs>:          Use <ofs> as an alternate output field separator\n"
    "  -c:                Wait for clean shutdown and report used instance count (for expert users)"
    "\nExample: pvget double01\n\n"
             , DEFAULT_REQUEST, DEFAULT_TIMEOUT);
}


class ChannelGetRequesterImpl : public ChannelGetRequester
{
    private:
    ChannelGet::shared_pointer m_channelGet;
    PVStructure::shared_pointer m_pvStructure;
    BitSet::shared_pointer m_bitSet;
    Mutex m_pointerMutex;
    Event m_event;
    String m_channelName;

    public:
    
    ChannelGetRequesterImpl(String channelName) : m_channelName(channelName) {}
    
    virtual String getRequesterName()
    {
        return "ChannelGetRequesterImpl";
    }

    virtual void message(String const & message, MessageType messageType)
    {
        std::cerr << "[" << getRequesterName() << "] message(" << message << ", " << getMessageTypeName(messageType) << ")" << std::endl;
    }

    virtual void channelGetConnect(const epics::pvData::Status& status,ChannelGet::shared_pointer const & channelGet,
                                   epics::pvData::PVStructure::shared_pointer const & pvStructure, 
                                   epics::pvData::BitSet::shared_pointer const & bitSet)
    {
        if (status.isSuccess())
        {
            // show warning
            if (!status.isOK())
            {
                std::cerr << "[" << m_channelName << "] channel get create: " << status.toString() << std::endl;
            }
            
            // assign smart pointers
            {
                Lock lock(m_pointerMutex);
                m_channelGet = channelGet;
                m_pvStructure = pvStructure;
                m_bitSet = bitSet;
            }
            
            channelGet->get(true);
        }
        else
        {
            std::cerr << "[" << m_channelName << "] failed to create channel get: " << status.toString() << std::endl;
        }
    }

    virtual void getDone(const epics::pvData::Status& status)
    {
        if (status.isSuccess())
        {
            // show warning
            if (!status.isOK())
            {
                std::cerr << "[" << m_channelName << "] channel get: " << status.toString() << std::endl;
            }

            // access smart pointers
            {
                Lock lock(m_pointerMutex);
                {
                    // needed since we access the data
                    ScopedLock dataLock(m_channelGet);
    
                    if (mode == ValueOnlyMode)
                    {
                        PVField::shared_pointer value = m_pvStructure->getSubField("value");
                        if (value.get() == 0)
                        {
                        	std::cerr << "no 'value' field" << std::endl;
                            return;
                        }

                        if (fieldSeparator == ' ' && value->getField()->getType() == scalar)
                        	std::cout << std::setw(30) << std::left << m_channelName;
                        else
                        	std::cout << m_channelName;

                        std::cout << fieldSeparator;

                        terse(std::cout, value) << std::endl;
                    }
                    else if (mode == TerseMode)
                        terseStructure(std::cout, m_pvStructure) << std::endl;
                    else
                        std::cout << *(m_pvStructure.get()) << std::endl;
                } 
                // this is OK since calle holds also owns it
                m_channelGet.reset();
            }
            
            m_event.signal();
            
        }
        else
        {
            std::cerr << "[" << m_channelName << "] failed to get: " << status.toString() << std::endl;
            {
                Lock lock(m_pointerMutex);
                // this is OK since calle holds also owns it
                m_channelGet.reset();
            }
        }
        
    }

    bool waitUntilGet(double timeOut)
    {
        return m_event.wait(timeOut);
    }
};

class MonitorRequesterImpl : public MonitorRequester
{
	private:

    String m_channelName;

    public:

    MonitorRequesterImpl(String channelName) : m_channelName(channelName) {};

    virtual String getRequesterName()
    {
        return "MonitorRequesterImpl";
    };

    virtual void message(String const & message,MessageType messageType)
    {
        std::cerr << "[" << getRequesterName() << "] message(" << message << ", " << getMessageTypeName(messageType) << ")" << std::endl;
    }

    virtual void monitorConnect(const epics::pvData::Status& status, Monitor::shared_pointer const & monitor, StructureConstPtr const & /*structure*/)
    {
        if (status.isSuccess())
        {
        	/*
            String str;
            structure->toString(&str);
            std::cout << str << std::endl;
        	*/

            Status startStatus = monitor->start();
            // show error
            // TODO and exit
            if (!startStatus.isSuccess())
            {
                std::cerr << "[" << m_channelName << "] channel monitor start: " << startStatus.toString() << std::endl;
            }

        }
        else
        {
            std::cerr << "monitorConnect(" << status.toString() << ")" << std::endl;
        }
    }

    virtual void monitorEvent(Monitor::shared_pointer const & monitor)
    {

		MonitorElement::shared_pointer element;
		while (element = monitor->poll())
		{
            if (mode == ValueOnlyMode)
            {
                PVField::shared_pointer value = element->pvStructurePtr->getSubField("value");
                if (value.get() == 0)
                {
                	std::cerr << "no 'value' field" << std::endl;
                    return;
                }

                if (fieldSeparator == ' ' && value->getField()->getType() == scalar)
                	std::cout << std::setw(30) << std::left << m_channelName;
                else
                	std::cout << m_channelName;

                std::cout << fieldSeparator;

                terse(std::cout, value) << std::endl;
            }
            else if (mode == TerseMode)
            {
            	if (fieldSeparator == ' ')
                	std::cout << std::setw(30) << std::left << m_channelName;
                else
                	std::cout << m_channelName;

                std::cout << fieldSeparator;

                terseStructure(std::cout, element->pvStructurePtr) << std::endl;
            }
            else
            {
            	std::cout << m_channelName << std::endl;
                std::cout << *(element->pvStructurePtr.get()) << std::endl << std::endl;
            }

			monitor->release(element);
		}

    }

    virtual void unlisten(Monitor::shared_pointer const & /*monitor*/)
    {
        std::cerr << "unlisten" << std::endl;
    }
};



/*+**************************************************************************
 *
 * Function:	main
 *
 * Description:	pvget main()
 * 		Evaluate command line options, set up CA, connect the
 * 		channels, print the data as requested
 *
 * Arg(s) In:	[options] <pv-name>...
 *
 * Arg(s) Out:	none
 *
 * Return(s):	Standard return code (0=success, 1=error)
 *
 **************************************************************************-*/

int main (int argc, char *argv[])
{
    int opt;                    /* getopt() current option */
    bool debug = false;
    bool cleanupAndReport = false;
    bool monitor = false;

    setvbuf(stdout,NULL,_IOLBF,BUFSIZ);    /* Set stdout to line buffering */

    while ((opt = getopt(argc, argv, ":hr:w:tmdcF:")) != -1) {
        switch (opt) {
        case 'h':               /* Print usage */
            usage();
            return 0;
        case 'w':               /* Set CA timeout value */
            if(epicsScanDouble(optarg, &timeOut) != 1 || timeOut <= 0.0)
            {
                fprintf(stderr, "'%s' is not a valid timeout value "
                        "- ignored. ('pvget -h' for help.)\n", optarg);
                timeOut = DEFAULT_TIMEOUT;
            }
            break;
        case 'r':               /* Set CA timeout value */
            request = optarg;
            // do not override terse mode
            if (mode == ValueOnlyMode) mode = StructureMode;
            break;
        case 't':               /* Terse mode */
            mode = TerseMode;
            break;
        case 'm':               /* Monitor mode */
            monitor = true;
            break;
        case 'd':               /* Debug log level */
            debug = true;
            break;
        case 'c':               /* Clean-up and report used instance count */
            cleanupAndReport = true;
            break;
        case 'F':               /* Store this for output formatting */
            fieldSeparator = (char) *optarg;
            break;
        case '?':
            fprintf(stderr,
                    "Unrecognized option: '-%c'. ('pvget -h' for help.)\n",
                    optopt);
            return 1;
        case ':':
            fprintf(stderr,
                    "Option '-%c' requires an argument. ('pvget -h' for help.)\n",
                    optopt);
            return 1;
        default :
            usage();
            return 1;
        }
    }

    int nPvs = argc - optind;       /* Remaining arg list are PV names */
    if (nPvs < 1)
    {
        fprintf(stderr, "No pv name(s) specified. ('pvget -h' for help.)\n");
        return 1;
    }

    vector<string> pvs;     /* Array of PV structures */
    for (int n = 0; optind < argc; n++, optind++)
        pvs.push_back(argv[optind]);       /* Copy PV names from command line */


    SET_LOG_LEVEL(debug ? logLevelDebug : logLevelError);

    std::cout << std::boolalpha;
    terseSeparator(fieldSeparator);

    bool allOK = true;

    {
        Requester::shared_pointer requester(new RequesterImpl("pvget"));
    
        PVStructure::shared_pointer pvRequest = getCreateRequest()->createRequest(request, requester);
        if(pvRequest.get()==NULL) {
            fprintf(stderr, "failed to parse request string\n");
            return 1;
        }
        
        ClientFactory::start();
        ChannelProvider::shared_pointer provider = getChannelAccess()->getProvider("pvAccess");
    
        // first connect to all, this allows resource (e.g. TCP connection) sharing
        vector<Channel::shared_pointer> channels(nPvs);
        for (int n = 0; n < nPvs; n++)
        {
            shared_ptr<ChannelRequesterImpl> channelRequesterImpl(new ChannelRequesterImpl()); 
            channels[n] = provider->createChannel(pvs[n], channelRequesterImpl);
        }
        
        // for now a simple iterating sync implementation, guarantees order
        for (int n = 0; n < nPvs; n++)
        {
            /*
            shared_ptr<ChannelRequesterImpl> channelRequesterImpl(new ChannelRequesterImpl()); 
            Channel::shared_pointer channel = provider->createChannel(pvs[n], channelRequesterImpl);
            */
            
            Channel::shared_pointer channel = channels[n];
            shared_ptr<ChannelRequesterImpl> channelRequesterImpl = dynamic_pointer_cast<ChannelRequesterImpl>(channel->getChannelRequester());
             
            if (channelRequesterImpl->waitUntilConnected(timeOut))
            {
            	if (!monitor)
            	{
					shared_ptr<ChannelGetRequesterImpl> getRequesterImpl(new ChannelGetRequesterImpl(channel->getChannelName()));
					ChannelGet::shared_pointer channelGet = channel->createChannelGet(getRequesterImpl, pvRequest);
					allOK &= getRequesterImpl->waitUntilGet(timeOut);
            	}
            	else
            	{
					shared_ptr<MonitorRequesterImpl> monitorRequesterImpl(new MonitorRequesterImpl(channel->getChannelName()));
					Monitor::shared_pointer monitorGet = channel->createMonitor(monitorRequesterImpl, pvRequest);
					allOK &= true;
            	}
            }
            else
            {
                allOK = false;
                channel->destroy();
                std::cerr << "[" << channel->getChannelName() << "] connection timeout" << std::endl;
            }
        }    

        if (allOK && monitor)
        {
        	while (true)
        		epicsThreadSleep(timeOut);
        }

        ClientFactory::stop();
    }

    if (cleanupAndReport)
    {
        // TODO implement wait on context
        epicsThreadSleep ( 3.0 );
        //std::cout << "-----------------------------------------------------------------------" << std::endl;
        //epicsExitCallAtExits();
    }

    return allOK ? 0 : 1;
}
