////////////////////////////////////////////////////////////////////
//  
// 
//     This source file is part of Virtual Choreographer
//     (3D interactive multimedia scene rendering)
//     For the latest info, see http://virchor.sourceforge.net/
//     
//     Copyright © 2002-2005 CNRS-LIMSI and Université Paris 11
//     Also see acknowledgements in Readme.html
// 
//     File vc-film.h
// 
//
//
// This program is free software; you can redistribute it and/or
// modify it under the terms of the GNU Lesser General Public License
// as published by the Free Software Foundation; either version 2.1
// of the License, or (at your option) any later version.
// 
// This program is distributed in the hope that it will be useful,
// but WITHOUT ANY WARRANTY; without even the implied warranty of
// MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU
// Lesser General Public License for more details.
// 
// You should have received a copy of the GNU Lesser General Public
// License along with this program; if not, write to the Free
// Software Foundation, Inc., 59 Temple Place - Suite 330, Boston, MA
// 02111-1307, USA.
////////////////////////////////////////////////////////////////////
#ifndef VC_FILM_H
#define VC_FILM_H

#include <stdio.h>
#include <stdlib.h>
#include <pthread.h>
#include <string.h>
#include <unistd.h> 
#include <math.h>


#include <GL/gl.h>
#include <GL/glu.h>
#include <GL/glut.h>


// ffmpeg includes
extern "C"
{
#include "avcodec.h"
#include "avformat.h"
}

enum SoundType{ SOUNDFINITE = 0 , SOUNDLOOP , EmptySoundType};
enum VideoType{ VIDEOFINITE = 0 , VIDEOLOOP , EmptyVideoType};
enum TagType{ OpeningTag = 0, ClosingTag , EmptyTag };

extern float CurrentClockTime;

#define VC_AUDIO_INPUT_BUF_SIZE 128
#define VC_AUDIO_OUTPUT_BUF_SIZE 10000
#define VC_VIDEO_PACKET_QUEUE_SIZE 8
#define VC_OUTSTREAM_FRAME_RATE 25 /* 25 images/s */

enum ReadNextFrameStatus{ VC_READ_FRAME_ERROR = 0 , VC_GOT_VIDEO_FRAME , VC_GOT_NO_VIDEO_FRAME };

// Packet queue for audio and video packets

struct vc_AudioVideoPacketQueue
{
  AVPacketList              *head;
  AVPacketList              *tail;
  int                        nbPackets;

  void            flushQueue( pthread_mutex_t queueMutex );
};

// stores RGB image
struct vc_ImageRRGB
{
  int                        bytesPerPixel;
  int                        sizeX;
  int                        sizeY;
  int                        sizeZ;
  
  AVPicture                 *avPicture;
};

// stores film parameters
struct vc_VideoParameters
{
  int                        sizeX;
  int                        sizeY;
  double                     frameDuration;
  double                     filmDuration;
};

// stores film parameters
struct vc_AudioParameters
{
  int                        audioSampleRate;
  int                        audioChannelCount;
  int                        audioSampleSize;
  int                        audioSamplesPerFrame;
};

void greenDot( uint8_t *pixMap , int x , int y , int width , int size );

class vc_AudioVideoOutData;

// Class that stores data for films with or without audio

class vc_AudioVideoData {
 public:
  // VIDEO
  AVFormatContext             *videoFormatContext;
  AVCodecContext              *videoCodecContext;
  AVCodec                     *videoCodec;
  // temporary storage of a video frame
  // (used to store the yuv image before its conversion into rgb)
  AVFrame                     *videoFrame;
  int                          videoStreamIndex;
  AVPacket                     moviePacket;

  // boolean for live video
  bool                         liveVideo;

  // current rgb image
  vc_ImageRRGB                *rgbFrame;
  int                          NPOTSizeX;
  int                          NPOTSizeY;

  
  /// notices whether a new frame is loaded in order to load
  /// the corresponding texture in the graphic card
  bool                         newFrame;

  /// if the video is not rewindable, does not try to loop
  bool                         rewindable;


  vc_AudioVideoPacketQueue    *videoQueue;

  vc_VideoParameters          *videoParam;

  int                          indVideoStream;

  // AUDIO
  AVCodecContext              *audioCodecContext;
  AVCodec                     *audioCodec;
  AVFrame                     *audioFrame;
  int                          audioStreamIndex;

  vc_AudioVideoPacketQueue    *audioQueue;

  vc_AudioParameters          *audioParam;

  int                          indAudioStream;

  // temporary storage of audio data before 
  // before streaming
  unsigned char                audioBuffer[VC_AUDIO_INPUT_BUF_SIZE * 8];
  int                          audioBufferSize;

  vc_AudioVideoData();

  // the video thread
  pthread_t                    videoThread;
  pthread_mutex_t              videoMutex;

  // the audio thread
  pthread_t                    audioThread;
  pthread_mutex_t              audioMutex;

  // queue mutual exclusive access
  pthread_mutex_t              queueMutex;

  // player management
  unsigned char                *outputBuffer;
  double                       currentFrameTime;
  double                       totalFileTime;
  double                       lastAbsoluteTime;
  int                          frameNo;

