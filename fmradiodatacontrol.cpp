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

#include "fmradiodatacontrol.h"

QT_BEGIN_NAMESPACE

FMRadioDataControl::FMRadioDataControl(QObject *parent, FMRadioIrisControl *ctrl)
   : QRadioDataControl(parent), control(ctrl)
{
    connect(control, SIGNAL(stationIdChanged(QString)),
               this, SIGNAL(stationIdChanged(QString)));
    connect(control, SIGNAL(programTypeChanged(QRadioData::ProgramType)),
               this, SIGNAL(programTypeChanged(QRadioData::ProgramType)));
    connect(control, SIGNAL(programTypeNameChanged(QString)),
               this, SIGNAL(programTypeNameChanged(QString)));
    connect(control, SIGNAL(stationNameChanged(QString)),
               this, SIGNAL(stationNameChanged(QString)));
    connect(control, SIGNAL(radioTextChanged(QString)),
               this, SIGNAL(radioTextChanged(QString)));
    connect(control, SIGNAL(alternativeFrequenciesEnabledChanged(bool)),
               this, SIGNAL(alternativeFrequenciesEnabledChanged(bool)));
    connect(control, SIGNAL(error(QRadioData::Error)),
               this, SIGNAL(error(QRadioData::Error)));
}

FMRadioDataControl::~FMRadioDataControl()
{
}

bool FMRadioDataControl::isAvailable() const
{
    return control->isRdsAvailable();
}

QMultimedia::AvailabilityStatus FMRadioDataControl::availability() const
{
    return control->rdsAvailability();
}

QString FMRadioDataControl::stationId() const
{
    return control->stationId();
}

QRadioData::ProgramType FMRadioDataControl::programType() const
{
    return control->programType();
}

QString FMRadioDataControl::programTypeName() const
{
    return control->programTypeName();
}

QString FMRadioDataControl::stationName() const
{
    return control->stationName();
}

QString FMRadioDataControl::radioText() const
{
    return control->radioText();
}

void FMRadioDataControl::setAlternativeFrequenciesEnabled(bool enabled)
{
    return control->setAlternativeFrequenciesEnabled(enabled);
}

bool FMRadioDataControl::isAlternativeFrequenciesEnabled() const
{
    return control->isAlternativeFrequenciesEnabled();
}

QRadioData::Error FMRadioDataControl::error() const
{
    return control->rdsError();
}

QString FMRadioDataControl::errorString() const
{
    return control->rdsErrorString();
}

QT_END_NAMESPACE
