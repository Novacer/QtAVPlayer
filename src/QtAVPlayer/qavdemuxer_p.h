/*********************************************************
 * Copyright (C) 2020, Val Doroshchuk <valbok@gmail.com> *
 *                                                       *
 * This file is part of QtAVPlayer.                      *
 * Free Qt Media Player based on FFmpeg.                 *
 *********************************************************/

#ifndef QAVDEMUXER_H
#define QAVDEMUXER_H

//
//  W A R N I N G
//  -------------
//
// This file is not part of the Qt API. It exists purely as an
// implementation detail. This header file may change from version to
// version without notice, or even be removed.
//
// We mean it.
//

#include "qavpacket_p.h"
#include "qavstream.h"
#include "qavframe.h"
#include "qavsubtitleframe.h"
#include <QObject>
#include <QMap>
#include <memory>

QT_BEGIN_NAMESPACE

extern "C" {
#include <libavutil/avutil.h>
}

class QAVDemuxerPrivate;
class QAVVideoCodec;
class QAVAudioCodec;
class QAVIODevice;
struct AVStream;
struct AVCodecContext;
struct AVFormatContext;
class Q_AVPLAYER_EXPORT QAVDemuxer : public QObject
{
public:
    QAVDemuxer(QObject *parent = nullptr);
    ~QAVDemuxer();

    void abort(bool stop = true);
    int load(const QString &url, QAVIODevice *dev = nullptr);
    void unload();

    QAVStream stream(int index) const;
    AVMediaType currentCodecType(int index) const;

    QList<QAVStream> availableVideoStreams() const;
    QList<QAVStream> currentVideoStreams() const;
    bool setVideoStreams(const QList<QAVStream> &streams);

    QList<QAVStream> availableAudioStreams() const;
    QList<QAVStream> currentAudioStreams() const;
    bool setAudioStreams(const QList<QAVStream> &streams);

    QList<QAVStream> availableSubtitleStreams() const;
    QList<QAVStream> currentSubtitleStreams() const;
    bool setSubtitleStreams(const QList<QAVStream> &streams);

    QAVPacket read();

    bool decode(const QAVPacket &pkt, QAVFrame &frame) const;
    bool decode(const QAVPacket &pkt, QAVSubtitleFrame &frame) const;

    double duration() const;
    bool seekable() const;
    int seek(double sec);
    bool eof() const;
    double videoFrameRate() const;

    QMap<QString, QString> metadata() const;

    QString bitstreamFilter() const;
    int applyBitstreamFilter(const QString &bsfs);

    QString inputFormat() const;
    void setInputFormat(const QString &format);

    QString inputVideoCodec() const;
    void setInputVideoCodec(const QString &codec);

    QMap<QString, QString> inputOptions() const;
    void setInputOptions(const QMap<QString, QString> &opts);

    static QStringList supportedFormats();
    static QStringList supportedVideoCodecs();
    static QStringList supportedProtocols();
    static QStringList supportedBitstreamFilters();

protected:
    std::unique_ptr<QAVDemuxerPrivate> d_ptr;

private:
    Q_DISABLE_COPY(QAVDemuxer)
    Q_DECLARE_PRIVATE(QAVDemuxer)
};

QT_END_NAMESPACE

#endif
