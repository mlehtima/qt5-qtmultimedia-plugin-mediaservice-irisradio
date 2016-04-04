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

#include "fmradioiriscontrol.h"
#include "radio-iris-commands.h"

#include <QDebug>

#include <sys/stat.h>
#include <fcntl.h>

#include <stdio.h>
#include <errno.h>
#include <unistd.h>

#include <sys/ioctl.h>
#include <linux/videodev2.h>

static QString cconv = QString("áàéèíìóòúùÑÇŞß¡Ĳâäêëîïôöûüñçşğıĳªα©‰Ğěňőπ€£$←↑→↓º¹²³±İńűμ¿÷°¼½¾§ÁÀÉÈÍÌÓÒÚÙŘČŠŽÐĿÂÄÊËÎÏÔÖÛÜřčšžđŀÃÅÆŒŷÝÕØÞŊŔĆŚŹŦðãåæœŵýõøþŋŕćśźŧ ");

static void cconvert(QString *str) {
    int len = str->length();
    int i;
    for (i = 0; i < len; ++i) {
        if (str->at(i).toLatin1() > 0x7f) {
            str->replace(i, 1, cconv.at(str->at(i).toLatin1()-128));
        }
    }
}

FMRadioIrisControl::FMRadioIrisControl()
    : QObject(),
      m_workerThread(0),
      m_fd(-1),
      m_muted(false),
      m_stereo(false),
      m_low(false),
      m_tunerAvailable(false),
      m_signalStrength(0),
      m_scanning(false),
      m_forward(false),
      m_currentBand(QRadioTuner::FM),
      m_freqMin(0),
      m_freqMax(0),
      m_currentFreq(0),
      m_timer(new QTimer(this)),
      m_searchMode(QRadioTuner::SearchFast),
      m_searchStartFreq(0),
      m_searchPreviousFreq(0),
      m_rdsAvailable(false),
      m_rdsError(false),
      m_programType(QRadioData::Undefined)
{
    initRadio();
    m_timer->setInterval(2000);
    connect(m_timer, SIGNAL(timeout()), this, SLOT(search()));
    qDebug("Create FM Radio iris Control");
}

FMRadioIrisControl::~FMRadioIrisControl()
{
    m_timer->stop();
    m_workerThread->setQuit();
    SetCtrl(V4L2_CID_PRIVATE_IRIS_STATE, 0); // assuming this wakes blocking ioctl()
    m_workerThread->wait();
    delete m_workerThread;

    if (m_fd != -1) {
        close(m_fd);
    }
}

bool FMRadioIrisControl::initRadio()
{
    qDebug("Initialize radio");
    m_fd = open("/dev/radio0", O_RDONLY | O_NONBLOCK);
    if (m_fd != -1) {
        m_tunerAvailable = true;

        m_workerThread = new IrisWorkerThread(m_fd);
        connect(m_workerThread, SIGNAL(tunerAvailableChanged(bool)), this, SLOT(handleTunerAvailable(bool)));
        connect(m_workerThread, SIGNAL(rdsAvailableChanged(bool)), this, SLOT(handleRdsAvailable(bool)));
        connect(m_workerThread, SIGNAL(stereoStatusChanged(bool)), this, SLOT(handleStereoStatus(bool)));
        connect(m_workerThread, SIGNAL(tuneSuccessful()), this, SLOT(handleTuneSuccesful()));
        connect(m_workerThread, SIGNAL(seekComplete()), this, SLOT(handleSeekComplete()));
        connect(m_workerThread, SIGNAL(frequencyChanged()), this, SLOT(handleFrequencyChanged()));
        connect(m_workerThread, SIGNAL(stationsFound(const QList<int> &)),
                this, SLOT(handleStationsFound(const QList<int> &)));
        connect(m_workerThread, SIGNAL(radioTextChanged(const QString &)),
                this, SLOT(handleRadioTextChanged(const QString &)));
        connect(m_workerThread, SIGNAL(psChanged(QRadioData::ProgramType, const QString &, const QString &)),
                this, SLOT(handlePsChanged(QRadioData::ProgramType, const QString &, const QString &)));
        m_workerThread->start();

        SetCtrl(V4L2_CID_PRIVATE_IRIS_STATE, 1);
        SetCtrl(V4L2_CID_PRIVATE_IRIS_EMPHASIS, 0);
        SetCtrl(V4L2_CID_PRIVATE_IRIS_SPACING, 2);
        SetCtrl(V4L2_CID_PRIVATE_IRIS_RDS_STD, 1);
        GetTuner();
        GetFreq();
        SetCtrl(V4L2_CID_PRIVATE_IRIS_RDSON, 1);
        SetCtrl(V4L2_CID_PRIVATE_IRIS_REGION, IRIS_REGION_EU);
        SetCtrl(V4L2_CID_PRIVATE_IRIS_RDSGROUP_PROC, 255);//120
        SetCtrl(V4L2_CID_PRIVATE_IRIS_PSALL, 0);
        SetCtrl(V4L2_CID_PRIVATE_IRIS_ANTENNA, 0);
        SetFreq(m_currentFreq);
        return true;
    }

    return false;
}

