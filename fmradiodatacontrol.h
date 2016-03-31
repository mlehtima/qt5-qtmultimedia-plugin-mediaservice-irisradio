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

#ifndef __FMRADIODATACONTROL_H
#define __FMRADIODATACONTROL_H

#include "fmradioiriscontrol.h"

#include <QRadioDataControl>
#include <QRadioData>

QT_BEGIN_NAMESPACE

class FMRadioDataControl : public QRadioDataControl
{
   Q_OBJECT
public:
   FMRadioDataControl(QObject* parent = 0, FMRadioIrisControl *ctrl = 0);
   ~FMRadioDataControl();

   bool isAvailable() const;
   QMultimedia::AvailabilityStatus availability() const;

   QString stationId() const;
   QRadioData::ProgramType programType() const;
   QString programTypeName() const;
   QString stationName() const;
   QString radioText() const;
   void setAlternativeFrequenciesEnabled(bool enabled);
   bool isAlternativeFrequenciesEnabled() const;

   QRadioData::Error error() const;
   QString errorString() const;

private:
    FMRadioIrisControl *control;
};

QT_END_NAMESPACE

#endif // __FMRADIODATACONTROL_H
