#ifndef ___CLI_HPP___
#define ___CLI_HPP___

#include <iostream>
#include <stdio.h>
#include "netplan.service.hpp"

class Cli
{
public:
    Cli() {
    };
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