bool FMRadioIrisControl::isTunerAvailable() const
{
    return m_tunerAvailable;
}

QRadioTuner::State FMRadioIrisControl::tunerState() const
{
    return m_tunerAvailable ? QRadioTuner::ActiveState : QRadioTuner::StoppedState;
}

QRadioTuner::Band FMRadioIrisControl::band() const
{
    return m_currentBand;
}

void FMRadioIrisControl::setBand(QRadioTuner::Band b)
{
    if (m_freqMin <= 87500000 && m_freqMax >= 108000000 && b == QRadioTuner::FM) {
        // FM 87.5 to 108.0 MHz, except Japan 76-90 MHz
        m_currentBand = (QRadioTuner::Band)b;
        emit bandChanged(m_currentBand);
    }
}

bool FMRadioIrisControl::isBandSupported(QRadioTuner::Band b) const
{
    QRadioTuner::Band bnd = (QRadioTuner::Band)b;
    switch (bnd) {
        case QRadioTuner::FM:
            return true;
            break;
        default:
            return false;
    }
    return false;
}

int FMRadioIrisControl::frequency() const
{
    return m_currentFreq;
}

void FMRadioIrisControl::setFrequency(int frequency)
{
    qint64 f = frequency;
    v4l2_frequency freq;

    if (frequency < m_freqMin)
        f = m_freqMax;
    if (frequency > m_freqMax)
        f = m_freqMin;

    if (m_fd > 0) {
        memset(&freq, 0, sizeof(freq));
        freq.tuner = 0;
        if (ioctl(m_fd, VIDIOC_G_FREQUENCY, &freq) >= 0) {
            if (m_low) {
                // For low, freq in units of 62.5Hz, so convert from Hz to units.
                freq.frequency = (int)(f/62.5);
            } else {
                // For high, freq in units of 62.5kHz, so convert from Hz to units.
                freq.frequency = (int)(f/62500);
            }
            ioctl(m_fd, VIDIOC_S_FREQUENCY, &freq);
            m_currentFreq = f;
            emit frequencyChanged(m_currentFreq);
            m_radioText.clear();
            emit radioTextChanged(m_radioText);
            m_stationName.clear();
            emit stationNameChanged(m_stationName);
            m_programType = QRadioData::Undefined;
            emit programTypeChanged(m_programType);
            m_programTypeName.clear();
            emit programTypeNameChanged(m_programTypeName);
            m_stationId.clear();
            emit stationIdChanged(m_stationId);
        }
    }
}

bool FMRadioIrisControl::isStereo() const
{
    return m_stereo;
}

QRadioTuner::StereoMode FMRadioIrisControl::stereoMode() const
{
    return QRadioTuner::Auto;
}

