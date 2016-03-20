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

#include "fmradioservice.h"
#include "fmradiotunercontrol.h"
#include "fmradiodatacontrol.h"
#include "fmradioiriscontrol.h"

#include <QDebug>

QT_BEGIN_NAMESPACE

FMRadioService::FMRadioService(QObject *parent):
   QMediaService(parent)
{
    qDebug("Instantiating QMediaService...");

    m_irisControl = new FMRadioIrisControl();
    m_tunerControl = new FMRadioTunerControl(this, m_irisControl);
    m_dataControl = new FMRadioDataControl(this, m_irisControl);
}

FMRadioService::~FMRadioService()
{
    delete m_dataControl;
    delete m_tunerControl;
    delete m_irisControl;
}

QMediaControl *FMRadioService::requestControl(const char *name)
{
    qDebug("Requesting control for %s...", name);

    if (qstrcmp(name, QRadioTunerControl_iid) == 0)
        return m_tunerControl;

    if (qstrcmp(name, QRadioDataControl_iid) == 0)
        return m_dataControl;

    return 0;
}

void FMRadioService::releaseControl(QMediaControl *control)
{
}

QT_END_NAMESPACE
