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

#ifndef __FMRADIOTUNERCONTROL_H
#define __FMRADIOTUNERCONTROL_H

#include "fmradioiriscontrol.h"

#include <QRadioTunerControl>
#include <QRadioTuner>

QT_BEGIN_NAMESPACE

class FMRadioTunerControl : public QRadioTunerControl
{
    Q_OBJECT
public:
    FMRadioTunerControl(QObject* parent = 0, FMRadioIrisControl *ctrl = 0);
    ~FMRadioTunerControl();

    bool isAvailable() const;
    QMultimedia::AvailabilityStatus availability() const;

    QRadioTuner::State state() const;

    QRadioTuner::Band band() const;
    void setBand(QRadioTuner::Band b);
    bool isBandSupported(QRadioTuner::Band b) const;

    int frequency() const;
    int frequencyStep(QRadioTuner::Band b) const;
    QPair<int,int> frequencyRange(QRadioTuner::Band b) const;
    void setFrequency(int frequency);

    bool isStereo() const;
    QRadioTuner::StereoMode stereoMode() const;
    void setStereoMode(QRadioTuner::StereoMode mode);

    int signalStrength() const;

    int volume() const;
    void setVolume(int volume);

    bool isMuted() const;
    void setMuted(bool muted);

    bool isSearching() const;

    bool isAntennaConnected() const;

    void searchForward();
    void searchBackward();
    void searchAllStations(QRadioTuner::SearchMode searchMode = QRadioTuner::SearchFast);
    void cancelSearch();

    void start();
    void stop();

    QRadioTuner::Error error() const;
    QString errorString() const;

signals:
    void stateChanged(QRadioTuner::State state);
    void bandChanged(QRadioTuner::Band band);
    void frequencyChanged(int frequency);
    void stereoStatusChanged(bool stereo);
    void searchingChanged(bool searching);
    void signalStrengthChanged(int signalStrength);
    void volumeChanged(int volume);
    void mutedChanged(bool muted);
    void error(QRadioTuner::Error err);
    void stationFound(int frequency, QString stationId);
    void antennaConnectedChanged(bool connectionStatus);
    
    void availabilityChanged(bool available);
    void availabilityChanged(QMultimedia::AvailabilityStatus availability);

private:
    FMRadioIrisControl *control;
};

QT_END_NAMESPACE

#endif // __FMRADIOTUNERCONTROL_H
