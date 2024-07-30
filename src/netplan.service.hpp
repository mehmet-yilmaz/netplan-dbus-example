#ifndef ___NETPLAN_DBUS_SERVICE_HPP___
#define ___NETPLAN_DBUS_SERVICE_HPP___

#include <memory>
#include <string>
#include <dbus-cxx-2.0/dbus-cxx.h>
#include <dbus-cxx-2.0/dbus-cxx/path.h>
#include <dbus-cxx-2.0/dbus-cxx/dispatcher.h>
#include <dbus-cxx-2.0/dbus-cxx/connection.h>
#include <dbus-cxx-2.0/dbus-cxx/objectproxy.h>
#include <dbus-cxx-2.0/dbus-cxx/methodproxybase.h>

class NetplanService
{
public:
    NetplanService()
    {
        this->m_path.clear(); // Ensure that it will return "True" when we call ".empty()" method as default to start a new session!
        this->setConfigurationFile();
        this->connect();
    };

    void connect()
    {
        try
        {
            this->m_dispatcher = DBus::StandaloneDispatcher::create();

            // Netplan DBus service runs on the System Bus.
            // For connecting to Netplan, we have to create a connection in the System Bus
            // !!! IMPORTANT: System Bus requires ROOT Privilages!!!
            this->m_connection = this->m_dispatcher->create_connection(DBus::BusType::SYSTEM);
            this->m_object_proxy = this->m_connection->create_object_proxy("io.netplan.Netplan", "/io/netplan/Netplan");

            this->m_path.clear(); // Ensure that it will return "True" when we call ".empty()" method as default to start a new session!
        }
        catch (const DBus::Error &err)
        {
            this->handleDBusError(err);
        }
    };

    // Confguration .yaml file will be stored and processed with this filename under the default netplan configuration folder.
    void setConfigurationFile(const char *filename = "dbus-test-config")
    {
        this->m_filename = filename;
    };

    const std::string configurationPath()
    {
        if (this->m_path.empty())
            this->initConfigurationPath();
        return this->m_path;
    };

    const std::string getConfiguration()
    {
        try
        {
            const std::string path = this->configurationPath();
            this->m_object_proxy->set_path(path);
            DBus::MethodProxy<std::string()> &Get = *(this->m_object_proxy->create_method<std::string()>("io.netplan.Netplan.Config", "Get"));
            return Get();
        }
        catch (const DBus::Error &err)
        {
            this->handleDBusError(err);
        }
    };

    const bool setConfiguration(const std::string &target, const std::string &value)
    {
        try
        {
            const std::string path = this->configurationPath();
            this->m_object_proxy->set_path(path);
            std::string config = target + "=" + value;
            DBus::MethodProxy<bool(std::string, std::string)> &Set = *(this->m_object_proxy->create_method<bool(std::string, std::string)>("io.netplan.Netplan.Config", "Set"));
            return Set(config, this->m_filename);
        }
        catch (const DBus::Error &err)
        {
            this->handleDBusError(err);
        }
    };
    bool tryConfiguration(const uint32_t &timeout)
    {
        try
        {
            const std::string path = this->configurationPath();
            this->m_object_proxy->set_path(path);
            DBus::MethodProxy<bool(uint32_t)> &Try = *(this->m_object_proxy->create_method<bool(uint32_t)>("io.netplan.Netplan.Config", "Try"));
            return Try(timeout);
        }
        catch (DBus::Error &err)
        {
            this->handleDBusError(err);
        };
    };
    bool applyConfiguration()
    {
        try
        {
            const std::string path = this->configurationPath();
            this->m_object_proxy->set_path(path);
            DBus::MethodProxy<bool()> &Apply = *(this->m_object_proxy->create_method<bool()>("io.netplan.Netplan.Config", "Apply"));
            return Apply();
        }
        catch (DBus::Error &err)
        {
            this->handleDBusError(err);
        };
    };

    bool cancelConfiguration()
    {
        try
        {
            const std::string path = this->configurationPath();
            this->m_object_proxy->set_path(path);
            DBus::MethodProxy<bool()> &Cancel = *(this->m_object_proxy->create_method<bool()>("io.netplan.Netplan.Config", "Cancel"));
            const bool result = Cancel();
            this->m_path.clear();
            return result;
        }
        catch (DBus::Error &err)
        {
            this->handleDBusError(err);
        };
    };

private:
    std::shared_ptr<DBus::Dispatcher> m_dispatcher{nullptr};
    std::shared_ptr<DBus::Connection> m_connection{nullptr};
    std::shared_ptr<DBus::ObjectProxy> m_object_proxy{nullptr};
    std::string m_path{NULL};
    std::string m_filename{NULL};

    void initConfigurationPath()
    {
        try
        {
            // Ensure that object path corrected!
            this->m_object_proxy->set_path("/io/netplan/Netplan");
            // Create a Method Proxy to represent Config Method of the Netplan-DBus API
            DBus::MethodProxy<DBus::Path()> &Config = *(this->m_object_proxy->create_method<DBus::Path()>("io.netplan.Netplan", "Config"));
            // Run the proxy method to start a new Session and get the Configuration path as a return value and assing it to the m_configuratoin_path
            this->m_path = Config();
        }
        catch (const DBus::Error &err)
        {
            this->handleDBusError(err);
        }
    };
    void handleDBusError(const DBus::Error &err)
    {
        // Reset the session in case of error
        this->m_path.clear();

        // Print out the error.
        printf("ERROR \n\tDBus Error: %s \n\tType: %s \n\tMessage: %s\n", err.name(), err.what(), err.message());
    };
};

#endif // !___NETPLAN_DBUS_SERVICE_HPP___