void FMRadioIrisControl::setStereoMode(QRadioTuner::StereoMode mode)
{
    bool stereo = true;
    v4l2_tuner tuner;
    memset(&tuner, 0, sizeof(tuner));

    if (mode == QRadioTuner::ForceMono)
        stereo = false;

    if (ioctl(m_fd, VIDIOC_G_TUNER, &tuner) >= 0) {
        if (stereo)
            tuner.audmode = V4L2_TUNER_MODE_STEREO;
        else
            tuner.audmode = V4L2_TUNER_MODE_MONO;

        ioctl(m_fd, VIDIOC_S_TUNER, &tuner);
    }
}

int FMRadioIrisControl::signalStrength() const
{
    v4l2_tuner tuner;
    memset(&tuner, 0, sizeof(tuner));
    tuner.index = 0;
    if (ioctl(m_fd, VIDIOC_G_TUNER, &tuner) >= 0) {
        return 0;
        if (tuner.type != V4L2_TUNER_RADIO)
            return 0;
        // percentage signal strength, FIXME
        return tuner.signal*100/255;
    }
    return 0;
}

int FMRadioIrisControl::volume() const
{
    return 0;
}

void FMRadioIrisControl::setVolume(int)
{
}

bool FMRadioIrisControl::isMuted() const
{
    return m_muted;
}

void FMRadioIrisControl::setMuted(bool muted)
{
    v4l2_queryctrl queryctrl;
    if (m_fd > 0) {
        memset(&queryctrl, 0, sizeof(queryctrl));
        queryctrl.id = V4L2_CID_AUDIO_MUTE;
        if (ioctl(m_fd, VIDIOC_QUERYCTRL, &queryctrl) >= 0) {
            v4l2_control control;
            memset(&control, 0, sizeof(control));
            control.id = V4L2_CID_AUDIO_MUTE;
            control.value = (muted ? queryctrl.maximum : queryctrl.minimum);
            ioctl(m_fd, VIDIOC_S_CTRL, &control);
            this->m_muted = muted;
            emit mutedChanged(this->m_muted);
        }
    }
}

bool FMRadioIrisControl::isSearching() const
{
    return m_scanning;
}

void FMRadioIrisControl::doSeek(int dir)
{
    struct v4l2_hw_freq_seek seek;
    memset(&seek, 0, sizeof(seek));
    seek.tuner = 0;
    seek.type = V4L2_TUNER_RADIO;
    seek.seek_upward = dir;
    seek.wrap_around = 1;
    if (0 == ioctl(m_fd, VIDIOC_S_HW_FREQ_SEEK, &seek)) {
        m_scanning = true;
        emit searchingChanged(m_scanning);
    }
    else {
        m_scanning = false;
        emit searchingChanged(m_scanning);
        qCritical("Seek failed");
    }
}

void FMRadioIrisControl::search()
{
    int signal = signalStrength();

    if (m_signalStrength != signal) {
        m_signalStrength = signal;
        emit signalStrengthChanged(m_signalStrength);
    }

    m_radioText.clear();
    emit radioTextChanged(m_radioText);
    m_stationName.clear();
    emit stationNameChanged(m_stationName);
    m_programType = QRadioData::Undefined;
    emit programTypeChanged(m_programType);
    m_programTypeName.clear();
    emit programTypeNameChanged(m_programTypeName);
    m_stationId.clear();
    emit stationIdChanged(m_stationId);
    SetCtrl(V4L2_CID_PRIVATE_IRIS_SCANDWELL, 7);
    if (m_searchMode == QRadioTuner::SearchGetStationId) {
        SetCtrl(V4L2_CID_PRIVATE_IRIS_SRCHMODE, SEEK);
    }
    doSeek(m_forward ? 1 : 0);
}

void FMRadioIrisControl::handleTunerAvailable(bool available)
{
    m_tunerAvailable = available;
}

