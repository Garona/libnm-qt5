/*
Copyright 2011 Ilia Kats <ilia-kats@gmx.net>

This library is free software; you can redistribute it and/or
modify it under the terms of the GNU Lesser General Public
License as published by the Free Software Foundation; either
version 2.1 of the License, or (at your option) version 3, or any
later version accepted by the membership of KDE e.V. (or its
successor approved by the membership of KDE e.V.), which shall
act as a proxy defined in Section 6 of version 3 of the license.

This library is distributed in the hope that it will be useful,
but WITHOUT ANY WARRANTY; without even the implied warranty of
MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU
Lesser General Public License for more details.

You should have received a copy of the GNU Lesser General Public
License along with this library.  If not, see <http://www.gnu.org/licenses/>.
*/

#include "vpnconnection.h"
#include "activeconnection_p.h"
#include "connection.h"
#include "settings.h"
#include "device.h"

#include <QDBusObjectPath>

#include "manager_p.h"
#include "nm-vpn-connectioninterface.h"

class NetworkManager::VpnConnectionPrivate : public NetworkManager::ActiveConnectionPrivate
{
public:
    VpnConnectionPrivate(const QString &path)
        : ActiveConnectionPrivate(path), iface(NetworkManagerPrivate::DBUS_SERVICE, path, QDBusConnection::systemBus()) {
    }
    static NetworkManager::VpnConnection::State convertVpnConnectionState(uint state) {
        return (NetworkManager::VpnConnection::State)state;
    }
    QString banner;
    NetworkManager::VpnConnection::State state;
    OrgFreedesktopNetworkManagerVPNConnectionInterface iface;
};


NetworkManager::VpnConnection::VpnConnection(const QString &path, QObject *parent)
    : ActiveConnection(*new VpnConnectionPrivate(path), parent)
{
    Q_D(VpnConnection);
    d->banner = d->iface.banner();
    d->state = NetworkManager::VpnConnectionPrivate::convertVpnConnectionState(d->iface.vpnState());
    connect(&d->iface, SIGNAL(PropertiesChanged(QVariantMap)),
            this, SLOT(propertiesChanged(QVariantMap)));
    connect(&d->iface, SIGNAL(VpnStateChanged(uint,uint)),
            this, SLOT(vpnStateChanged(uint,uint)));
}

NetworkManager::VpnConnection::~VpnConnection()
{
}

QString NetworkManager::VpnConnection::banner() const
{
    Q_D(const VpnConnection);
    //return d->banner; // FIXME NM doesn't emit the Banner property change
    return d->iface.banner();
}

NetworkManager::VpnConnection::State NetworkManager::VpnConnection::state() const
{
    Q_D(const VpnConnection);
    return d->state;
}

void NetworkManager::VpnConnection::propertiesChanged(const QVariantMap &properties)
{
    Q_D(VpnConnection);

    QVariantMap::const_iterator it = properties.constBegin();
    while (it != properties.constEnd()) {
        const QString property = it.key();
        if (property == QLatin1String("Banner")) {
            d->banner = it->toString();
            emit bannerChanged(d->banner);
        } else if (property == QLatin1String("VpnState")) {
            d->state = NetworkManager::VpnConnectionPrivate::convertVpnConnectionState(it->toUInt());
            emit stateChanged(d->state);
        } else {
            qWarning() << Q_FUNC_INFO << "Unhandled property" << property;
        }
        ++it;
    }
}

void NetworkManager::VpnConnection::vpnStateChanged(uint state, uint reason)
{
    Q_D(VpnConnection);
    Q_UNUSED(reason);

    d->state = NetworkManager::VpnConnectionPrivate::convertVpnConnectionState(state);
    emit stateChanged(d->state);
}

NetworkManager::VpnConnection::operator VpnConnection *()
{
    Q_D(VpnConnection);
    if (d->vpn)
        return this;
    else
        return 0;
}

#include "vpnconnection.moc"
