#include <cstdint>
#include <iostream>
extern "C"
{
#include "libavcodec/avcodec.h"
#include "libavformat/avformat.h"
#include "libavutil/avutil.h"
#include "libavutil/imgutils.h"
#include "libswscale/swscale.h"
}
#include "VideoDecoder.h"


static std::string error_to_string(int error)
{
    std::string errorMessage;
    errorMessage.resize(256);
    
    ::av_strerror(error,(char *) errorMessage.data(),errorMessage.size());
    
    return errorMessage;
}


struct VideoDecoder::PrivateImpl
{
    AVFormatContext *formatContext;
    
    int streamIndex;
    AVCodecContext *codecContext;
    
    AVFrame *frame;
    AVPacket *packet;
    
    uint8_t *videoData[4];
    int videoLineSize[4];
    int videoBufferSize;
    
    SwsContext *scaleContext;
};


VideoDecoder::VideoDecoder() :
    m_impl(new PrivateImpl)
{
    if(m_impl)
    {
        m_impl->formatContext = nullptr;
        
        m_impl->streamIndex = -1;
        m_impl->codecContext = nullptr;
        
        m_impl->scaleContext = nullptr;
        
        m_impl->frame = nullptr;
        m_impl->packet = nullptr;
        
        std::fill(std::begin(m_impl->videoData),std::end(m_impl->videoData),nullptr);
        std::fill(std::begin(m_impl->videoLineSize),std::end(m_impl->videoLineSize),0);
        m_impl->videoBufferSize = 0;
    }
    
    
    setLoggingLevel(AV_LOG_FATAL);
}


VideoDecoder::~VideoDecoder()
{
    if(m_impl)
    {
        close();
        delete m_impl;
    }
}


bool VideoDecoder::valid() const
{
    if(!m_impl)
        return false;
    
    return m_impl->formatContext  &&  m_impl->streamIndex > -1  &&  m_impl->codecContext  &&
           m_impl->scaleContext  &&  m_impl->frame  &&  m_impl->packet  &&  m_impl->videoData[0];
}


bool VideoDecoder::open(const std::string& filename)
{
    if(!m_impl)
        return false;
    
    close();
    
    
    int result = ::avformat_open_input(&m_impl->formatContext,filename.c_str(),nullptr,nullptr);
    
    if(result < 0)
    {
        std::cerr << "VideoDecoder::open:  error opening input source:  " << error_to_string(result) << std::endl;
        return false;
    }
    
    
    result = ::avformat_find_stream_info(m_impl->formatContext,nullptr);
    
    if(result < 0)
    {
        std::cerr << "VideoDecoder::open:  error finding stream info:  " << error_to_string(result) << std::endl;
        return false;
    }
    
    
    result = ::av_find_best_stream(m_impl->formatContext,AVMEDIA_TYPE_VIDEO,-1,-1,nullptr,0);
    
    if(result < 0)
    {
        std::cerr << "VideoDecoder::open:  error finding best video stream:  " << error_to_string(result) << std::endl;
        return false;
    }
    
    m_impl->streamIndex = result;
    
    
    AVStream *stream = m_impl->formatContext->streams[m_impl->streamIndex];
    
    AVCodec *decoder = ::avcodec_find_decoder(stream->codecpar->codec_id);
    
    if(!decoder)
    {
        std::cerr << "VideoDecoder::open:  error finding codec" << std::endl;
        return false;
    }
    
    
    m_impl->codecContext = ::avcodec_alloc_context3(decoder);
    
    if(!m_impl->codecContext)
    {
        std::cerr << "VideoDecoder::open:  error allocating decoder context" << std::endl;
        return false;
    }
    
    
    result = ::avcodec_parameters_to_context(m_impl->codecContext,stream->codecpar);
    
    if(result < 0)
    {
        std::cerr << "VideoDecoder::open:  error copying parameters from stream to decoder context:  " << error_to_string(result) << std::endl;
        return false;
    }
    
    
    AVDictionary *options = nullptr;
    
    result = ::avcodec_open2(m_impl->codecContext,decoder,&options);
    
    if(result < 0)
    {
        std::cerr << "VideoDecoder::open:  error opening decoder:  " << error_to_string(result) << std::endl;
        return false;
    }
    
    
    m_impl->scaleContext = ::sws_getContext(m_impl->codecContext->width,m_impl->codecContext->height,m_impl->codecContext->pix_fmt,
                                            m_impl->codecContext->width,m_impl->codecContext->height,AV_PIX_FMT_RGBA,
                                            SWS_BICUBIC,nullptr,nullptr,nullptr);
    
    if(!m_impl->scaleContext)
    {
        std::cerr << "VideoDecoder::open:  error getting scaling context" << std::endl;
        return false;
    }
    
    
    result = ::av_image_alloc(m_impl->videoData,m_impl->videoLineSize,
                              m_impl->codecContext->width,m_impl->codecContext->height,AV_PIX_FMT_RGBA,1);
    
    if(result < 0)
    {
        std::cerr << "VideoDecoder::open:  error allocating image:  " << error_to_string(result) << std::endl;
        return false;
    }
    
    m_impl->videoBufferSize = result;
    
    
    if(loggingLevel() > 16)
    {
        ::av_dump_format(m_impl->formatContext,0,filename.c_str(),0);
    }
    
    
    m_impl->frame = ::av_frame_alloc();
    
    if(!m_impl->frame)
    {
        std::cerr << "VideoDecoder::open:  error allocating frame" << std::endl;
        return false;
    }
    
    
    m_impl->packet = ::av_packet_alloc();
    
    if(!m_impl->packet)
    {
        std::cerr << "VideoDecoder::open:  error allocating packet" << std::endl;
        return false;
    }
    
    std::cout << "decoder opened:  " << filename << std::endl;
    return true;
}