void FMRadioIrisControl::handleRdsAvailable(bool available)
{
    m_rdsAvailable = available;
    if (m_rdsAvailable && m_searchMode == QRadioTuner::SearchGetStationId) {
        m_timer->start();
    }
}

void FMRadioIrisControl::handleStereoStatus(bool stereo)
{
    if (stereo != m_stereo) {
        m_stereo = stereo;
        emit stereoStatusChanged(m_stereo);
    }
}

void FMRadioIrisControl::handleTuneSuccesful()
{
    GetFreq();
    emit frequencyChanged(m_currentFreq);
    int signal = signalStrength();
    if (m_signalStrength != signal) {
        m_signalStrength = signal;
        emit signalStrengthChanged(m_signalStrength);
    }
    if (m_searchMode == QRadioTuner::SearchGetStationId) {
        if (m_searchPreviousFreq <= m_currentFreq) {
            m_searchPreviousFreq = m_currentFreq;
            m_timer->start();
        } else {
            cancelSearch();
            setFrequency(m_searchStartFreq);
        }
    }
}

void FMRadioIrisControl::handleSeekComplete()
{
    m_scanning = false;
    emit searchingChanged(m_scanning);
}

void FMRadioIrisControl::handleFrequencyChanged()
{
    GetFreq();
    emit frequencyChanged(m_currentFreq);
}

void FMRadioIrisControl::handleStationsFound(const QList<int> &frequencies)
{
    foreach (int frequency, frequencies) {
        emit stationFound(frequency + m_freqMin, QString());
    }

    cancelSearch();
    setFrequency(m_searchStartFreq);
}

void FMRadioIrisControl::handleRadioTextChanged(const QString &text)
{
    if (m_radioText != text) {
        m_radioText = text;
        emit radioTextChanged(m_radioText);
    }
}

void FMRadioIrisControl::handlePsChanged(QRadioData::ProgramType type, const QString &stationId, const QString &stationName)
{
    if (type != m_programType) {
        m_programType = type;
        m_programTypeName = programTypeNameString(0, m_programType);
        emit programTypeChanged(m_programType);
        emit programTypeNameChanged(m_programTypeName);
    }

    if (stationId != m_stationId) {
        m_stationId = stationId;
        emit stationIdChanged(m_stationId);
    }

    if (stationName != m_stationName) {
        m_stationName = stationName;
        emit stationNameChanged(m_stationName);
    }

    if (m_searchMode == QRadioTuner::SearchGetStationId) {
        emit stationFound(m_currentFreq, m_stationId);
        search();
    }
}

bool FMRadioIrisControl::isAntennaConnected() const
{
    return true;
}

void FMRadioIrisControl::searchForward()
{
    if (m_scanning) {
        cancelSearch();
        return;
    }
    m_forward = true;
    SetCtrl(V4L2_CID_PRIVATE_IRIS_SRCHMODE, SEEK);
    SetCtrl(V4L2_CID_PRIVATE_IRIS_SRCH_CNT, 0);
    SetCtrl(V4L2_CID_PRIVATE_IRIS_SRCH_PTY, 0);
    search();
}

void FMRadioIrisControl::searchBackward()
{
    if (m_scanning) {
        cancelSearch();
        return;
    }
    m_forward = false;
    SetCtrl(V4L2_CID_PRIVATE_IRIS_SRCHMODE, SEEK);
    SetCtrl(V4L2_CID_PRIVATE_IRIS_SRCH_CNT, 0);
    SetCtrl(V4L2_CID_PRIVATE_IRIS_SRCH_PTY, 0);
    search();
}

void FMRadioIrisControl::searchAllStations(QRadioTuner::SearchMode searchMode)
{
    this->m_searchMode = searchMode;
    if (searchMode == QRadioTuner::SearchFast) {
        SetCtrl(V4L2_CID_PRIVATE_IRIS_SRCHMODE, SCAN_FOR_STRONG);
        SetCtrl(V4L2_CID_PRIVATE_IRIS_SRCH_CNT, 12);
        SetCtrl(V4L2_CID_PRIVATE_IRIS_SRCH_PTY, 1);
        m_searchStartFreq = m_currentFreq;
        search();
    }
    else {
        m_forward = true;
        m_searchStartFreq = m_currentFreq;
        setFrequency(m_freqMin);
        m_searchPreviousFreq = m_freqMin;
        search();
    }
}