  /// stores the last OpenGL frame number
  /// where this video has been updated so that not to
  /// update more thant once per OpenGL frame
  int                          lastOpenGLFrameUpdate;

  void                     img_convert_and_resize( void );
  ReadNextFrameStatus      readNextFrame( void );
  bool                     loadFilm( char *fname );
  void                     queuePushPacket( vc_AudioVideoPacketQueue *queue,
					    AVPacket *packet );
  bool                     queuePullPacket( vc_AudioVideoPacketQueue *queue,
					    AVPacket *packet );
  bool start_video( void );
  bool rewind_video( void );
  bool GoToNextFrame( void );
};

////////////////////////////////////////////////////
// VIDEO
////////////////////////////////////////////////////

class  vc_Video {
 private:
  char                       *id;
 public:
  char                       *fileName;

  // working variables
  int                        videoOn;
  int                        nbPeriods;
  float                      timeInPeriod;
  bool                       sourceInitialized;
  bool                       sourceOn;

  // specific attributes for repeated videos
  float                      begin;
  float                      end;
  float                      period;
  float                      duration;

  VideoType                  type;

  vc_AudioVideoData         *filmAudioVideoIn;
  vc_AudioVideoOutData      *filmAudioVideoOut;

  vc_Video( void );
  void update( void );
  char *GetId( void );
  void SetId( char *newId );
  
  /* void getParameters( char *attribute ,  */
/* 		      DataType * dataType , */
/* 		      double * valDouble , */
/* 		      int * valInt , */
/* 		      bool * valBool , */
/* 		      char ** valString , */
/* 		      int * tableOrMatrixSize , */
/* 		      vc_ValScalar ** valScalars ); */
  // void parse( FILE *fileScene , int *p_c );
  void print( FILE *file , int isLong , int depth );
  bool start_video( int is_loop );
  void stop_video( void );
  bool rewind_video( void );
  void displayVideo( int textureNo );
  void LoadVideoFrameInGPUMemory( int textureNo );

  static void *fillVideoBuffer(void *readerObject);
  static void *fillAudioBuffer(void *readerObject);
  
  void operator=(vc_Video&);
};
////////////////////////////////////////////////////
// VIDEO STREAM OUTPUT
////////////////////////////////////////////////////

// stores data for creating films with or without audio

class vc_AudioVideoOutData {
 public:
  // AUDIO-VIDEO FORMAT
  AVFormatContext             *videoFormatContext;

  // VIDEO
  AVStream                    *videoStream;
  AVCodecContext              *videoCodecContext;
  AVCodec                     *videoCodec;
  // temporary storage of a video frame
  // (used to store the yuv image before its conversion into rgb)
  AVFrame                     *videoFrame;
  //AVFrame                     *aux_videoFrame;
  AVPacket                     moviePacket;
  
  uint8_t                     *videoBuffer;
  int                          videoBuffer_size;

  // current rgb image
  vc_ImageRRGB                *rgbFrame;
  
  vc_VideoParameters          *videoParam;

  // AUDIO
  AVCodecContext              *audioCodecContext;
  AVStream                    *audioStream;
  AVCodec                     *audioCodec;
  AVPacket                     audioPacket;
  
  // temporary storage of audio data before 
  // before streaming
  uint8_t                     *audioBuffer;
  
  // player management
  int                          frameNo;

  vc_AudioVideoOutData();
  void add_audio_stream(int codec_id);
  void open_audio( void );
  void write_audio_frame( void );
  void close_audio( void );
  void add_video_stream( int codec_id ,
			 int videoWidth ,
			 int videoHeight );
  AVFrame *alloc_videoFrame( int pix_fmt, int width, int height );
  void open_video( void );
  void write_video_frame( void );
  void close_video( void );
  void close_audio_video_out( bool withAudio , 
			      bool withVideo );
  void write_frame( bool withAudio , 
		    bool withVideo );

  /// video frame by frame openGL display
  void stop_video( void );
  void displayVideo( int textureNo );


  /*!
   * \brief binds a frame to a texture (adn transfers the bitmap to the GPU)
   * \param textureNo						to be added
   * \param isVertexTexture			to be added
   */
  void LoadVideoFrameInGPUMemory( int textureNo );

};

vc_AudioVideoOutData *open_audio_video_out( char *filename ,
					    bool withAudio , 
					    bool withVideo ,
					    int videoWidth ,
					    int videoHeight );

extern int 				*deadpixelflags;
extern int 				*newpixelflags;
extern int 				iNumDeadpixels;
extern int 				iNumNewpixels;
extern int 				*pixelLookupX;
//stores the lookup table.for coordinate x from pixelindex .
extern int 				*pixelLookupY;

extern int extern_close_input_file( char *filmName );

void ReportError( char *errorString );
double RealTime( void );

extern char ErrorStr[ 1024 ];
extern double RealTime( void );
extern void ReportError( char *errorString );
extern void *fillVideoBuffer(void *film);
extern void *fillAudioBuffer(void *film);

#endif
