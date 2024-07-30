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