void FMRadioIrisControl::cancelSearch()
{
    m_searchMode = QRadioTuner::SearchFast;
    m_timer->stop();
    SetCtrl(V4L2_CID_PRIVATE_IRIS_SRCHON, 0);
    m_scanning = false;
    emit searchingChanged(m_scanning);
}

void FMRadioIrisControl::start()
{
}

void FMRadioIrisControl::stop()
{
}

QRadioTuner::Error FMRadioIrisControl::tunerError() const
{
    if (m_rdsError)
        return QRadioTuner::OpenError;

    return QRadioTuner::NoError;
}

QString FMRadioIrisControl::tunerErrorString() const
{
    return QString();
}

bool FMRadioIrisControl::isRdsAvailable() const
{
    return m_rdsAvailable;
}

QMultimedia::AvailabilityStatus FMRadioIrisControl::rdsAvailability() const
{
    if (m_tunerAvailable)
        return QMultimedia::Available;
    else
        return QMultimedia::ResourceError;
}

QString FMRadioIrisControl::stationId() const
{
    return m_stationId;
}

QRadioData::ProgramType FMRadioIrisControl::programType() const
{
    return m_programType;
}

QString FMRadioIrisControl::programTypeName() const
{
    return m_programTypeName;
}

QString FMRadioIrisControl::stationName() const
{
    return m_stationName;
}

QString FMRadioIrisControl::radioText() const
{
    return m_radioText;
}

void FMRadioIrisControl::setAlternativeFrequenciesEnabled(bool)
{
}

bool FMRadioIrisControl::isAlternativeFrequenciesEnabled() const
{
    return true;
}

QRadioData::Error FMRadioIrisControl::rdsError() const
{
    if (m_rdsError)
        return QRadioData::OpenError;

    return QRadioData::NoError;
}

QString FMRadioIrisControl::rdsErrorString() const
{
    return QString();
}

void FMRadioIrisControl::GetCaps(void)
{
    struct v4l2_capability caps;
    if (ioctl(m_fd, VIDIOC_QUERYCAP, &caps) < 0) {
        qCritical("Failed to query capabilities (id: %i)", errno);
    }
}

unsigned int FMRadioIrisControl::programTypeValue(int rdsStandard, unsigned int type)
{
    unsigned int rbdsTypes[] = {
        0,
        1,
        3,
        4,
        32,
        11,
        33,
        34,
        35,
        36,
        25,
        27,
        37,
        38,
        24,
        39,
        40,
        41,
        42,
        43,
        44,
        45,
        46,
        47,
        0,
        0,
        0,
        0,
        0,
        16,
        30,
        31
      };

    if (rdsStandard == 0)
        return type;
    else
        return rbdsTypes[type];
}

