#include <stdexcept>
#include <chrono>
#define __STDC_CONSTANT_MACROS
extern "C" {
#include <libavdevice/avdevice.h>
#include <libavfilter/avfilter.h>
#include <libavformat/avformat.h>
#include <libavutil/pixdesc.h>
#include <libavcodec/avcodec.h>
#include <libavfilter/buffersrc.h>
#include <libavfilter/buffersink.h>
}
#include "VidCapture.h"

using namespace std::literals::chrono_literals;

static const uint64_t kMicrosecondsPerSecond = 1000000LL;
static const AVRational kMicrosBase = { 1, kMicrosecondsPerSecond };

class DictHelper {
    AVDictionary *_dict;
public:
    DictHelper()
    :_dict(nullptr)
    {
    }

    ~DictHelper()
    {
        av_dict_free(&_dict);
    }

    void Set(const char* key, const char* value) 
    {
        av_dict_set(&_dict, key, value, 0);
    }

    operator AVDictionary**()
    {
        return &_dict;
    }
};

class PacketHelper {
public:
    AVPacket packet;
    PacketHelper()
    :packet{}
    {
    }
    ~PacketHelper()
    {
        av_packet_unref(&packet);
    }
};

class Source {
    AVFormatContext *_input;
    AVCodecContext *_decoder;
public:
    Source()
    :_input(avformat_alloc_context()), _decoder(nullptr)
    {
        // Create Framebuffer source
        const AVInputFormat *ifmt = av_find_input_format("fbdev");
        if (!ifmt)
            throw std::runtime_error("Failed to find framebuffer device");
        DictHelper options;
        options.Set("framerate", "30");
        if (avformat_open_input(&_input, "/dev/fb0", ifmt, options) != 0)
            throw std::runtime_error("Failed to open framebuffer");

        // Get "codec" to "decompress" the framebuffer
        AVStream *stream = _input->streams[0];
        const AVCodec *codec = avcodec_find_decoder(stream->codecpar->codec_id);
        if (!codec)
            throw std::runtime_error("Failed to find decoder for framebuffer");
        _decoder = avcodec_alloc_context3(codec);
        if (avcodec_parameters_to_context(_decoder, stream->codecpar) < 0)
            throw std::runtime_error("Failed to copy decoder parameters");
        _decoder->framerate = {30, 1};
        if (avcodec_open2(_decoder, codec, 0) < 0)
            throw std::runtime_error("Failed to open decoder");
    }
    ~Source()
    {
        avcodec_free_context(&_decoder);
        avformat_close_input(&_input);
    }
    AVCodecContext* Get() const
    {
        return _decoder;
    }
    AVFormatContext* GetInput() const
    {
        return _input;
    }
    template<typename F>
    bool ReadFrame(F callback)
    {
        PacketHelper helper;
        if (av_read_frame(_input, &helper.packet) < 0)
            return false;
        callback(helper.packet);
        return true;
    }
};

