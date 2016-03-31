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

#include "fmradiotunercontrol.h"

QT_BEGIN_NAMESPACE

FMRadioTunerControl::FMRadioTunerControl(QObject *parent, FMRadioIrisControl *ctrl)
   : QRadioTunerControl(parent), control(ctrl)
{
    connect(control, SIGNAL(stateChanged(QRadioTuner::State)),
               this, SIGNAL(stateChanged(QRadioTuner::State)));
    connect(control, SIGNAL(bandChanged(QRadioTuner::Band)),
               this, SIGNAL(bandChanged(QRadioTuner::Band)));
    connect(control, SIGNAL(frequencyChanged(int)),
               this, SIGNAL(frequencyChanged(int)));
    connect(control, SIGNAL(stereoStatusChanged(bool)),
               this, SIGNAL(stereoStatusChanged(bool)));
    connect(control, SIGNAL(searchingChanged(bool)),
               this, SIGNAL(searchingChanged(bool)));
    connect(control, SIGNAL(signalStrengthChanged(int)),
               this, SIGNAL(signalStrengthChanged(int)));
    connect(control, SIGNAL(volumeChanged(int)),
               this, SIGNAL(volumeChanged(int)));
    connect(control, SIGNAL(mutedChanged(bool)),
               this, SIGNAL(mutedChanged(bool)));
    connect(control, SIGNAL(error(QRadioTuner::Error)),
               this, SIGNAL(error(QRadioTuner::Error)));
    connect(control, SIGNAL(stationFound(int, QString)),
               this, SIGNAL(stationFound(int, QString)));
    connect(control, SIGNAL(antennaConnectedChanged(bool)),
               this, SIGNAL(antennaConnectedChanged(bool)));
}

FMRadioTunerControl::~FMRadioTunerControl()
{
}

QRadioTuner::State FMRadioTunerControl::state() const
{
    return control->tunerState();
}

QRadioTuner::Band FMRadioTunerControl::band() const
{
    return control->band();
}

void FMRadioTunerControl::setBand(QRadioTuner::Band b)
{
    control->setBand(b);
}

bool FMRadioTunerControl::isBandSupported(QRadioTuner::Band b) const
{
    return control->isBandSupported(b);
}

int FMRadioTunerControl::frequency() const
{
    return control->frequency();
}

int FMRadioTunerControl::frequencyStep(QRadioTuner::Band b) const
{
    int step = 0;

    if (b == QRadioTuner::FM)
        step = 100000; // 100kHz steps
    else if (b == QRadioTuner::LW)
        step = 1000; // 1kHz steps
    else if (b == QRadioTuner::AM)
        step = 1000; // 1kHz steps
    else if (b == QRadioTuner::SW)
        step = 500; // 500Hz steps

    return step;
}

QPair<int,int> FMRadioTunerControl::frequencyRange(QRadioTuner::Band b) const
{
    if (b == QRadioTuner::AM)
        return qMakePair<int,int>(520000,1710000);
    else if (b == QRadioTuner::FM)
        return qMakePair<int,int>(87500000,108000000);
    else if (b == QRadioTuner::SW)
        return qMakePair<int,int>(1711111,30000000);
    else if (b == QRadioTuner::LW)
        return qMakePair<int,int>(148500,283500);

    return qMakePair<int,int>(0,0);
}

void FMRadioTunerControl::setFrequency(int frequency)
{
    control->setFrequency(frequency);
}

bool FMRadioTunerControl::isStereo() const
{
    return control->isStereo();
}

QRadioTuner::StereoMode FMRadioTunerControl::stereoMode() const
{
    return control->stereoMode();
}

void FMRadioTunerControl::setStereoMode(QRadioTuner::StereoMode mode)
{
    control->setStereoMode(mode);
}

int FMRadioTunerControl::signalStrength() const
{
    return control->signalStrength();
}

int FMRadioTunerControl::volume() const
{
    return 0;
}

void FMRadioTunerControl::setVolume(int)
{
}

bool FMRadioTunerControl::isMuted() const
{
    return control->isMuted();
}

void FMRadioTunerControl::setMuted(bool muted)
{
    control->setMuted(muted);
}

bool FMRadioTunerControl::isSearching() const
{
    return control->isSearching();
}

bool FMRadioTunerControl::isAntennaConnected() const
{
    return true;
}

void FMRadioTunerControl::searchForward()
{
    control->searchForward();
}

void FMRadioTunerControl::searchBackward()
{
    control->searchBackward();
}

void FMRadioTunerControl::searchAllStations(QRadioTuner::SearchMode searchMode)
{
    control->searchAllStations(searchMode);
}

void FMRadioTunerControl::cancelSearch()
{
    control->cancelSearch();
}

void FMRadioTunerControl::start()
{
    control->start();
}

void FMRadioTunerControl::stop()
{
    control->stop();
}

QRadioTuner::Error FMRadioTunerControl::error() const
{
    return control->tunerError();
}

QString FMRadioTunerControl::errorString() const
{
    return control->tunerErrorString();
}

QT_END_NAMESPACE
