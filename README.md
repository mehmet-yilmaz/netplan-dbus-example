# Netplan-DBus-Example

This is an example application to explain how to communicate and set the network settings over DBus.

Please read the following article first:

https://mehmet-yilmaz.medium.com/manage-ubuntu-linux-networking-programmatically-with-netplan-and-d-bus-in-c-c-f608e4f351ed

### Dependencies

- dbus-cxx

#### Build Notes

- Set include path to `dbus-cxx` library correctly.
- Set LFlags for `-ldbus-cxx`
- **Since Netplan runs on the System Session, running this application need the `sudo` privilages. Please be careful and never change the configuration if you are not totaly sure about.**


---

# Manage Ubuntu Linux Networking Programmatically with Netplan and D-Bus in C/C++

When we need to manage network adapter settings programmatically, we often encounter the challenge of using system calls to override static configurations. These attempts frequently lead to panic and crashes in the network services. At best, we might temporarily achieve our objectives until the next reboot, and even then, a correct implementation is rare.

If DHCP enters the picture, things become even more complicated. In addition to handling other complexities, we might need to implement or interact with a DHCP client.

As we have all experienced, deviating into these tangential and irrational concepts during development often leads to significant setbacks and failures.

In light of these challenges, we need to consider alternative approaches. Given that network management is operated by OS-provided services, we should aim to communicate correctly with these services instead of attempting to take over their roles.

