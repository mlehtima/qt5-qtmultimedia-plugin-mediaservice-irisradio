/*
  Copyright (C) 2016 Matti Lehtimäki
  Contact: Matti Lehtimäki <matti.lehtimaki@gmail.com>
  All rights reserved.

  This library is free software; you can redistribute it and/or
  modify it under the terms of the GNU Lesser General Public
  License as published by the Free Software Foundation; either
  version 2.1 of the License, or (at your option) any later version.

  This library is distributed in the hope that it will be useful,
  but WITHOUT ANY WARRANTY; without even the implied warranty of
  MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU
  Lesser General Public License for more details.

  You should have received a copy of the GNU Lesser General Public
  License along with this library; if not, write to the Free Software
  Foundation, Inc., 51 Franklin Street, Fifth Floor, Boston, MA  02110-1301  USA
*/

#include "fmradioserviceplugin.h"
#include "fmradioservice.h"

#include <QDebug>
#include <QString>

QMediaService* FMRadioServicePlugin::create(const QString &key)
{
    qDebug("Instantiating FM Radio service...");
    if (key == QLatin1String(Q_MEDIASERVICE_RADIO))
        return new FMRadioService(this);

    qDebug("FM Radio service plugin: unsupported key: %s.", qPrintable(key));
    return NULL;
}

void FMRadioServicePlugin::release(QMediaService* service)
{
    delete service;
}
