// Host Account Management
#include "ham.h"
#include "dbus-proxy.h"
#include "../shared/dbus-address.h" /* DBUS_BUS_NAME_BASE, DBUS_OBJ_PATH_BASE */

int ham_userdel(const char * login)
{
    DBus::BusDispatcher         dispatcher;
    DBus::default_dispatcher = &dispatcher;
    DBus::Connection conn    = DBus::Connection::SystemBus();

    accounts_proxy_c interface(conn, DBUS_BUS_NAME_BASE, DBUS_OBJ_PATH_BASE);
    ::DBus::Struct< bool, std::string > ret = interface.userdel(login);

    return ret._1;
}


