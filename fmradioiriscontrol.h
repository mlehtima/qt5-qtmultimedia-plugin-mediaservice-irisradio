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

#ifndef __FMRADIOIRISCONTROL_H
#define __FMRADIOIRISCONTROL_H

#include <QDateTime>
#include <QRadioTunerControl>
#include <QRadioTuner>
#include <QRadioDataControl>
#include <QRadioData>
#include <QTimer>
#include <pthread.h>

QT_BEGIN_NAMESPACE

class FMRadioIrisControl : public QObject
{
    Q_OBJECT
public:
    FMRadioIrisControl();
    ~FMRadioIrisControl();

    bool isTunerAvailable() const;
    QMultimedia::AvailabilityStatus tunerAvailability() const;

    QRadioTuner::State tunerState() const;

    QRadioTuner::Band band() const;
    void setBand(QRadioTuner::Band b);
    bool isBandSupported(QRadioTuner::Band b) const;

    int frequency() const;
    void setFrequency(int frequency);

    bool isStereo() const;
    QRadioTuner::StereoMode stereoMode() const;
    void setStereoMode(QRadioTuner::StereoMode mode);

    int signalStrength() const;

    int volume() const;
    void setVolume(int volume);

    bool isMuted() const;
    void setMuted(bool muted);

    void setSearching(bool search);
    bool isSearching() const;

    bool isAntennaConnected() const;

    void searchForward();
    void searchBackward();
    void searchAllStations(QRadioTuner::SearchMode searchMode = QRadioTuner::SearchFast);
    void cancelSearch();

    void start();
    void stop();

    QRadioTuner::Error tunerError() const;
    QString tunerErrorString() const;

    bool isRdsAvailable() const;
    QMultimedia::AvailabilityStatus rdsAvailability() const;

    QString stationId() const;
    QRadioData::ProgramType programType() const;
    QString programTypeName() const;
    QString stationName() const;
    QString radioText() const;
    void setAlternativeFrequenciesEnabled(bool enabled);
    bool isAlternativeFrequenciesEnabled() const;

    QRadioData::Error rdsError() const;
    QString rdsErrorString() const;

public Q_SLOTS:

signals:
    void tunerAvailabilityChanged(bool available);
    void tunerAvailabilityChanged(QMultimedia::AvailabilityStatus availability);
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

    void stationIdChanged(QString stationId);
    void programTypeChanged(QRadioData::ProgramType programType);
    void programTypeNameChanged(QString programTypeName);
    void stationNameChanged(QString stationName);
    void radioTextChanged(QString radioText);
    void alternativeFrequenciesEnabledChanged(bool enabled);
    void error(QRadioData::Error err);

private slots:
    void search();
    void doSeek(int dir);

private:
    pthread_t event_listener_thread;

    bool initRadio();
    int fd;

    bool m_tunerError;
    bool muted;
    bool stereo;
    bool low;
    bool tunerAvailable;
    int  tuners;
    int  step;
    int  vol;
    int  sig;
    bool scanning;
    bool forward;
    QRadioTuner::Band currentBand;
    qint64 freqMin;
    qint64 freqMax;
    qint64 currentFreq;
    QTime  playTime;
    QTimer* timer;
    QRadioTuner::SearchMode searchMode;
    qint64 searchStartFreq;
    qint64 searchPreviousFreq;

    bool rdsAvailable;
    bool m_rdsError;
    QString m_stationId;
    QRadioData::ProgramType m_programType;
    QString m_programTypeName;
    QString m_stationName;
    QString m_radioText;
    bool m_alternativeFrequenciesEnabled;

    bool SetFreq(int frequency);//Hz
    int GetFreq(void);//Hz
    void GetCaps(void);
    int GetBuffer(int type);
    int GetEvent();
    void DoSeek(int dir);//0 is down, 1 is up
    bool SetTuner();
    bool GetTuner();
    bool SetCtrl(int id, int value);
    int GetCtrl(int id);
    static void *EventListener(void* context);
    unsigned int programTypeValue(int rdsStandard, unsigned int type);
    QString programTypeNameString(int rdsStandard, unsigned int type);
};

QT_END_NAMESPACE

#endif // __FMRADIOIRISCONTROL_H