class OutputSink {
    AVCodecContext *_encoder;
public:
    OutputSink(const Source& source)
    :_encoder(nullptr)
    {
        const AVCodec *codec = avcodec_find_encoder_by_name(
#define SOFTWARE_ENCODER
#ifdef SOFTWARE_ENCODER
        "libx264"
#else
        "h264_v4l2m2m"
#endif
            );
        if (!codec)
            throw std::runtime_error("Failed to find encoder");
        _encoder = avcodec_alloc_context3(codec);
        if (!_encoder)
            throw std::runtime_error("Failed to allocate encoder");
        _encoder->width = 800;
        _encoder->height = 480;
        _encoder->sample_aspect_ratio = {4, 3};
        _encoder->pix_fmt = codec->pix_fmts ? codec->pix_fmts[0] : source.Get()->pix_fmt;
        _encoder->time_base = av_inv_q(source.Get()->framerate);
        _encoder->framerate = source.Get()->framerate;
        av_opt_set(_encoder->priv_data, "profile", "baseline", 0);
        _encoder->bit_rate = 800000;
#ifdef SOFTWARE_ENCODER
        av_opt_set(_encoder->priv_data, "x264opts", "no-mbtree:keyint=30:min-keyint=1:force-cfr=1:sync-lookahead=0", 0);
#else
        char buf[10];
        sprintf(buf, "%i", _encoder->bit_rate);
        av_opt_set(_encoder->priv_data, "b", buf, 0);
        av_opt_set(_encoder->priv_data, "rsh", "0", 0); // repeat sequence header
        av_opt_set(_encoder->priv_data, "shm", "separate_buffer", 0);
        av_opt_set(_encoder->priv_data, "g", "30", 0);
#endif
        if (avcodec_open2(_encoder, codec, NULL) < 0)
            throw std::runtime_error("Failed to open encoder");
    }
    ~OutputSink()
    {
        avcodec_free_context(&_encoder);
    }
    AVCodecContext* Get() const {
        return _encoder;
    }
};

struct InOutHelper
{
    AVFilterInOut *value;

    InOutHelper()
    :value(avfilter_inout_alloc())
    {
        if (!value)
            throw std::runtime_error("Couldn't allocate filter inouts");
        value->pad_idx = 0;
        value->next = NULL;
    }
    ~InOutHelper()
    {
        avfilter_inout_free(&value);
    }
};

class Filterer {
    AVFilterContext *_source;
    AVFilterContext *_sink;
    AVFilterGraph *_graph;
public:
    Filterer(const Source& source, const OutputSink& sink)
    :_graph(avfilter_graph_alloc()), _source(nullptr), _sink(nullptr)
    {
        const AVFilter *bufferSource = avfilter_get_by_name("buffer");
        if (!bufferSource)
            throw std::runtime_error("Couldn't find source filter");
        const AVFilter *bufferSink = avfilter_get_by_name("buffersink");
        if (!bufferSink)
            throw std::runtime_error("Couldn't find sink filter");
        auto decctx = source.Get();
        std::string args = std::string("video_size=")
            + std::to_string(decctx->width) + "x" + std::to_string(decctx->height)
            + ":pix_fmt=" + av_get_pix_fmt_name(decctx->pix_fmt)
            + ":time_base=" + std::to_string(decctx->time_base.num) + "/" + std::to_string(decctx->time_base.den)
            + ":pixel_aspect=" + std::to_string(decctx->sample_aspect_ratio.num) + "/" + std::to_string(decctx->sample_aspect_ratio.den);
        if (avfilter_graph_create_filter(&_source, bufferSource, "in", args.c_str(), NULL, _graph) < 0)
            throw std::runtime_error("Failed to create source buffer");
        if (avfilter_graph_create_filter(&_sink, bufferSink, "out", NULL, NULL, _graph) < 0)
            throw std::runtime_error("Failed to create sink buffer");
        if (av_opt_set_bin(_sink, "pix_fmts", (uint8_t*)&sink.Get()->pix_fmt, sizeof(AVPixelFormat), AV_OPT_SEARCH_CHILDREN) < 0)
            throw std::runtime_error("Could not set output pixel format");
        InOutHelper inputs, outputs;
        outputs.value->name = av_strdup("in");
        outputs.value->filter_ctx = _source;
        inputs.value->name = av_strdup("out");
        inputs.value->filter_ctx = _sink;
        char filt[50];
        auto encctx = sink.Get();
        sprintf(filt, "scale=%ix%i", encctx->width, encctx->height);
        if (avfilter_graph_parse_ptr(_graph, filt, &inputs.value, &outputs.value, NULL) < 0)
            throw std::runtime_error("Couldn't parse graph");
        if (avfilter_graph_config(_graph, NULL) < 0)
            throw std::runtime_error("Couldn't configure graph");
//        printf("%s\n", avfilter_graph_dump(_graph, NULL));
    }
    ~Filterer()
    {

    }
    AVFilterContext* GetSource() const { return _source; }
    AVFilterContext* GetSink() const { return _sink; }
};