void VideoDecoder::close()
{
    if(!m_impl)
        return;
    
    
    if(m_impl->packet)
        ::av_packet_free(&m_impl->packet);
    
    if(m_impl->frame)
        ::av_frame_free(&m_impl->frame);
    
    if(m_impl->videoData)
        ::av_freep(m_impl->videoData);
    
    if(m_impl->scaleContext)
    {
        ::sws_freeContext(m_impl->scaleContext);
        m_impl->scaleContext = nullptr;
    }
    
    if(m_impl->codecContext)
        ::avcodec_free_context(&m_impl->codecContext);
    
    if(m_impl->formatContext)
        ::avformat_close_input(&m_impl->formatContext);
}


int VideoDecoder::decode(Image& image)
{
    if(!valid())
        return -1;
    
    
    int result;
    static bool framesRemaining = false;
    
    while(1)
    {
        if(!framesRemaining)
        {
            result = ::av_read_frame(m_impl->formatContext,m_impl->packet);
            
            if(result < 0)
            {
                if(result == AVERROR_EOF)
                {
                    return -1;
                }
                else
                {
                    std::cerr << "VideoDecoder::decode:  error reading frame:  " << error_to_string(result) << std::endl;
                    continue;
                }
            }
            
            
            if(m_impl->packet->stream_index != m_impl->streamIndex)
            {
                ::av_packet_unref(m_impl->packet);
                continue;
            }
            
            
            result = ::avcodec_send_packet(m_impl->codecContext,m_impl->packet);
            
            if(result < 0)
            {
                std::cerr << "VideoDecoder::decode:  error sending packet to decoder:  " << error_to_string(result) << std::endl;
            }
            
            framesRemaining = true;
        }
        
        
        result = ::avcodec_receive_frame(m_impl->codecContext,m_impl->frame);
        
        if(result < 0)
        {
            if(result != AVERROR_EOF  &&  result != AVERROR(EAGAIN))
            {
                std::cerr << "VideoDecoder::decode:  error receiving frame from decoder:  " << error_to_string(result) << std::endl;
            }
            
            framesRemaining = false;
        }
        else
        {
            result = ::sws_scale(m_impl->scaleContext,(const uint8_t **) m_impl->frame->data,m_impl->frame->linesize,
                                 0,m_impl->codecContext->height,
                                 m_impl->videoData,m_impl->videoLineSize);
            
            if(result < 0)
            {
                std::cerr << "VideoDecoder::decode:  error scaling image:  " << error_to_string(result) << std::endl;
                ::av_frame_unref(m_impl->frame);
                ::av_packet_unref(m_impl->packet);
                return 0;
            }
            
            
            image.load(m_impl->videoData[0],m_impl->codecContext->width,m_impl->codecContext->height,32);
            
            
            ::av_frame_unref(m_impl->frame);
            ::av_packet_unref(m_impl->packet);
            return 1;
        }
        
        
        ::av_packet_unref(m_impl->packet);
    }
}


int VideoDecoder::loggingLevel() const
{
    return ::av_log_get_level();
}


void VideoDecoder::setLoggingLevel(int loggingLevel)
{
    ::av_log_set_level(loggingLevel);
}