QString FMRadioIrisControl::programTypeNameString(int rdsStandard, unsigned int type)
{
    static const char *rbdsTypes[] = {
        QT_TR_NOOP("No program type or undefined"),
        QT_TR_NOOP("News"),
        QT_TR_NOOP("Information"),
        QT_TR_NOOP("Sports"),
        QT_TR_NOOP("Talk"),
        QT_TR_NOOP("Rock"),
        QT_TR_NOOP("Classic rock"),
        QT_TR_NOOP("Adult hits"),
        QT_TR_NOOP("Soft rock"),
        QT_TR_NOOP("Top 40"),
        QT_TR_NOOP("Country"),
        QT_TR_NOOP("Oldies"),
        QT_TR_NOOP("Soft"),
        QT_TR_NOOP("Nostalgia"),
        QT_TR_NOOP("Jazz"),
        QT_TR_NOOP("Classical"),
        QT_TR_NOOP("Rhythm and blues"),
        QT_TR_NOOP("Soft rhythm and blues"),
        QT_TR_NOOP("Language"),
        QT_TR_NOOP("Religious music"),
        QT_TR_NOOP("Religious talk"),
        QT_TR_NOOP("Personality"),
        QT_TR_NOOP("Public"),
        QT_TR_NOOP("College"),
        QT_TR_NOOP("Spanish Talk"),
        QT_TR_NOOP("Spanish Music"),
        QT_TR_NOOP("Hip Hop"),
        QT_TR_NOOP("Unassigned"),
        QT_TR_NOOP("Unassigned"),
        QT_TR_NOOP("Weather"),
        QT_TR_NOOP("Emergency test"),
        QT_TR_NOOP("Emergency")
      };

    static const char *rdsTypes[] = {
        QT_TR_NOOP("No programme type or undefined"),
        QT_TR_NOOP("News"),
        QT_TR_NOOP("Current affairs"),
        QT_TR_NOOP("Information"),
        QT_TR_NOOP("Sport"),
        QT_TR_NOOP("Education"),
        QT_TR_NOOP("Drama"),
        QT_TR_NOOP("Culture"),
        QT_TR_NOOP("Science"),
        QT_TR_NOOP("Varied"),
        QT_TR_NOOP("Pop music"),
        QT_TR_NOOP("Rock music"),
        QT_TR_NOOP("Easy listening"),
        QT_TR_NOOP("Light classical"),
        QT_TR_NOOP("Serious classical"),
        QT_TR_NOOP("Other music"),
        QT_TR_NOOP("Weather"),
        QT_TR_NOOP("Finance"),
        QT_TR_NOOP("Children’s programmes"),
        QT_TR_NOOP("Social affairs"),
        QT_TR_NOOP("Religion"),
        QT_TR_NOOP("Phone-in"),
        QT_TR_NOOP("Travel"),
        QT_TR_NOOP("Leisure"),
        QT_TR_NOOP("Jazz music"),
        QT_TR_NOOP("Country music"),
        QT_TR_NOOP("National music"),
        QT_TR_NOOP("Oldies music"),
        QT_TR_NOOP("Folk music"),
        QT_TR_NOOP("Documentary"),
        QT_TR_NOOP("Alarm test"),
        QT_TR_NOOP("Alarm")
    };

    if (rdsStandard == 0)
        return tr(rdsTypes[type]);
    else
        return tr(rbdsTypes[type]);
}

bool FMRadioIrisControl::SetTuner()
{
    struct v4l2_tuner tuner;
    memset(&tuner, 0, sizeof(tuner));
    tuner.index = 0;

    if (ioctl(m_fd, VIDIOC_S_TUNER, &tuner) >= 0) {
        return true;
    }
    else {
        qCritical("Failed to set tuner (error: %i)", errno);
        return false;
    }
}

bool FMRadioIrisControl::GetTuner(void)
{
    struct v4l2_tuner tuner;
    memset(&tuner, 0, sizeof(tuner));
    tuner.index = 0;

    if (ioctl(m_fd, VIDIOC_G_TUNER, &tuner) >= 0) {
        if ((tuner.capability & V4L2_TUNER_CAP_LOW) != 0) {
            m_low = true;
        }
        if (m_low) {
            m_freqMin = tuner.rangelow * 62.5;
            m_freqMax = tuner.rangehigh * 62.5;
        } else {
            m_freqMin = tuner.rangelow * 62500;
            m_freqMax = tuner.rangehigh * 62500;
        }
        if ((tuner.rxsubchans & V4L2_TUNER_SUB_STEREO) != 0)
            m_stereo = true;
        return true;
    }
    else {
        qCritical("Failed to get tuner (id: %i)", errno);
        return false;
    }
}

