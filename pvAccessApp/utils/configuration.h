/**
 * Copyright - See the COPYRIGHT that is included with this distribution.
 * pvAccessCPP is distributed subject to a Software License Agreement found
 * in file LICENSE that is included with this distribution.
 */

#ifndef CONFIGURATION_H
#define CONFIGURATION_H

#include <pv/pvType.h>
#include <pv/noDefaultMethods.h>
#include <pv/lock.h>
#include <pv/sharedPtr.h>

#include <envDefs.h>


#include <string.h>
#include <iostream>
#include <sstream>
#include <fstream>
#include <map>

// TODO implement using smart pointers

namespace epics {
namespace pvAccess {

class Properties
{
public:
	Properties();
	Properties(const std::string fileName);
	virtual ~Properties();

	void setProperty(const std::string key,const  std::string value);
	std::string getProperty(const std::string key);
	std::string getProperty(const std::string key, const std::string defaultValue);

	void store();
	void store(const std::string fileName);
	void load();
	void load(const std::string fileName);
	void list();

private:
	std::map<std::string,std::string> _properties;
	std::map<std::string,std::string>::iterator _propertiesIterator;
	std::ifstream* _infile;
	std::ofstream* _outfile;
	std::string _fileName;

	inline void	truncate(std::string& str)
	{
		while(str.length() != 0 && (str.at(0) == ' ' || str.at(0) == '\t'))
		{
			str.erase(0,1);
		}
		while(str.length() != 0 && (str.at(str.length()-1) == ' ' || str.at(str.length()-1) == '\t'))
		{
			str.erase(str.length()-1,1);
		}
	}
};



/**
 * Configuration
 */
class Configuration : private epics::pvData::NoDefaultMethods
{
public:
     POINTER_DEFINITIONS(Configuration);

	/**
	 * Destructor.
	 */
	virtual ~Configuration() {};
	/**
	 * Get the environment variable specified by name or return default value
	 * if it does not exist.
	 *
	 * @param name name of the environment variable to return.
	 * @param defualtValue default value to return if environment variable does not exists.
	 *
	 * @return environment variable value as bool or default value if it does not exist.
	 */
	virtual bool getPropertyAsBoolean(const std::string name, const bool defaultValue) = 0;
	/**
	 * Get the environment variable specified by name or return default value
	 * if it does not exist.
	 *
	 * @param name name of the environment variable to return.
	 * @param defualtValue default value to return if environment variable does not exists.
	 *
	 * @return environment variable value as int32 or default value if it does not exist.
	 */
	virtual epics::pvData::int32 getPropertyAsInteger(const std::string name, const epics::pvData::int32 defaultValue) = 0;
	/**
	 * Get the environment variable specified by name or return default value
	 * if it does not exist.
	 *
	 * @param name name of the environment variable to return.
	 * @param defualtValue default value to return if environment variable does not exists.
	 *
	 * @return environment variable value as float or default value if it does not exist.
	 */
	virtual float getPropertyAsFloat(const std::string name, const float defaultValue) = 0;
	/**
	 * Get the environment variable specified by name or return default value
	 * if it does not exist.
	 *
	 * @param name name of the environment variable to return.
	 * @param defualtValue default value to return if environment variable does not exists.
	 *
	 * @return environment variable value as double or default value if it does not exist.
	 */
	virtual float getPropertyAsDouble(const std::string name, const double defaultValue) = 0;
	/**
	 * Get the environment variable specified by name or return default value
	 * if it does not exist.
	 *
	 * @param name name of the environment variable to return.
	 * @param defualtValue default value to return if environment variable does not exists.
	 *
	 * @return environment variable value as std::string or default value if it does not exist.
	 */
	virtual std::string getPropertyAsString(const std::string name, const std::string defaultValue) = 0;
};

class SystemConfigurationImpl: public Configuration
{
public:
	SystemConfigurationImpl();
	~SystemConfigurationImpl();
	bool getPropertyAsBoolean(const std::string name, const bool defaultValue);
	epics::pvData::int32 getPropertyAsInteger(const std::string name, const epics::pvData::int32 defaultValue);
	float getPropertyAsFloat(const std::string name, const float defaultValue);
	float getPropertyAsDouble(const std::string name, const double defaultValue);
	std::string getPropertyAsString(const std::string name, std::string defaultValue);
	Properties* _properties;
private:
	ENV_PARAM _envParam;
	std::istringstream _ibuffer;
	std::ostringstream _obuffer;

};

/**
 * Configuration provider.
 */
class ConfigurationProvider : private epics::pvData::NoDefaultMethods
{
public:
	/**
	 * Destructor.
	 */
	virtual ~ConfigurationProvider() {};
	/**
	 * Return configuration specified by name.
	 *
	 * @param name name of the configuration to return.
	 *
	 * @return configuration specified by name or NULL if it does not exists.
	 */
	virtual Configuration* getConfiguration(const std::string name) = 0;
	/**
	 * Register configuration.
	 *
	 * @param name name of the configuration to register.
	 * @param configuration configuration to register.
	 */
	virtual void registerConfiguration(const std::string name, const Configuration* configuration) = 0;
};

class ConfigurationProviderImpl: public ConfigurationProvider
{
public:
	ConfigurationProviderImpl();
	/**
	 * Destructor. Note: Registered configurations will be deleted!!
	 */
	~ConfigurationProviderImpl();
	Configuration* getConfiguration(const std::string name);
	void registerConfiguration(const std::string name, const Configuration* configuration);
private:
	epics::pvData::Mutex _mutex;
	std::map<std::string,const Configuration*> _configs;
	std::map<std::string,const Configuration*>::iterator _configsIter;
};

/**
 * Configuration factory.
 */
class ConfigurationFactory : private epics::pvData::NoDefaultMethods
{
public:
	/**
	 * Lazily creates configuration provider.
	 *
	 * @param name name of the configuration to register.
	 * @param configuration configuration to register.
	 *
	 * @return configuration provider
	 */
	static ConfigurationProviderImpl* getProvider();

private:
	ConfigurationFactory() {};
	static ConfigurationProviderImpl* _configurationProvider;
	static epics::pvData::Mutex _conf_factory_mutex;
};

}}

#endif  /* CONFIGURATION_H */