struct FrameHelper {
    AVFrame *frame;

    FrameHelper():frame(av_frame_alloc()){}
    ~FrameHelper(){av_frame_free(&frame);}
};

struct NaughtyInit {
    NaughtyInit()
    {
//        av_log_set_level(AV_LOG_DEBUG);
        avdevice_register_all();
    }
} naughtyInstance;

VidCapture::VidCapture()
:_exit(false), _thread(&VidCapture::thread, this)
{

}

VidCapture::~VidCapture()
{
    _exit = true;
    _thread.join();
}

void VidCapture::start(std::function<void(int64_t, const uint8_t*, int)> callback)
{
    _callback = callback;
}

void VidCapture::stop()
{
    _callback = std::function<void(int64_t, const uint8_t*,int)>();
}

void VidCapture::thread()
{
    while (!_exit) {
        if (_callback) {
            Source source;
            OutputSink sink(source);
            Filterer filters(source, sink);
            while (!_exit && _callback && source.ReadFrame([&](AVPacket& packet){
                av_packet_rescale_ts(&packet, source.GetInput()->streams[0]->time_base, source.Get()->time_base);
                {
                    FrameHelper inputFrame;
                    int got_frame = 0;
                    if (avcodec_send_packet(source.Get(), &packet) < 0)
//                    if (avcodec_decode_video2(source.Get(), inputFrame.frame, &got_frame, &packet) < 0)
                        throw std::runtime_error("Failed to send frame to decoder");
                    int res = avcodec_receive_frame(source.Get(), inputFrame.frame);
                    if (res == AVERROR(EAGAIN))
                        return;
                    if (res < 0)
                        throw std::runtime_error("Failed to read raw frame from decoder");
                    if (av_buffersrc_add_frame_flags(filters.GetSource(), inputFrame.frame, 0) < 0)
                        throw std::runtime_error("Failed to feed frame to filter graph");
                }
                while (!_exit) {
                    // PacketHelper helper;
                    // int gotframe = 0;
                    {
                        FrameHelper outputFrame;
                        int res = av_buffersink_get_frame(filters.GetSink(), outputFrame.frame);
                        if (res == AVERROR(EAGAIN))
                            break;
                        if (res < 0)
                            throw std::runtime_error("Failed to read frame from filter graph");
                        // if (avcodec_encode_video2(sink.Get(), &helper.packet, outputFrame.frame, &gotframe) < 0)
                        //     throw std::runtime_error("Failed to encode video");
                        // if (!gotframe)
                        //     break;
                        res = avcodec_send_frame(sink.Get(), outputFrame.frame);
                        if (res < 0)
                            throw std::runtime_error("Failed to send frame to encoder");
                        while (true) {
                            PacketHelper helper;
                            res = avcodec_receive_packet(sink.Get(), &helper.packet);
                            if (res == AVERROR(EAGAIN))
                                break;
                            if (res < 0)
                                throw std::runtime_error("Failed to get packet from encoder");
                            if (!_callback)
                                break;
                            _callback(av_rescale_q(helper.packet.pts, sink.Get()->time_base, kMicrosBase), helper.packet.data, helper.packet.size);
                        }
                    }
                    // This is not at all safe
                    // if (!_callback)
                    //     break;
                    // printf("%i:", helper.packet.size);
                    // for (int i = 0; i < helper.packet.size; i++)
                    //     printf(" %02x", helper.packet.data[i]);
                    // printf("\n");
                    // _callback(av_rescale_q(helper.packet.pts, sink.Get()->time_base, kMicrosBase), helper.packet.data, helper.packet.size);
                }
            }));
        } else {
            std::this_thread::sleep_for(500ms);
        }
    }
}