bool FMRadioIrisControl::SetCtrl(int id, int value)
{
    struct v4l2_control ctrl;
    memset(&ctrl, 0, sizeof(ctrl));
    ctrl.id = id;
    ctrl.value = value;
    if (0 == ioctl(m_fd, VIDIOC_S_CTRL, &ctrl)) {
        return true;
    } else {
        qCritical("Failed to set control (id: %i)", id);
        return false;
    }
}

int FMRadioIrisControl::GetCtrl(int id)
{
    struct v4l2_control ctrl;
    memset(&ctrl, 0, sizeof(ctrl));
    ctrl.id = id;
    if (0 == ioctl(m_fd, VIDIOC_G_CTRL, &ctrl)) {
        return ctrl.value;
    } else {
        qCritical("Failed to get control (id: %i)", id);
        return -1;
    }
}

bool FMRadioIrisControl::SetFreq(int f)
{
    struct v4l2_frequency freq;
    memset(&freq, 0, sizeof(freq));
    freq.tuner = 0;
    freq.type = V4L2_TUNER_RADIO;
    if (m_low) {
        // For low, freq in units of 62.5Hz, so convert from Hz to units.
        freq.frequency = (int)(f/62.5);
    } else {
        // For high, freq in units of 62.5kHz, so convert from Hz to units.
        freq.frequency = (int)(f/62500);
    }
    if (0 == ioctl(m_fd, VIDIOC_S_FREQUENCY, &freq)) {
        return true;
    } else {
        qCritical("Failed to set frequency");
        return false;
    }
}

int FMRadioIrisControl::GetFreq(void)
{
    struct v4l2_frequency freq;
    memset(&freq, 0, sizeof(freq));
    freq.tuner = 0;
    freq.type = V4L2_TUNER_RADIO;
    if (0 == ioctl(m_fd, VIDIOC_G_FREQUENCY, &freq)) {
        if (m_low)
            m_currentFreq = freq.frequency * 62.5;
        else
            m_currentFreq = freq.frequency * 62500;
    } else {
        qCritical("Failed to get frequency");
    }
    return m_currentFreq;
}


IrisWorkerThread::IrisWorkerThread(int fd)
    : QThread(),
      m_fd(fd)
{
}

IrisWorkerThread::~IrisWorkerThread()
{
}

void IrisWorkerThread::setQuit()
{
    m_quit.store(1);
}

void IrisWorkerThread::run()
{
    while (m_quit.loadAcquire() == 0) {
        getEvents(IRIS_BUF_EVENTS);
    }
}