[https://github.com/mehmet-yilmaz/netplan-dbus-example](https://github.com/mehmet-yilmaz/netplan-dbus-example)

---

> PS: If you work on a remote host, remember that changing the network settings may cause the connection lost. Do not apply any changes to your active connection interface if you are not sure what are you doing.

---

## How Do OS Services Communicate with Each Other?

### Inter-Process Communication
Operating systems consist of multiple services and processes that need to work together smoothly. IPC is the communication layer that allows these services to exchange data and synchronize actions effectively.

### What is D-Bus?
D-Bus (Desktop Bus) is a message-oriented bus system and Remote Procedure Call (RPC) mechanism in Linux that enables communication between different processes on the same machine. It is designed to be simple and lightweight, suitable for both desktop and embedded environments.

#### Key Features of D-Bus:

- Message Passing: Processes can send and receive messages, including signals, method calls, and return values.
- Service Registration: Processes can register services on the bus, making functionalities available to others.
- Introspection: D-Bus supports dynamic discovery of services and their capabilities.
- Security: Includes mechanisms for authenticating and authorizing messages.
- Efficiency: Designed to be efficient in both performance and resource usage.
- Using Netplan on Ubuntu
- For this article, we will use Ubuntu Server 22.04 LTS and its default network configuration service, Netplan. However, you can choose any OS and network service that provides a D-Bus API.

### What is Netplan?
Netplan is a tool that provides a high-level configuration interface for network settings using YAML files. It abstracts the network configuration by communicating with supported network services like `NetworkManager` and `systemd-networkd`.

For more information: [Netplan Documentation](https://netplan.readthedocs.io)

Netplan also provides a simple D-Bus API and a CLI example for communication with the Netplan service over D-Bus:

[Netplan D-Bus API](https://netplan.readthedocs.io/en/stable/netplan-dbus/)
[CLI Example](https://netplan.readthedocs.io/en/stable/dbus-config/)

## How to use D-Bus configuration API
See also: Netplan D-Bus reference, busctl reference. Copy the current state from/{etc,run,lib}/netplan/*.yaml by…
netplan.readthedocs.io

### DBus CLI Example:
Before diving in, please try the following CLI example to get the current configuration for ensuring that all services working correctly.

> busctl is a DBus communication CLI tool already installed in the Ubuntu system, there are some other alternatives that you can install if needed.

We will use the following command to call the `Config()` method, which starts a session and returns the `Netplan-DBus` configuration path.

**Netplan-DBus service runs on the System Session, so you will need sudo privileges to communicate with it.**

```
sudo busctl call io.netplan.Netplan /io/netplan/Netplan io.netplan.Netplan Config
```
Expected output:
```
o "/io/netplan/Netplan/config/ULJIU0"
```
`o` represents the output type `OBJECT_PATH` and the `”/io/netplan/Netplan/config/ULJIU0"` as value.

> OBJECT_PATH value is a session-based dynamic value, we will do all the configuration with this path during the session, but it will be changed on the next session after the timeout or if you call the Config method again.

Now we can call the `Get()` method to this configuration path as `busctl call io.netplan.Netplan [ConfigurationPath] io.netplan.Netplan.Config` Get to read the current configuration as examples here.

```
sudo busctl call io.netplan.Netplan /io/netplan/Netplan/config/ULJIU0 io.netplan.Netplan.Config Get
```

Expected output depends on your existing network configuration differs:
```
s "network:\n  ethernets:\n    eth0:\n      dhcp4: true\n  renderer: networkd\n  version: 2\n"
```
`s` represents the output type `STRING` and the value is the string parse output of the Netplans `.yaml` current configuration file.

Now we can move to the programming.

> PS: If you work on a remote host, remember that changing the network settings may cause the connection lost. Do not apply any changes to your active connection interface if you are not sure what are you doing.

## Lets Coding…
After this introduction, we can focus on implementing this in our C++ Program.

### Dependencies
There are different DBus libraries you can choose from. I prefer the dbus-cxx library because it provides the native C++ implementation with Smart pointers support.

Please take a look at the project: [https://dbus-cxx.github.io](https://dbus-cxx.github.io)

Installation of the `dbus-cxx` library on `Ubuntu 22.04 LTS`.

Install the pre-requirements
- ≥ C++17
- cmake(≥3.8)
- make
- g++
- libsig++(≥3.0)
- expat
- libdbus

```
sudo apt install build-essentials cmake libdbus-1-dev libexpat1-dev

git clone https://github.com/dbus-cxx/libsigc--3.0.git
cd libsigc--3.0
cd build
cmake ..
make install
```

Please refer to the documentation for your operation system.

[dbus-cxx: dbus-cxx Library - dbus-cxx.github.io](https://dbus-cxx.github.io)
Do not forget to link the library `dbus-cxx` to your compiler with -`ldbus-cxx` flag for using it.

### Finally, we are ready to hands-on.

I will create a service class that will interface the Netplan-DBus API and a basic CLI to interact with.

Project Structure

```
netplan-dbus-example/
├─ netplan.service.hpp
├─ cli.class.hpp
├─ main.cpp
```

First of all, I will create the `netplan.service.hpp` file

```
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
        printf("ERROR \n\tDBus Error: %s \n\tType: %s \n\tMessage: %s\n", err.name().c_str(), err.what().c_str(), err.message().c_str());
    };
};

#endif // !___NETPLAN_DBUS_SERVICE_HPP___
```

then the cli.hpp file;

```
#ifndef ___CLI_HPP___
#define ___CLI_HPP___

#include <iostream>
#include <stdio.h>
#include "netplan.service.hpp"

class Cli
{
public:
    Cli() {};
    void run()
    {
        this->m_running = true;
        while (this->m_running)
        {
            this->printmenu();
        }
    };
    void printmenu()
    {
        printf("\n------------MENU-------------\n");
        for (const auto &item : menu)
        {
            printf("\n%c \t %s", item.first, item.second.c_str());
        }
        printf("\nPlease Enter Your Selection: ");
        this->readmenu();
    };

    void readmenu()
    {
        char c;
        std::cin >> c;
        switch (c)
        {
        case 'P':
            this->configurationPath();
            break;
        case 'G':
            this->getConfiguration();
            break;
        case 'S':
            this->setMenu();
            break;
        case 'Q':
            this->stop();
            break;
        default:
            break;
        }
    };

private:
    std::atomic_bool m_running = true;
    std::unique_ptr<NetplanService>
        netplanService = std::make_unique<NetplanService>();
    std::map<const char, const std::string> menu{
        {'P', "Configuration Path"},
        {'G', "Get Configuration"},
        {'S', "Set Configuration"},
        {'Q', "Quit"}};

    void stop()
    {
        this->m_running = false;
    };

    void configurationPath()
    {
        const std::string path = this->netplanService->configurationPath();
        printf("\n\t\tConfigiration Path: %s", path.c_str());
    };

    void getConfiguration()
    {
        const std::string configuration = this->netplanService->getConfiguration();
        printf("\nConfiguration\n%s\n", configuration.c_str());
    };

    void setConfiguration(const std::string &target, const std::string &value)
    {
        this->netplanService->setConfiguration(target, value);
    };

    void setMenu()
    {
        printf("\n------------------");
        this->getConfiguration();
        printf("\n\tSet Configuration");
        printf("\n\tEnter the configuration target: ");
        std::string target;
        std::cin >> target;
        printf("\n\tEnter the configuration value: ");
        std::string value;
        std::cin >> value;
        this->setConfiguration(target, value);
    };

protected:
};

#endif // !___CLI_HPP___
```


and the main.cpp file
```
#include "cli.hpp"
int main(int argc, char *argv[])
{

    const auto cli = std::make_unique<Cli>();
    if (cli)
        cli->run();
    return 0;
}
```

> Since the Netplan runs on the System Session, we have to run our application with sudo privileges

Running Application Output:

```
------------MENU-------------

G        Get Configuration
P        Configuration Path
Q        Quit
S        Set Configuration
Please Enter Your Selection: 
```

Configuration Path Selection:

```
------------MENU-------------

G        Get Configuration
P        Configuration Path
Q        Quit
S        Set Configuration
Please Enter Your Selection: P

                Configiration Path: /io/netplan/Netplan/config/EF2OR2
```

Get Configuration Selection:

```
------------MENU-------------

G        Get Configuration
P        Configuration Path
Q        Quit
S        Set Configuration
Please Enter Your Selection: G

Configuration
network:
  version: 2
  ethernets:
    enxf8e43b893689:
      dhcp4: true
    eno1:
      dhcp4: true
```

Set Configuration:

```
------------MENU-------------

G        Get Configuration
P        Configuration Path
Q        Quit
S        Set Configuration
Please Enter Your Selection: S

------------------
Configuration
network:
  version: 2
  ethernets:
    enxf8e43b893689:
      dhcp4: true
    eno1:
      dhcp4: true


        Set Configuration
        Enter the configuration target: 

```
Here we can test and change the interface.

> I have plug a USB-ETH adaptor for testing, if you are not sure which interface you are working on, do not try this.

`ethernets.enxf8e43b893689.dhcp4` value to `false` .

```
------------MENU-------------

G        Get Configuration
P        Configuration Path
Q        Quit
S        Set Configuration
Please Enter Your Selection: S

------------------
Configuration
network:
  version: 2
  ethernets:
    enxf8e43b893689:
      dhcp4: true
    eno1:
      dhcp4: true


        Set Configuration
        Enter the configuration target: ethernets.enxf8e43b893689.dhcp4

        Enter the configuration value: false
```

It will change the interface `enxf8e43b893689.dhcp4` to `false` but since we do not apply this. It will not affect anything in this application.

Complete example could be inspected on github.
[mehmet-yilmaz/netplan-dbus-example](https://github.com/mehmet-yilmaz/netplan-dbus-example)

This was all, you can implement your requirements and communicate with Netplan or any other service over DBus.

I hope this helps you.