void IrisWorkerThread::getEvents(int type)
{
    struct v4l2_buffer buf;
    memset(&buf, 0, sizeof(buf));
    void *mbuf = malloc(128);
    buf.index = type;
    buf.type = V4L2_BUF_TYPE_PRIVATE;
    buf.memory = V4L2_MEMORY_USERPTR;
    buf.m.userptr = (long unsigned int)mbuf;
    buf.length = 128;

    if (0 == ioctl(m_fd, VIDIOC_DQBUF, &buf)) {
        if (type == IRIS_BUF_EVENTS) {
            for (unsigned int cnt = 0; cnt < buf.bytesused; cnt++) {
                switch (((unsigned char *)mbuf)[cnt]) {
                case IRIS_EVT_RADIO_READY:
                    qDebug("Radio ready");
                    emit tunerAvailableChanged(true);
                    break;
                case IRIS_EVT_TUNE_SUCC:
                    emit tuneSuccessful();
                    break;
                case IRIS_EVT_SEEK_COMPLETE:
                    emit seekComplete();
                    break;
                case IRIS_EVT_SCAN_NEXT:
                    emit frequencyChanged();
                    break;
                case IRIS_EVT_NEW_RAW_RDS:
                    //getEvents(IRIS_BUF_RAW_RDS);
                    break;
                case IRIS_EVT_NEW_RT_RDS:
                    getEvents(IRIS_BUF_RT_RDS);
                    break;
                case IRIS_EVT_NEW_PS_RDS:
                    getEvents(IRIS_BUF_PS_RDS);
                    break;
                case IRIS_EVT_ERROR:
                    qDebug("Radio error");
                    break;
                case IRIS_EVT_BELOW_TH:
                    break;
                case IRIS_EVT_ABOVE_TH:
                    break;
                case IRIS_EVT_STEREO:
                    emit stereoStatusChanged(true);
                    break;
                case IRIS_EVT_MONO:
                    emit stereoStatusChanged(false);
                    break;
                case IRIS_EVT_RDS_AVAIL:
                    emit rdsAvailableChanged(true);
                    break;
                case IRIS_EVT_RDS_NOT_AVAIL:
                    emit rdsAvailableChanged(false);
                    break;
                case IRIS_EVT_NEW_SRCH_LIST:
                    getEvents(IRIS_BUF_SRCH_LIST);
                    break;
                case IRIS_EVT_NEW_AF_LIST:
                    break;
                case IRIS_EVT_TXRDSDAT:
                    break;
                case IRIS_EVT_TXRDSDONE:
                    qDebug("IRIS_EVT_TXRDSDONE");
                    break;
                case IRIS_EVT_RADIO_DISABLED:
                    qDebug("Radio disabled");
                    emit tunerAvailableChanged(false);
                    break;
                case IRIS_EVT_NEW_ODA:
                    break;
                case IRIS_EVT_NEW_RT_PLUS:
                    break;
                case IRIS_EVT_NEW_ERT:
                    break;
                default:
                    qCritical("Unknown event");
                }
            }
        } else {
            switch (type) {
            case IRIS_BUF_SRCH_LIST:
            {
                qDebug("Search list ready");
                int cnt;
                int stn_num;
                int len;
                unsigned char *data = (unsigned char *)mbuf;
                int num_stations_found = data[0];
                len = num_stations_found * 2 + 1;
                QList<int> frequencies;

                for (cnt = 1, stn_num = 0;
                     (cnt < len) && (stn_num < num_stations_found)
                     && (stn_num < num_stations_found);
                     cnt += 2, stn_num++) {
                    frequencies << (data[cnt+1] | (data[cnt] << 8)) * 50000;
                }
                emit stationsFound(frequencies);
            }
                break;
            case IRIS_BUF_RT_RDS:
            {
                QString radioText;
                radioText = QString::fromLatin1((char *)&((unsigned char *)mbuf)[5], (int)((unsigned char *)mbuf)[0]);
                radioText = radioText.trimmed();
                cconvert(&radioText);
                emit radioTextChanged(radioText);
            }
                break;
            case IRIS_BUF_PS_RDS:
            {
                QRadioData::ProgramType programType = (QRadioData::ProgramType)(int)((unsigned char *)mbuf)[1];
                QString stationId = QString("%1%2")
                        .arg((unsigned int)(((unsigned char *)mbuf)[2]), 2, 16, QChar('0'))
                        .arg((unsigned int)(((unsigned char *)mbuf)[3]), 2, 16, QChar('0'));
                QString stationName = QString::fromLatin1((char *)&((unsigned char *)mbuf)[5], 8);
                cconvert(&stationName);
                emit psChanged(programType, stationId, stationName);
            }
                break;
            case IRIS_BUF_RAW_RDS:
                break;
            case IRIS_BUF_AF_LIST:
                break;
            case IRIS_BUF_PEEK:
                break;
            case IRIS_BUF_SSBI_PEEK:
                break;
            case IRIS_BUF_RDS_CNTRS:
                break;
            case IRIS_BUF_RD_DEFAULT:
                break;
            case IRIS_BUF_CAL_DATA:
                break;
            case IRIS_BUF_RT_PLUS:
                break;
            case IRIS_BUF_ERT:
                break;
            default:
                qCritical("Unknown event");
            }
        }
    } else {
        qCritical("Failed to get buffer (error: %i)", errno);
    }

    free(mbuf);
}

