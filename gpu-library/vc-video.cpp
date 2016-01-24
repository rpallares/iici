// Programme de test de lecture/ecriture d'un stream video
// programme inspiré (voir quasiment pompé) sur VirChor

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
//     File vc-film.cpp - from VESS http://vess.ist.ucf.edu/
//                        and ffmpeg (ffplay) http://ffmpeg.sourceforge.net/
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

#include "main.h"

char ErrorStr[ 1024 ];
char *VideoTypeString[EmptyVideoType + 1] = { "videofinite" , "videoloop" , "emptyvideotype" };


double RealTime( void ) {
  struct timeval time;
  gettimeofday(&time, 0);
  double realtime = (double)time.tv_sec + ((double)time.tv_usec / 1000000.);
  return realtime;
}

void ReportError( char *errorString ) {
  fprintf( stderr , "%s\n" , errorString );
}

void StrCpy( char **target , char *source ) {
  if( !source ) {
    return;
  }
  if( *target ) {
    delete [] *target;
  }
  int str_l = strlen( source ) + 1;
  *target = new char[ str_l ];
  memcpy( *target , source , str_l * sizeof( char ) );
}  

///////////////////////////////////////////////////////////
// PACKET QUEUES
///////////////////////////////////////////////////////////

// empties a queue

void vc_AudioVideoPacketQueue::flushQueue( pthread_mutex_t queueMutex )
{
  AVPacketList *packetQueueEntry;
  AVPacket packet;

  // Lock the queue mutex
  pthread_mutex_lock( &queueMutex );

  // Remove and free each packet in turn
  while( head && nbPackets > 0 ) {
    // Try to dequeue a packet, free its memory if successful
    packetQueueEntry = head;

    if (packetQueueEntry != NULL) {
      // Update the queue
      head = packetQueueEntry->next;
      if (head == NULL)
	tail = NULL;
      nbPackets--;

      // Extract the packet from the queue element, free the element 
      // and free the packet structure as well
      packet = packetQueueEntry->pkt;
      free(packetQueueEntry);
      av_free_packet(&packet);
    }
  }

  // Reset all queue pointers and counters
  head = tail = NULL;
  nbPackets = 0;

  // Unlock the queue mutex
  pthread_mutex_unlock(&queueMutex);
}

///////////////////////////////////////////////////////////
// FILMS
///////////////////////////////////////////////////////////

// Film constructor

vc_AudioVideoData::vc_AudioVideoData( void ) {
  // libavcodec data initialization

  ///////////////////////////////////////
  // codec registration

  // register the ffmpeg accessible codecs
  av_register_all();

  //////////////////////////////////
  // video
  videoFormatContext = NULL;
  videoCodecContext = NULL;
  videoCodec = NULL;
  videoStreamIndex = -1;

  liveVideo = false;

  // current video frame allocation
  videoFrame = avcodec_alloc_frame();

  // RGB frame
  rgbFrame = new vc_ImageRRGB;
  // allocates a frame that will hold the image converted from yuv to rgb
  // pixel buffer initialized in a second step when the image size is known
  rgbFrame->avPicture = (AVPicture *)malloc(sizeof(AVPicture));
  // resets the rgb frame
  memset(rgbFrame, 0, sizeof(rgbFrame));
  rgbFrame->bytesPerPixel = 3;
  rgbFrame->sizeX = 0;
  rgbFrame->sizeY = 0;
  rgbFrame->sizeZ = 0;
 
//   // RGB frame
//   rgbNPOTFrame = new vc_ImageRRGB;
//   // allocates a frame that will hold the image converted from yuv to rgb
//   // pixel buffer initialized in a second step when the image size is known
//   rgbNPOTFrame->avPicture = (AVPicture *)malloc(sizeof(AVPicture));
//   // resets the rgb frame
//   memset(rgbNPOTFrame, 0, sizeof(rgbNPOTFrame));
//   rgbNPOTFrame->bytesPerPixel = 3;
//   rgbNPOTFrame->sizeX = 0;
//   rgbNPOTFrame->sizeY = 0;
//   rgbNPOTFrame->sizeZ = 0;
 
  // video packet queue initialization
  videoQueue = new vc_AudioVideoPacketQueue;
  videoQueue->head = NULL;
  videoQueue->tail = NULL;
  videoQueue->nbPackets = 0;

  // video parameters initialization
  videoParam = new vc_VideoParameters;
  videoParam->sizeX = 0;
  videoParam->sizeY = 0;
  videoParam->frameDuration = 0.0;
  videoParam->filmDuration = 0.0;
  newFrame = false;
  rewindable = true;

  //////////////////////////////////
  // audio
  audioCodecContext = NULL;
  audioCodec = NULL;
  audioStreamIndex = -1;

  // current audio frame allocation
  audioFrame = avcodec_alloc_frame();

  // audio packet queue initialization
  audioQueue = new vc_AudioVideoPacketQueue;
  audioQueue->head = NULL;
  audioQueue->tail = NULL;
  audioQueue->nbPackets = 0;

  // audio parameters initialization
  audioParam = new vc_AudioParameters;
  audioParam->audioSampleRate = 0;
  audioParam->audioSampleSize = 0;
  audioParam->audioChannelCount = 0;
  audioParam->audioSamplesPerFrame = 0;

  ///////////////////////////////////////
  // thread initialization
  
  // video, audio and  and queue mutex objects
  pthread_mutex_init(&queueMutex, NULL);
  pthread_mutex_init(&videoMutex, NULL);
  pthread_mutex_init(&audioMutex, NULL);
  
  ///////////////////////////////////////
  // dates
  
  // relative time (wrt film time) of the current frame
  currentFrameTime = 0.0;
  // relative total time (wrt film time) of the current film
  totalFileTime = 0.0;
  frameNo = 0;
  // absolute time of the last frame display
  double the_time = glutGet(GLUT_ELAPSED_TIME) / 1000.0;
  lastAbsoluteTime = (double)the_time;
}

// film loader

/////////////////////////////////////////////////////////////////////
// video access: load - frame by frame access - rewind
//////////////////////////////////////////////////////////////////////

bool vc_AudioVideoData::loadFilm(char *fname) {
  int errorCode;
  int errNo = 0;
  AVFormatParameters formatParams;
  AVInputFormat *iformat=NULL;
    
  // Acquire the file mutex
  pthread_mutex_lock(&videoMutex);

  if( strcmp( fname , "/dev/video0" ) == 0
      || strcmp( fname , "/dev/video" ) == 0
      || strcmp( fname , "/dev/v4l/video" ) == 0 ) {
    formatParams.device = "/dev/video0";
    formatParams.channel = 0;
    formatParams.standard = "ntsc";
    formatParams.width = 320;
    formatParams.height = 240;
    formatParams.frame_rate = 25;
    formatParams.frame_rate_base = 1;
    iformat = av_find_input_format("video4linux");
    liveVideo = true;

    errNo = av_open_input_file(&videoFormatContext,
			       "", iformat, 0, &formatParams);
    if( errNo < 0 ) {
      fprintf( stderr , "Error: live video not opened, no such device [%s] error %d!\n" , fname , errNo ); exit(0);
    }
  }
  else if( strcmp( fname , "/dev/dv1394" ) == 0
      || strcmp( fname , "/dev/dvd" ) == 0 ) {
    formatParams.device = "/dev/dv1394";
    iformat = av_find_input_format("dv1394");
    liveVideo = true;
    
    errNo = av_open_input_file(&videoFormatContext,
			       "", iformat, 0, &formatParams);
    if( errNo < 0 ) {
      fprintf( stderr , "Error: live video not opened, no such device [%s] error %d!\n" , fname , errNo ); exit(0);
    }
  }
  else {
    // file opening
    errNo = av_open_input_file(&videoFormatContext, fname, NULL, 0, NULL);
    liveVideo = false;
    if( errNo < 0 ) {
      fprintf( stderr , "Error: video file not found [%s] error %d!\n" , fname , errNo ); exit(0);
    }
  }
  
  // codec retrieval
  errorCode = av_find_stream_info(videoFormatContext);
  if( errNo < 0 ) {
    fprintf( stderr, "Error: codec of video file not supported [%s] error %d, see http://www1.mplayerhq.hu/MPlayer/releases/codecs/!\n" , fname , errNo ); exit(0);
  }

  // default video frame rate
  videoParam->frameDuration = 1.0 / 25.0;

  // retrievaes a video stream from the file
  bool videoFound = false;
  fprintf(stderr,"nb of streams = %d\n", videoFormatContext->nb_streams);

  for( indVideoStream = 0 ; indVideoStream < videoFormatContext->nb_streams ; indVideoStream++ ) {
  
    // Get the video codec context for this stream
    videoCodecContext = &(videoFormatContext->streams[indVideoStream]->codec);

    if( videoCodecContext->codec_type == CODEC_TYPE_VIDEO ) {
      videoFound = true;
      videoStreamIndex = indVideoStream;
      break;
    }
  }

  // no video, probably a sound file
  if( !videoFound ) {
    // video variables reset
    videoCodecContext = NULL;
    videoCodec = NULL;
    indVideoStream = -1;
  }
  // there is a video stream
  else {
    // Find the appropriate video codec
    videoCodec = avcodec_find_decoder(videoCodecContext->codec_id);
    if (videoCodec == NULL) {
      fprintf( stderr , "Error: codec of video file not supported [%s] error %d, see http://www1.mplayerhq.hu/MPlayer/releases/codecs/!\n" , fname , errNo ); exit(0);
      // video variables reset
      videoCodecContext = NULL;
      videoCodec = NULL;
      indVideoStream = -1;
    }
    else {
      // Initialize the video codec
      errorCode = avcodec_open(videoCodecContext, videoCodec);
      if (errorCode < 0) {
	fprintf( stderr , "Error: codec initialization error [%s] error %d, see http://www1.mplayerhq.hu/MPlayer/releases/codecs/!\n" , fname , errNo ); exit(0);
      // video variables reset
	videoCodecContext = NULL;
	videoCodec = NULL;
	indVideoStream = -1;
      }
      else {
	// video dimensions
	videoParam->sizeX = videoCodecContext->width;
	videoParam->sizeY = videoCodecContext->height;
	
	// rgb raw image allocation (without alpha channel)
	rgbFrame->sizeX = videoCodecContext->width;
	rgbFrame->sizeY = videoCodecContext->height;
	printf( "Video %dx%d texture %dx%d\n" , videoCodecContext->width , videoCodecContext->height , rgbFrame->sizeX , rgbFrame->sizeY );
	rgbFrame->bytesPerPixel = 3;
	rgbFrame->avPicture->data[0] 
	  = new GLubyte[rgbFrame->sizeX * rgbFrame->sizeY 
			* rgbFrame->bytesPerPixel];
	memset( rgbFrame->avPicture->data[0], (GLubyte)255, 
		rgbFrame->sizeX * rgbFrame->sizeY 
		* rgbFrame->bytesPerPixel );
	rgbFrame->avPicture->data[1] = NULL;
	rgbFrame->avPicture->data[2] = NULL;
	rgbFrame->avPicture->data[3] = NULL;
	rgbFrame->avPicture->linesize[0] 
	  = rgbFrame->sizeX * rgbFrame->bytesPerPixel;
	rgbFrame->avPicture->linesize[1] = 0;
	rgbFrame->avPicture->linesize[2] = 0;
	rgbFrame->avPicture->linesize[3] = 0;
	
	// if needed - rgb NPOT raw image allocation (without alpha channel)
	if( rgbFrame->sizeX != videoCodecContext->width 
	    || rgbFrame->sizeY != videoCodecContext->height ) {
	  NPOTSizeX = videoCodecContext->width;
	  NPOTSizeY = videoCodecContext->height;
// 	  rgbNPOTFrame->sizeX = videoCodecContext->width;
// 	  rgbNPOTFrame->sizeY = videoCodecContext->height;
// 	  rgbNPOTFrame->bytesPerPixel = 3;
// 	  rgbNPOTFrame->avPicture->data[0] 
// 	    = new GLubyte[rgbNPOTFrame->sizeX * rgbNPOTFrame->sizeY 
// 			  * rgbNPOTFrame->bytesPerPixel];
// 	  memset( rgbNPOTFrame->avPicture->data[0], (GLubyte)255, 
// 		  rgbNPOTFrame->sizeX * rgbNPOTFrame->sizeY 
// 		  * rgbNPOTFrame->bytesPerPixel );
// 	  rgbNPOTFrame->avPicture->data[1] = NULL;
// 	  rgbNPOTFrame->avPicture->data[2] = NULL;
// 	  rgbNPOTFrame->avPicture->data[3] = NULL;
// 	  rgbNPOTFrame->avPicture->linesize[0] 
// 	    = rgbNPOTFrame->sizeX * rgbNPOTFrame->bytesPerPixel;
// 	  rgbNPOTFrame->avPicture->linesize[1] = 0;
// 	  rgbNPOTFrame->avPicture->linesize[2] = 0;
// 	  rgbNPOTFrame->avPicture->linesize[3] = 0;
	}
	
	// video frame duration
	if( videoCodecContext->frame_rate != 0) {
	  fprintf(stderr,"fps = %d\n", videoCodecContext->frame_rate);
	  videoParam->frameDuration = (double)videoCodecContext->frame_rate_base /
	    (double)videoCodecContext->frame_rate;
	}
      }
    }
  }

  // retrievaes an audio stream from the file
  bool audioFound = false;
  for( indAudioStream = 0 ; indAudioStream < videoFormatContext->nb_streams ; indAudioStream++ ) {
  
    // Get the audio codec context for this stream
    audioCodecContext = &(videoFormatContext->streams[indAudioStream]->codec);

    if( audioCodecContext->codec_type == CODEC_TYPE_AUDIO ) {
      audioFound = true;
      audioStreamIndex = indAudioStream;
      break;
    }
  }

  // no audio
  if( !audioFound ) {
    // audio variables reset
    audioCodecContext = NULL;
    audioCodec = NULL;
    indAudioStream = -1;
  }
  // there is a audio stream
  else {
    // Find the appropriate audio codec
    audioCodec = avcodec_find_decoder(audioCodecContext->codec_id);
    if (audioCodec == NULL) {
      fprintf( stderr , "Error: codec of audio file not supported [%s] error %d, see http://www1.mplayerhq.hu/MPlayer/releases/codecs/!\n" , fname , errNo ); exit(0);
      // audio variables reset
      audioCodecContext = NULL;
      audioCodec = NULL;
      indAudioStream = -1;
    }
    else {
      // Initialize the audio codec
      errorCode = avcodec_open(audioCodecContext, audioCodec);
      if (errorCode < 0) {
	fprintf( stderr , "Error: codec initialization error [%s] error %d, see http://www1.mplayerhq.hu/MPlayer/releases/codecs/!\n" , fname , errNo ); exit(0);
      // audio variables reset
	audioCodecContext = NULL;
	audioCodec = NULL;
	indAudioStream = -1;
      }
      else {
	// Get the audio parameters of the audio stream
	audioParam->audioSampleRate = audioCodecContext->sample_rate;
	audioParam->audioChannelCount = audioCodecContext->channels;

	// ffmpeg always decodes 16-bit audio
	audioParam->audioSampleSize = 2;

	// Compute the number of audio samples per frame of video
	audioParam->audioSamplesPerFrame = (int)( videoParam->frameDuration 
					          * audioParam->audioSampleRate )  
	                                * audioParam->audioSampleSize
	                                * audioParam->audioChannelCount;

	// Acquire the audio mutex
	pthread_mutex_lock(&audioMutex);

	// generate an audio stream

	// Release the audio mutex
	pthread_mutex_unlock(&audioMutex);
      }
    }
  }

  // Release the file mutex
  pthread_mutex_unlock(&videoMutex);

  // for the moment we are only interested in video streams
  if( videoCodecContext ) {
    // reset the video time parameters 
    currentFrameTime = 0.0;
    totalFileTime = 0.0;
    frameNo = 0;
    double the_time = glutGet(GLUT_ELAPSED_TIME) / 1000.0;
    lastAbsoluteTime = (double)the_time;
    lastOpenGLFrameUpdate = FrameNo - 1;
    newFrame = false;
    // a priori a video is rewindable, unless a rewind sets it to false
    rewindable = true; 

    // initiates the decoder by putting the first frame in the queue
    ReadNextFrameStatus readStatus = readStatus;
    if( (readStatus = readNextFrame()) != VC_READ_FRAME_ERROR ) {
      if( readStatus == VC_GOT_VIDEO_FRAME ) {
	img_convert_and_resize();
      }
      // Release the packet
      av_free_packet(&moviePacket);
    }

    // Return true to indicate that we successfully opened the file
    return true;
  }
  else {
    fprintf( stderr , "Error: file with no video or audio stream!\n" ); exit(0);
  }

  // Return false to indicate that we couldn't open the file or that we
  // have nothing to play
  return false;
}

// gets next frame from the video buffer

ReadNextFrameStatus vc_AudioVideoData::readNextFrame( void ) {
  int len;
  int gotPicture;
  // Get the next video packet from the queue
  if( videoCodecContext != NULL ) {
    // printf( "readNextFrame\n" );
    if( queuePullPacket( videoQueue , &moviePacket ) ) {
      // Allocate a video frame and decode the video packet
      /* NOTE1: some codecs are stream based (mpegvideo, mpegaudio)
	 and this is the only method to use them because you cannot
	 know the compressed data size before analysing it. 

	 BUT some other codecs (msmpeg4, mpeg4) are inherently frame
	 based, so you must call them with all the data for one
	 frame exactly. You must also initialize 'width' and
	 'height' before initializing them. */

      /* NOTE2: some codecs allow the raw parameters (frame size,
	 sample rate) to be changed at any frame. We handle this, so
	 you should also take care of it */

      len = avcodec_decode_video(videoCodecContext, videoFrame, 
				 &gotPicture, moviePacket.data, 
				 moviePacket.size);

      // incomplete frame
      if (len < 0) {
	fprintf( stderr , "Error: incomplete frame decoding!" ); 
	// Release the packet
	av_free_packet(&moviePacket);
	return VC_READ_FRAME_ERROR;
      }
      // full frame
      
      if( gotPicture ) {
	return VC_GOT_VIDEO_FRAME;
      }
      else {
	return VC_GOT_NO_VIDEO_FRAME;
      }
    }
    else {
      return VC_READ_FRAME_ERROR;
    }
  }

  return VC_READ_FRAME_ERROR;

  ////////////////////////////////////////////////////////
  // audio read
  // not implemented in the current version

//   unsigned char *audioBufferPtr;
//   int size, outputSize;
//   unsigned char *dataPtr;

//   // See if we need to decode more audio
//   while( (audioCodecContext != NULL) && 
// 	 (audioBufferSize < VC_AUDIO_BUF_SIZE * 6) &&
// 	 (audioQueue->nbPackets > 0)) {
//     // Get a packet from the audio queue
//     if (queuePullPacket(audioQueue, &moviePacket)) {
//       // Decode the packet data
//       size = moviePacket.size;
//       dataPtr = moviePacket.data;
//       audioBufferPtr = (unsigned char *)&audioBuffer[audioBufferSize];
//       while (size > 0) {
// 	// Lock the audio mutex
// 	pthread_mutex_lock(&audioMutex);

// 	// Decode a chunk of the packet's data
// 	len = avcodec_decode_audio(audioCodecContext, 
// 				   (short *)audioBufferPtr, 
// 				   &outputSize, dataPtr,
// 				   size);

// 	// error: throw away
// 	if( len < 0 || outputSize < 0 ) {
// 	  size = 0;
// 	}
// 	else {
// 	  // moves forward in the buffer and updates pointers
// 	  size -= len;
// 	  dataPtr += len;
// 	  audioBufferPtr += outputSize;
// 	  audioBufferSize += outputSize;
// 	}
	  
// 	// Unlock the audio mutex
// 	pthread_mutex_unlock(&audioMutex);
//       }
      
//       // Release the packet
//       av_free_packet(&moviePacket);
//     }
//   }

//   // If we've run out of audio data, we need to stop playing
//   if( audioQueue->nbPackets == 0 
//       && audioBufferSize < audioParam->audioSamplesPerFrame ) {
//     return false;
//   }
//   return true;
}

// converts a frame from yuv to rgb

void vc_AudioVideoData::img_convert_and_resize( void ) {
  if( rgbFrame->sizeX == videoParam->sizeX
      && rgbFrame->sizeY == videoParam->sizeY ) {
    // conversion to RGB (3 ubytes)
    // printf( "image conversion\n" );
    img_convert( rgbFrame->avPicture , 
		 PIX_FMT_RGB24, 
		 (AVPicture *)videoFrame, 
		 videoCodecContext->pix_fmt, 
		 rgbFrame->sizeX , rgbFrame->sizeY );
  }
  else 

//   {
//     img_convert( rgbNPOTFrame->avPicture , 
// 		 PIX_FMT_RGB24, 
// 		 (AVPicture *)videoFrame, 
// 		 videoCodecContext->pix_fmt, 
// 		 rgbNPOTFrame->sizeX , rgbNPOTFrame->sizeY );

//     uint8_t *src = rgbNPOTFrame->avPicture->data[0];
//     uint8_t *dst = rgbFrame->avPicture->data[0];
//     int width = rgbFrame->sizeX * rgbFrame->bytesPerPixel;
//     int NPOTwidth = rgbNPOTFrame->sizeX * rgbNPOTFrame->bytesPerPixel;

//     for( int height = 0 ; height < rgbNPOTFrame->sizeY ; height++ ) {
//       memcpy( dst , src , NPOTwidth );
//       src += NPOTwidth;
//       dst += width;
//     }
//   }

  {
    img_convert( rgbFrame->avPicture , 
		 PIX_FMT_RGB24, 
		 (AVPicture *)videoFrame, 
		 videoCodecContext->pix_fmt, 
		 NPOTSizeX , NPOTSizeY );
  }
}

// rewinds a video

bool vc_AudioVideoData::start_video( void ) {
  // neither audio nor video -> returns
  if (!videoCodecContext && !audioCodecContext)
    return false;

  // no video opened
  if (!videoFormatContext) {
    return false;
  }

  // printf( "start video\n" );

  // flush both queues
  videoQueue->flushQueue(queueMutex);
  audioQueue->flushQueue(queueMutex);

  // empty the audio buffer
  audioBufferSize = 0;

  // acquire the video mutex
  pthread_mutex_lock(&videoMutex);

  // seeks the beginning of the video file if the stream is valid
  if( !liveVideo && (audioStreamIndex >= 0 || videoStreamIndex >= 0) ) {
    int ret = av_seek_frame(videoFormatContext,-1, 0, AVSEEK_FLAG_BACKWARD);
    if( ret < 0 ) {
      fprintf( stderr , "Error: incorrect video file rewind!" ); 
    }
    if (videoCodecContext != NULL) {
      avcodec_flush_buffers(videoCodecContext);
    }
  }

  // release the video mutex
  pthread_mutex_unlock(&videoMutex);

  // timer reset
  currentFrameTime = 0.0;
  totalFileTime = 0.0;
  frameNo = 0;
  double the_time = glutGet(GLUT_ELAPSED_TIME) / 1000.0;
  lastAbsoluteTime = (double)the_time;
  
  // acquires the first frame
  ReadNextFrameStatus readStatus = readStatus;
  if( (readStatus = readNextFrame()) != VC_READ_FRAME_ERROR ) {
    if( readStatus == VC_GOT_VIDEO_FRAME ) {
      img_convert_and_resize();
    }
    // Release the packet
    av_free_packet(&moviePacket);
  }
  else {
    fprintf(stderr,"read first frame failed, continuing anyway...\n");
    return true;
  }
  return true;
}
/*!
 * \brief rewinds a video
 */
bool vc_AudioVideoData::rewind_video( void ) {
  // neither audio nor video -> returns
  if (!videoCodecContext && !audioCodecContext)
    return false;

  // no video opened
  if (!videoFormatContext) {
    return false;
  }

  // printf( "flushes video\n" );

  // flush both queues
  videoQueue->flushQueue( queueMutex );
  audioQueue->flushQueue( queueMutex );
  newFrame = false;

  // empty the audio buffer
  audioBufferSize = 0;

  // acquire the video mutex
  pthread_mutex_lock( &videoMutex );

  // seeks the beginning of the video file if the stream is valid
  if( !liveVideo && (audioStreamIndex >= 0 || videoStreamIndex >= 0) ) {
    // printf( "rewind video\n" );
    int ret = av_seek_frame(videoFormatContext,videoStreamIndex, 0, AVSEEK_FLAG_BACKWARD);
    rewindable = true;
    if( ret < 0 ) {
      strcpy( ErrorStr , "Error: incorrect video file rewind!" ); ReportError( ErrorStr );
      // release the video mutex
      pthread_mutex_unlock(&videoMutex);

      rewindable = false;
      return false;
    }

    // flushes buffers
    if (videoCodecContext != NULL) {
      avcodec_flush_buffers(videoCodecContext);
    }
  }

  // release the video mutex
  pthread_mutex_unlock(&videoMutex);

  // timer reset
  currentFrameTime = 0.0;
  totalFileTime = 0.0;
  frameNo = 0;
  double the_time = RealTime();
  lastAbsoluteTime = (double)the_time;
  lastOpenGLFrameUpdate = FrameNo - 1;
  
  // // acquires the first frame
  // if( !liveVideo ) {
  //   ReadNextFrameStatus readStatus;
  //   if( (readStatus = readNextFrame()) != VC_READ_FRAME_ERROR ) {
  //     if( readStatus == VC_GOT_VIDEO_FRAME ) {
  // 	fprintf(stderr,"got video frame at startup...\n" );
  // 	img_convert_and_resize( false );
  //     }
  //     else {
  // 	fprintf(stderr,"got no video frame at startup...\n" );
  // 	return true;
  //     }
  //     // Release the packet
  //     av_free_packet(&moviePacket);
  //   }
  //   else {
  //     fprintf(stderr,"read frame at startup failed...\n" );
  //     // for some video formats such as DivX, rewind 
  //     // is difficult and can take several frames
  //     // I suppose that the solution could be to close
  //     // and reopen the file...
  //     // another solution is to use an easily rewindable
  //     // format for video loops (such as cinepak or mjpeg)
      
  //     // this close/reopen does not work as it is
  //     // it would be necessary to make a reopen
  //     // different from load film that only
  //     // makes what has to be made for reopening
  //     //     if( !liveVideo ) {
  //     //       av_close_input_file(videoFormatContext);
  //     //       loadFilm( filmName , 0 , 0 );
  //     //     }
  //     return true;
  //   }
  // }
  return true;
}

bool vc_AudioVideoData::GoToNextFrame( void ) {
  // neither audio nor video -> returns
  if( !videoCodecContext && !audioCodecContext ) {
    return false;
  }

  // Add the specified time to the video timer
  double the_time = glutGet(GLUT_ELAPSED_TIME) / 1000.0;
  float elapsed_time = (double)the_time - lastAbsoluteTime;
  
  currentFrameTime += elapsed_time; // mise a jour temps de la frame
  
  
  // if the time for the current frame is greater than the video's
  // time-per-frame, then advance the frame
  bool packetAllocated = false;
  ReadNextFrameStatus readStatus = VC_GOT_NO_VIDEO_FRAME;
  // printf( "currentFrameTime %.5f  totalFileTime %.5f videoParam->frameDuration %.5f frame No %d\n" , currentFrameTime ,  totalFileTime , videoParam->frameDuration , frameNo );
  while( currentFrameTime > videoParam->frameDuration ) {
    // printf( "currentFrameTime %.5f  videoParam->frameDuration %.5f frame No %d\n" , currentFrameTime , videoParam->frameDuration , frameNo );

    // computes the current frame time 
    // in order to check whether we have reached the current frame
    currentFrameTime -= videoParam->frameDuration;

    // reads a frame
    if( (readStatus = readNextFrame()) != VC_READ_FRAME_ERROR ) {
      if( currentFrameTime > videoParam->frameDuration ) {
	// Release the packet because we are going to reader further frames
	// otherwise, packet release must be performed after rgb conversion
	av_free_packet(&moviePacket);
      } 
      else {
	packetAllocated = true;
      }
      frameNo++;
    }
  }
  if( readStatus == VC_GOT_VIDEO_FRAME ) {
    // converts to rgb only if a new frame is created
    img_convert_and_resize();
  }
  if( packetAllocated ) {
    // Release the packet
    av_free_packet(&moviePacket);
  }

  totalFileTime += elapsed_time; // mise a jour temps courant fichier lu
  lastAbsoluteTime += elapsed_time; // mise a jour temps courant

  //printf( "End of Goto next frame\n" );
  return (readStatus == VC_GOT_VIDEO_FRAME);
}

/////////////////////////////////////////////////////////////////////
// packet queue management
//////////////////////////////////////////////////////////////////////

// adds to a queue

void vc_AudioVideoData::queuePushPacket( vc_AudioVideoPacketQueue *queue, 
					 AVPacket *packet) {
  AVPacketList *packetQueueEntry;

  // Duplicate the packet to keep it from being invalidated by
  // the next av_read_frame() call
  av_dup_packet(packet);

  // Create a new packet queue entry
  packetQueueEntry = (AVPacketList *)malloc(sizeof(AVPacketList));
  packetQueueEntry->pkt = *packet;
  packetQueueEntry->next = NULL;

  // Lock the queue mutex
  pthread_mutex_lock(&queueMutex);

  // Add the packet to the video packet queue
  if (queue->tail == NULL)
    queue->head = packetQueueEntry;
  else
    queue->tail->next = packetQueueEntry;

  // Update the tail of the queue
  queue->tail = packetQueueEntry;

  // Update the packet count
  queue->nbPackets++;

  // Unlock the queue mutex
  pthread_mutex_unlock(&queueMutex);
}

// removes from a queue

bool vc_AudioVideoData::queuePullPacket( vc_AudioVideoPacketQueue *queue, 
					 AVPacket *packet)
{
    AVPacketList *packetQueueEntry;

    // Lock the queue mutex
    pthread_mutex_lock(&queueMutex);

    // Try to get the packet from the head of the queue
    packetQueueEntry = queue->head;

    if (packetQueueEntry != NULL)
    {
      //fprintf(stderr,"read 1 frame in the queue !\n");
	// Update the queue
        queue->head = packetQueueEntry->next;
        if (queue->head == NULL)
            queue->tail = NULL;
        queue->nbPackets--;

        // Extract the packet from the queue element to the packet structure
        // provided and free the element
        *packet = packetQueueEntry->pkt;
        av_free(packetQueueEntry);

        // Unlock the queue mutex
        pthread_mutex_unlock(&queueMutex);

        // Return true to indicate a successful dequeue
        return true;
    }
    else
    {
      //fprintf(stderr,"nothing to read, queue empty !\n");
      // Unlock the queue mutex
      pthread_mutex_unlock(&queueMutex);

      // Return false to indicate an empty queue
      return false;
    }
}


///////////////////////////////////////////////////////////
// VIDEO PROPERTIES
///////////////////////////////////////////////////////////

vc_Video::vc_Video( void ) {
  id = NULL;
  fileName = NULL;

  type = EmptyVideoType;
  videoOn = false;
  nbPeriods = 0;
  timeInPeriod = 0;
  sourceInitialized = false;
  sourceOn = false;
  begin = 0;
  end = 0;
  duration = 0;
  period = 0; 
  filmAudioVideoIn = new vc_AudioVideoData;

  // video  and audio threads
  fprintf(stderr,"creating threads...\n");
  pthread_create(&(filmAudioVideoIn->videoThread), NULL, fillVideoBuffer, this);
  pthread_create(&(filmAudioVideoIn->audioThread), NULL, fillAudioBuffer, this);
  fprintf(stderr,"creating threads done...\n");
  
}

////////////////////////////////////////////////////////////
// updating a video
////////////////////////////////////////////////////////////
void vc_Video::update( void ) {
  // printf( "video update %d\n" , on );
  // video loop: in case of spatializer: starts once/stops once
  // video loop: in case of playing interface: restarted at each period
  // already updated in another instance of this video
  // nothing to do
  if( filmAudioVideoIn->lastOpenGLFrameUpdate == FrameNo ) {
    // printf( "video already updated \n" );
    return;
  }

  if( type == VIDEOLOOP ) {
    // outside the playing time
    if( CurrentClockTime < begin || CurrentClockTime > end ) {
      // printf( "outside interval\n" );
      if( videoOn ) {
	stop_video();
	videoOn = false;
      }
    }
    // inside the playing time
    else if( !videoOn ) {
      // printf( "non on loop restart\n" );
      start_video( true );
      videoOn = true;
      nbPeriods = 1;
      timeInPeriod = 0;
    }
      
    int indCurrentPeriod = (int)floorf((CurrentClockTime - begin)/period) + 1; 
    float currentTimeInPeriod 
      = CurrentClockTime - begin - (indCurrentPeriod - 1) * period;
    if( !videoOn ) {
      nbPeriods = 0;
      timeInPeriod = 0;
    }
    //printf( "  nbPeriods %d indCurrentPeriod %d currentTimeInPeriod %.2f on %d\n" , nbPeriods , indCurrentPeriod , currentTimeInPeriod , on );
    // rewinds if a new period begins or if the
    // elapsed time in a period is less than the current elapsed time
    // as it is the case after a begin=now action
    if( indCurrentPeriod != nbPeriods || currentTimeInPeriod < timeInPeriod ) {
      start_video( true );
      videoOn = true;
    }
    // inside the playing time

    nbPeriods = indCurrentPeriod;
    timeInPeriod = currentTimeInPeriod;
  }
  // video loop

  // non looping video: starts once/stops once
  else if( type == VIDEOFINITE ) {
    // outside the playing time
    if( CurrentClockTime < begin || CurrentClockTime > end 
	|| CurrentClockTime > begin + duration ) {
      if(videoOn) {
	stop_video();
	videoOn = false;
      }
    }
    // inside the playing time
    else {
      if( !videoOn) {
	start_video( false );
	videoOn = true;
      }
    }
  }  
}

////////////////////////////////////////////////////////////
// parsing a video
////////////////////////////////////////////////////////////
char *vc_Video::GetId( void ) {
  return id;
} 

void vc_Video::SetId( char *newId ) {
  StrCpy( &id , newId );
}

void vc_Video::stop_video( void ) {
  // printf( "stop video\n" );
  videoOn = false;
}

bool vc_Video::rewind_video( void ) {
  if( !filmAudioVideoIn->rewind_video() ) {
    // printf( "rewind error\n" );
    videoOn = false;
    return false;
  }
 // sets the video status videoOn
  videoOn = true;
  return true;
}
void vc_Video::displayVideo( int textureNo ) {
  if( filmAudioVideoIn->newFrame && FrameNo > 10  ) {
    LoadVideoFrameInGPUMemory( textureNo );
  }
}

void vc_Video::LoadVideoFrameInGPUMemory( int textureNo ) {
  // printf( "video to GPU %d %d\n" , textureNo,videoOn);

  // not on: no new texture to display
  if ( !videoOn || textureNo < 0 )
    return;

  if( TextureID[ textureNo ] == NULL_ID ) {
    GLuint texID = 0;
    glGenTextures(1, &texID);
    TextureID[ textureNo ] = texID;
  }

  // these instructions are executed by  vc_Texture::draw that precedes vc_Video::bindTexture
  // binds the texture id
  // bind the texture to the texture arrays index and initXMLTag the texture
  glEnable(GL_TEXTURE_RECTANGLE_ARB);
  glBindTexture(GL_TEXTURE_RECTANGLE_ARB, TextureID[ textureNo ] );

  glTexParameteri(GL_TEXTURE_RECTANGLE_ARB, GL_TEXTURE_MIN_FILTER,  
		  GL_LINEAR);
  glTexParameteri(GL_TEXTURE_RECTANGLE_ARB, GL_TEXTURE_MAG_FILTER, 
		  GL_LINEAR);
  
  // printf( "Loads GL_TEXTURE_RECTANGLE_ARB texture No %d internal_format=%d (%s) %dx%d (%d)\n" , textureNo ,format  , VC_CompositionNode->texture_names[textureNo] ,filmAudioVideo->rgbFrame->sizeX , filmAudioVideo->rgbFrame->sizeY, (int)(filmAudioVideo->rgbFrame->avPicture->data[0][0]));
  glTexImage2D(GL_TEXTURE_RECTANGLE_ARB, 0, GL_RGB8,
	       filmAudioVideoIn->rgbFrame->sizeX ,
	       filmAudioVideoIn->rgbFrame->sizeY , 
	       0,
	       GL_RGB, GL_UNSIGNED_BYTE, 
	       (const void *)filmAudioVideoIn->rgbFrame->avPicture->data[0] );
}

////////////////////////////////////////////////////////////
// reading a video
////////////////////////////////////////////////////////////

void vc_Video::print( FILE *file , int isLong , int depth ) {
  if( !(*id) ) {
    return;
  }
  fprintf( file , "<video id=\"%s\" xlink:href=\"%s\" type=\"%s\" begin=%.5f end=%.5f duration=%.5f period=%.5f/>\n" , 
	   id , fileName , VideoTypeString[type]  , begin , end , duration , period );
}

////////////////////////////////////////////////////////////
// video and audio buffer processing
////////////////////////////////////////////////////////////

// fills frame buffer with images from a film

void *vc_Video::fillVideoBuffer(void *film) {
  vc_Video    *currentFilm;
  vc_AudioVideoData     *currentAudioVideoData;
  AVPacket               audioVideoPacket;
  int readVideoBufferStatus;
  bool needVideo, needAudio;
    
  // Initialize readVideoBufferStatus
  readVideoBufferStatus = 0;

  // Get the currentAudioVideoData of the reader object from the parameter
  currentFilm = (vc_Video *)film;
  currentAudioVideoData = currentFilm->filmAudioVideoIn;

  // Keep looping until we're signaled to quit by the main thread
  while( true ) {
    //fprintf(stderr,"loop fillbuffer\n");
    // make sure the queues are always  full.  
    if( currentFilm->videoOn ) {
      // Initialize the read status to zero (no error)
      readVideoBufferStatus = 0;
      //fprintf(stderr,"fill video buffer\n");
      // Initialize the flags indicating whether or not we need
      // video and audio
      needVideo = (currentAudioVideoData->videoCodecContext != NULL) && 
	(currentAudioVideoData->videoQueue->nbPackets < VC_VIDEO_PACKET_QUEUE_SIZE);
      needAudio = (currentAudioVideoData->audioCodecContext != NULL) && 
	(currentAudioVideoData->audioQueue->nbPackets < VC_VIDEO_PACKET_QUEUE_SIZE);

      // Keep reading until we hit a read error, or we read enough
      // video and/or audio
      while( readVideoBufferStatus >= 0 && (needVideo || needAudio) ) {
	// Acquire the file mutex
	pthread_mutex_lock(&currentAudioVideoData->videoMutex);
	
	// Try to read a packet
	readVideoBufferStatus = av_read_frame(
			currentAudioVideoData->videoFormatContext, 
			&audioVideoPacket );
	
	// if correct read, adds to the corresponding queue
	if (readVideoBufferStatus >= 0)  {
	  if ((audioVideoPacket.stream_index == 
	       currentAudioVideoData->videoStreamIndex) &&
	      (currentAudioVideoData->videoCodecContext != NULL)) {
	    // adds the packet to the video queue
	    currentAudioVideoData->queuePushPacket(
				currentAudioVideoData->videoQueue, 
				&audioVideoPacket );
	  }
	  else if ((audioVideoPacket.stream_index == 
		    currentAudioVideoData->audioStreamIndex) &&
		   (currentAudioVideoData->audioCodecContext != NULL)) {
	    // adds the packet to the audio queue
	    currentAudioVideoData->queuePushPacket(
				 currentAudioVideoData->audioQueue, 
				 &audioVideoPacket );
	  }
	  else {
	    // Throw the packet away
	    av_free_packet(&audioVideoPacket);
	  }
	}
	
	// Update the video and audio flags
	needVideo = (currentAudioVideoData->videoCodecContext != NULL) && 
	  (currentAudioVideoData->videoQueue->nbPackets < VC_VIDEO_PACKET_QUEUE_SIZE);
	needAudio = (currentAudioVideoData->audioCodecContext != NULL) && 
	  (currentAudioVideoData->audioQueue->nbPackets < VC_VIDEO_PACKET_QUEUE_SIZE);
	
// 	printf( "nb audio packets %d need audio %d readVideoBufferStatus %d is Audio %d %d\n" , 
// 		currentAudioVideoData->audioQueue->nbPackets ,
// 		needAudio , readVideoBufferStatus , audioVideoPacket.stream_index , 
// 		currentAudioVideoData->audioStreamIndex );
// 	printf( "nb video packets %d need video %d readVideoBufferStatus %d is Video %d %d\n" , 
// 		currentAudioVideoData->videoQueue->nbPackets ,
// 		needVideo , readVideoBufferStatus , audioVideoPacket.stream_index , 
// 		currentAudioVideoData->videoStreamIndex  );

	// Release the file mutex
	pthread_mutex_unlock(&currentAudioVideoData->videoMutex);
      }
    }
    
    // end of the file, sets the on value to false for the main thread
    if( readVideoBufferStatus < 0 && currentFilm->videoOn ) {
      fprintf(stderr, "end of the file\n" );
      currentFilm->videoOn = false;
    }
    
    
    // Sleep for a while to yield the processor to other threads
    usleep(10000);
  }
  
  return NULL;
}

// fills sound buffer with packets from a sound film

void *vc_Video::fillAudioBuffer(void *film) {
  vc_Video    *currentFilm;
  vc_AudioVideoData     *currentAudioVideoData;

  // Get the currentAudioVideoData of the reader object from the parameter
  currentFilm = (vc_Video *)film;
  currentAudioVideoData = currentFilm->filmAudioVideoIn;

  // Keep looping until we're signaled to quit by the main thread
  while( true ) {
    // If we're currently playing audio, see if we need to queue a
    // new buffer on the audio stream
    if( currentFilm->videoOn ) {
      int samplespPerF = currentAudioVideoData->audioParam->audioSamplesPerFrame;
      // Check if it's time to update the audio stream
      while (//(currentAudioVideoData->soundStream != NULL) && 
	     // (currentAudioVideoData->soundStream->isBufferReady()) && 
	     (currentAudioVideoData->audioBufferSize > samplespPerF )) {
	// Lock the audio mutex
	pthread_mutex_lock(&currentAudioVideoData->audioMutex);

	// Copy the data from the local audio buffer to the sound 
	// stream
	//currentAudioVideoData->soundStream->queueBuffer(currentAudioVideoData->audioBuffer);

	// Slide the data in the local buffer down and update the
	// buffer size
	memmove(currentAudioVideoData->audioBuffer, 
		&currentAudioVideoData->audioBuffer[samplespPerF], 
		currentAudioVideoData->audioBufferSize - samplespPerF);
	currentAudioVideoData->audioBufferSize -= samplespPerF;

	// Unlock the audio mutex
	pthread_mutex_unlock(&currentAudioVideoData->audioMutex);
      }
    }

    // Sleep for a while
    usleep(10000);
  }
    
  return NULL;
}

////////////////////////////////////////////////////////////
// video frame by frame openGL display
////////////////////////////////////////////////////////////

bool vc_Video::start_video( int is_loop ) {
  if( !filmAudioVideoIn->start_video() ) {
    // printf( "start error\n" );
    videoOn = false;
    return false;
  }

  // sets the video status on
  videoOn = true;
  return true;
}


void vc_Video::operator=(vc_Video& c) {
  StrCpy( &fileName , c.fileName );
  StrCpy( &id , c.id );
  begin = c.begin;
  end = c.end;
  period = c.period;
  duration = c.duration;
}

///////////////////////////////////////////////////////////
// AUDIO-VIDEO OUTPUT
///////////////////////////////////////////////////////////

///////////////////////////////////////////////////////////
/* AUDIO OUTPUT */

/* 
 * add an audio output stream
 */
vc_AudioVideoOutData::vc_AudioVideoOutData( void ) {
  videoFormatContext = NULL;

  // VIDEO
  videoStream = NULL;
  videoCodecContext = NULL;
  videoCodec = NULL;
  // temporary storage of a video frame
  // (used to store the yuv image before its conversion into rgb)
  videoFrame = NULL;
//   aux_videoFrame = NULL;
  
  videoBuffer = NULL;
  videoBuffer_size = 0;

  // video parameters initialization
  videoParam = new vc_VideoParameters;
  videoParam->sizeX = 0;
  videoParam->sizeY = 0;
  videoParam->frameDuration = 0.0;
  videoParam->filmDuration = 0.0;

  // RGB frame
  rgbFrame = new vc_ImageRRGB;
  // allocates a frame that will hold the rgb image
  // pixel buffer initialized in a second step when the image size is known
  rgbFrame->avPicture = (AVPicture *)malloc(sizeof(AVPicture));
  // resets the rgb frame
  memset(rgbFrame, 0, sizeof(rgbFrame));
  rgbFrame->bytesPerPixel = 3;
  rgbFrame->sizeX = 0;
  rgbFrame->sizeY = 0;
  rgbFrame->sizeZ = 0;
  rgbFrame->avPicture->data[0] = NULL;
  rgbFrame->avPicture->data[1] = NULL;
  rgbFrame->avPicture->data[2] = NULL;
  rgbFrame->avPicture->data[3] = NULL;
  rgbFrame->avPicture->linesize[0] = 0;
  rgbFrame->avPicture->linesize[1] = 0;
  rgbFrame->avPicture->linesize[2] = 0;
  rgbFrame->avPicture->linesize[3] = 0;

  // AUDIO
  audioCodecContext = NULL;
  audioStream = NULL;
  audioCodec = NULL;
  
  // temporary storage of audio data before 
  // before streaming
  audioBuffer = NULL;
  
  // player management
  frameNo = 0;
}

void vc_AudioVideoOutData::add_audio_stream(int codec_id) {
  audioStream = av_new_stream(videoFormatContext, 1);
  if (!audioStream) {
    fprintf( stderr, "Error: Could not alloc audio stream!" ); exit(0);
  }

  audioCodecContext = &audioStream->codec;
  audioCodecContext->codec_id = (CodecID)codec_id;
  audioCodecContext->codec_type = CODEC_TYPE_AUDIO;

  /* put sample parameters */
  audioCodecContext->bit_rate = 64000;
  audioCodecContext->sample_rate = 44100;
  audioCodecContext->channels = 2;
}

void vc_AudioVideoOutData::open_audio( void ) {
  audioCodecContext = &audioStream->codec;

  /* find the audio encoder */
  audioCodec = avcodec_find_encoder(audioCodecContext->codec_id);
  if (!audioCodec) {
    fprintf( stderr , "Error: audio codec not found!" ); exit(0);
  }

  /* open it */
  if (avcodec_open(audioCodecContext, audioCodec) < 0) {
    fprintf( stderr , "Error: audio codec not opened!" ); exit(0);
  }

  audioBuffer = new uint8_t[ VC_AUDIO_OUTPUT_BUF_SIZE ];

  /* ugly hack for PCM codecs (will be removed ASAP with new PCM
     support to compute the input frame size in samples */

  // allocs memory for making dummy samples 
  //     if (audioCodecContext->frame_size <= 1) {
  //         audio_input_frame_size = VC_AUDIO_OUTPUT_BUF_SIZE 
  // 	  / audioCodecContext->channels;
  //         switch(audioStream->codec.codec_id) {
  //         case CODEC_ID_PCM_S16LE:
  //         case CODEC_ID_PCM_S16BE:
  //         case CODEC_ID_PCM_U16LE:
  //         case CODEC_ID_PCM_U16BE:
  //             audio_input_frame_size >>= 1;
  //             break;
  //         default:
  //             break;
  //         }
  //     } else {
  //         audio_input_frame_size = audioCodecContext->frame_size;
  //     }
  //     samples = malloc(audio_input_frame_size * 
  // 		     2 * audioCodecContext->channels);
}

void vc_AudioVideoOutData::write_audio_frame( void ) {
  av_init_packet(&audioPacket);
    
  audioCodecContext = &audioStream->codec;

  //     get_audio_frame( samples, audio_input_frame_size, 
  // 		     audioCodecContext->channels );

  //     audioPacket.size= avcodec_encode_audio( audioCodecContext, audioBuffer,
  // 					    VC_AUDIO_OUTPUT_BUF_SIZE, 
  // 					    samples );

  audioPacket.pts= audioCodecContext->coded_frame->pts;
  audioPacket.flags |= PKT_FLAG_KEY;
  audioPacket.stream_index= audioStream->index;
  audioPacket.data= audioBuffer;

  /* write the compressed frame in the media file */
  if (av_write_frame(videoFormatContext, &audioPacket) != 0) {
    fprintf( stderr , "Error: error while writing audio frame!" ); exit(0);
  }
}

// closes audio and release memory

void vc_AudioVideoOutData::close_audio( void ) {
  avcodec_close(&audioStream->codec);
    
  //     av_free(samples);
  av_free(audioBuffer);
}

///////////////////////////////////////////////////////////
/* VIDEO OUTPUT */

/* add a video output stream */
void vc_AudioVideoOutData::add_video_stream( int codec_id ,
					     int videoWidth ,
					     int videoHeight ) {
  videoStream = av_new_stream(videoFormatContext, 0);
  if (!videoStream) {
    fprintf( stderr , "Error: Could not alloc video stream!" ); exit(0);
  }
    
  videoCodecContext = &videoStream->codec;
  videoCodecContext->codec_id = (CodecID)codec_id;
  videoCodecContext->codec_type = CODEC_TYPE_VIDEO;

  /* put sample parameters */
  videoCodecContext->bit_rate = 400000;
  /* resolution must be a multiple of two */
  videoCodecContext->width = videoWidth;  
  videoCodecContext->height = videoHeight;
  /* frames per second */
  videoCodecContext->frame_rate = VC_OUTSTREAM_FRAME_RATE;  
  videoCodecContext->frame_rate_base = 1;
  videoCodecContext->gop_size = 12; 
  /* emit one intra frame every twelve frames at most */
  if (videoCodecContext->codec_id == CODEC_ID_MPEG2VIDEO) {
    /* just for testing, we also add B frames */
    videoCodecContext->max_b_frames = 2;
  }
  if (videoCodecContext->codec_id == CODEC_ID_MPEG1VIDEO){
    /* needed to avoid using macroblocks in which some coeffs overflow 
       this doesnt happen with normal video, it just happens here as the 
       motion of the chroma plane doesnt match the luma plane */
    videoCodecContext->mb_decision=2;
  }
  // some formats want stream headers to be seperate
  if(!strcmp(videoFormatContext->oformat->name, "mp4") 
     || !strcmp(videoFormatContext->oformat->name, "mov") 
     || !strcmp(videoFormatContext->oformat->name, "3gp"))
    videoCodecContext->flags |= CODEC_FLAG_GLOBAL_HEADER;
}

AVFrame *vc_AudioVideoOutData::alloc_videoFrame( int pix_fmt, 
						 int width, int height ) {
  AVFrame *videoFr;
  uint8_t *videoFrame_buf;
  int size;
    
  videoFr = avcodec_alloc_frame();
  if (!videoFr)
    return NULL;
  size = avpicture_get_size(pix_fmt, width, height);
  videoFrame_buf = new uint8_t[ size ];
  if (!videoFrame_buf) {
    av_free(videoFr);
    return NULL;
  }
  avpicture_fill((AVPicture *)videoFr, videoFrame_buf, 
		 pix_fmt, width, height);
  return videoFr;
}
    
void vc_AudioVideoOutData::open_video( void ) {
  videoCodecContext = &videoStream->codec;

  /* find the video encoder */
  videoCodec = avcodec_find_encoder(videoCodecContext->codec_id);
  if (!videoCodec) {
    fprintf( stderr, "Error: video codec not found!" ); exit(0);
  }

  /* open the codec */
  if (avcodec_open(videoCodecContext, videoCodec) < 0) {
    fprintf(stderr , "Error: video codec not opened!" ); exit(0);
  }

  videoBuffer = NULL;
  if (!(videoFormatContext->oformat->flags & AVFMT_RAWPICTURE)) {
    /* allocate output buffer */
    /* XXX: API change will be done */
    videoBuffer_size = 200000;
    videoBuffer = new uint8_t[ videoBuffer_size ];
  }

  // rgb frame points to back buffer
  rgbFrame->sizeX = videoCodecContext->width;
  rgbFrame->sizeY = videoCodecContext->height;
  rgbFrame->bytesPerPixel = 3;
  rgbFrame->avPicture->data[0] 
    = new GLubyte[rgbFrame->sizeX * rgbFrame->sizeY 
		  * rgbFrame->bytesPerPixel];
  rgbFrame->avPicture->linesize[0] 
    = rgbFrame->sizeX * rgbFrame->bytesPerPixel;

  /* allocate the encoded raw videoFrame */
  videoFrame = alloc_videoFrame( videoCodecContext->pix_fmt, 
				 videoCodecContext->width, 
				 videoCodecContext->height);
  if (!videoFrame) {
    fprintf( stderr , "Error: video frame not allocated (fmt %d, %dx%d)!" , videoCodecContext->pix_fmt, videoCodecContext->width, videoCodecContext->height); exit(0);
  }

//   /* if the output format is not YUV420P, then a temporary YUV420P
//      video frame is needed too. It is then converted to the required
//      output format */
//   aux_videoFrame = NULL;
//   if (videoCodecContext->pix_fmt != PIX_FMT_YUV420P) {
//     aux_videoFrame = alloc_videoFrame( PIX_FMT_YUV420P, 
// 				       videoCodecContext->width, 
// 				       videoCodecContext->height);
//     if (!aux_videoFrame) {
//       sprintf( ErrorStr , "Error: temporary video frame not allocated!" ); exit(0);
//     }
//   }
}

void vc_AudioVideoOutData::write_video_frame( void ) {
  int out_size, ret;
  AVFrame *videoFrame_ptr;
    
  videoCodecContext = &videoStream->codec;
    
  if (frameNo >= videoParam->filmDuration / videoParam->frameDuration ) {
    /* no more frame to compress. The codec has a latency of a few
       frames if using B frames, so we get the last frames by
       passing a NULL videoFrame */
    videoFrame_ptr = NULL; 
  } else {
    /* OpenGL's default 4 byte pack alignment would leave extra bytes at the
       end of each image row so that each full row contained a number of bytes
       divisible by 4.  Ie, an RGB row with 3 pixels and 8-bit componets would
       be laid out like "RGBRGBRGBxxx" where the last three "xxx" bytes exist
       just to pad the row out to 12 bytes (12 is divisible by 4). To make sure
       the rows are packed as tight as possible (no row padding), set the pack
       alignment to 1. */
    glPixelStorei(GL_PACK_ALIGNMENT, 1);

//     memset( rgbFrame->avPicture->data[0], (GLubyte)255, 
// 	    rgbFrame->sizeX * rgbFrame->sizeY 
// 	    * rgbFrame->bytesPerPixel );

    if (videoCodecContext->pix_fmt != PIX_FMT_RGB24 ) {
      /* as we only generate a YUV420P videoFrame, we must convert it
	 to the codec pixel format if needed */
      //             fill_yuv_image( aux_videoFrame, frameNo, 
      // 			    videoCodecContext->width, 
      // 			    videoCodecContext->height );
      // copies the back buffer
      uint8_t bufferVideo[1024 * 1024 * 3];
      glReadPixels(0, 0, videoCodecContext->width, videoCodecContext->height, 
		   GL_RGB, GL_UNSIGNED_BYTE, bufferVideo);

      // image is upside down
      uint8_t *dest = rgbFrame->avPicture->data[0];
      for(int n = 0 ; n < videoCodecContext->height ; n++ ) {
	bcopy( &bufferVideo[n * videoCodecContext->width * 3 ], 
	       &dest[(videoCodecContext->height - (n + 1)) 
		     * videoCodecContext->width * 3 ], 
	       videoCodecContext->width * 3 );
      }

      // converts into the video format
      img_convert( (AVPicture *)videoFrame, videoCodecContext->pix_fmt, 
		   rgbFrame->avPicture, PIX_FMT_RGB24 ,
		   videoCodecContext->width, videoCodecContext->height );
    } else {
      glReadPixels(0, 0, videoCodecContext->width, videoCodecContext->height, 
		   GL_RGB, GL_UNSIGNED_BYTE, 
		   videoFrame->data[0]);
      //             fill_yuv_image(videoFrame, frameNo, 
      // 			   videoCodecContext->width, 
      // 			   videoCodecContext->height);
    }
    videoFrame_ptr = videoFrame;
  }

    
  if (videoFormatContext->oformat->flags & AVFMT_RAWPICTURE) {
    /* raw video case. The API will change slightly in the near
       futur for that */
    av_init_packet(&moviePacket);
        
    moviePacket.flags |= PKT_FLAG_KEY;
    moviePacket.stream_index= videoStream->index;
    moviePacket.data= (uint8_t *)videoFrame_ptr;
    moviePacket.size= sizeof(AVPicture);
        
    ret = av_write_frame(videoFormatContext, &moviePacket);
  } else {
    /* encode the image */
    out_size = avcodec_encode_video( videoCodecContext, videoBuffer, 
				     videoBuffer_size, videoFrame_ptr);
    /* if zero size, it means the image was buffered */
    if (out_size != 0) {
      av_init_packet(&moviePacket);
            
      moviePacket.pts= videoCodecContext->coded_frame->pts;
      if(videoCodecContext->coded_frame->key_frame)
	moviePacket.flags |= PKT_FLAG_KEY;
      moviePacket.stream_index= videoStream->index;
      moviePacket.data= videoBuffer;
      moviePacket.size= out_size;
            
      /* write the compressed frame in the media file */
      ret = av_write_frame(videoFormatContext, &moviePacket);
    } else {
      ret = 0;
    }
  }
  if (ret != 0) {
    fprintf(stderr , "Error: error while writing video frame!" ); exit(0);
  }
  frameNo++;
}

// closes video and release memory

void vc_AudioVideoOutData::close_video( void ) {
  avcodec_close(&videoStream->codec);
  av_free(videoFrame->data[0]);
  av_free(videoFrame);
  if (rgbFrame) {
    if (rgbFrame->avPicture) {
      av_free(rgbFrame->avPicture->data[0]);
      av_free(rgbFrame->avPicture);
    }
    av_free(rgbFrame);
  }
  av_free(videoBuffer);
}

// closes audio and video, writes trailer and release memory

void vc_AudioVideoOutData::close_audio_video_out( bool withAudio , 
						  bool withVideo ) {
  /* close each codec */
  if( withVideo && videoStream )
    close_video();
  if( withAudio && audioStream )
    close_audio();

  /* write the trailer, if any */
  av_write_trailer(videoFormatContext);
    
  /* free the streams */
  for(int i = 0; i < videoFormatContext->nb_streams; i++) {
    av_freep(&videoFormatContext->streams[i]);
  }

  if (!(videoFormatContext->oformat->flags & AVFMT_NOFILE)) {
    /* close the output file */
    url_fclose(&videoFormatContext->pb);
  }

  /* free the stream */
  av_free(videoFormatContext);
}

/**************************************************************/
/* media file output */

// /* prepare a dummy image */
// void fill_yuv_image(AVFrame *pict, int frame_index, int width, int height)
// {
//   int x, y, i;

//   i = frame_index;

//   /* Y */
//   for(y=0;y<height;y++) {
//     for(x=0;x<width;x++) {
//       pict->data[0][y * pict->linesize[0] + x] = x + y + i * 3;
//     }
//   }
    
//   /* Cb and Cr */
//   for(y=0;y<height/2;y++) {
//     for(x=0;x<width/2;x++) {
//       pict->data[1][y * pict->linesize[1] + x] = 128 + y + i * 2;
//       pict->data[2][y * pict->linesize[2] + x] = 64 + x + i * 5;
//     }
//   }
// }

/* prepare a 16 bit dummy audio frame of 'frame_size' samples and
   'nb_channels' channels */
// void get_audio_frame(int16_t *samples, int frame_size, int nb_channels) {
//     int j, i, v;
//     int16_t *q;

//     q = samples;
//     for(j=0;j<frame_size;j++) {
//         v = (int)(sin(t) * 10000);
//         for(i = 0; i < nb_channels; i++)
//             *q++ = v;
//         t += tincr;
//         tincr += tincr2;
//     }
// }

vc_AudioVideoOutData *open_audio_video_out( char *filename ,
					    bool withAudio , 
					    bool withVideo ,
					    int videoWidth ,
					    int videoHeight ) {
  vc_AudioVideoOutData *avOutData;
  AVOutputFormat *fmt;
  
  /* initialize libavcodec, and register all codecs and formats */
  av_register_all();
  
  /* auto detect the output format from the name. default is
     mpeg. */
  fmt = guess_format(NULL, filename, NULL);
  if (!fmt) {
    printf("Could not deduce output format from file extension: using MPEG.\n");
    fmt = guess_format("mpeg", NULL, NULL);
  }
  if (!fmt) {
    fprintf(stderr , "Error: suitable video output format not found, check file extension %s!" , filename ); exit(0);
  }
    
  /* allocate the output media context */
  avOutData = new vc_AudioVideoOutData();

  avOutData->videoFormatContext = av_alloc_format_context();
  if (!avOutData->videoFormatContext) {
    fprintf( stderr , "Error: audio/video memory error!" ); exit(0);
  }
  avOutData->videoFormatContext->oformat = fmt;
  snprintf(avOutData->videoFormatContext->filename, 
	   sizeof(avOutData->videoFormatContext->filename), 
	   "%s", filename);

  /* add the audio and video streams using the default format codecs
     and initialize the codecs */
  if( withVideo && fmt->video_codec != CODEC_ID_NONE ) {
    avOutData->add_video_stream(fmt->video_codec , videoWidth , videoHeight );
  }
  if( withAudio && fmt->audio_codec != CODEC_ID_NONE ) {
    avOutData->add_audio_stream(fmt->audio_codec);
  }

  /* set the output parameters (must be done even if no
     parameters). */
  if (av_set_parameters(avOutData->videoFormatContext, NULL) < 0) {
    fprintf(stderr , "Error: Invalid video output format parameters!" ); exit(0);
  }

  dump_format( avOutData->videoFormatContext , 0 , filename , 1 );

  /* now that all the parameters are set, we can open the audio and
     video codecs and allocate the necessary encode buffers */
  if( withVideo && avOutData->videoStream )
    avOutData->open_video();
  if( withAudio && avOutData->audioStream )
    avOutData->open_audio();

  /* open the output file, if needed */
  if (!(fmt->flags & AVFMT_NOFILE)) {
    if (url_fopen(&avOutData->videoFormatContext->pb, filename, URL_WRONLY) < 0) {
      fprintf( stderr , "Error: video output file not opened %s!" , filename ); exit(0);
    }
  }
    
  /* write the stream header, if any */
  av_write_header( avOutData->videoFormatContext );

  return avOutData;
}

void vc_AudioVideoOutData::write_frame( bool withAudio , 
					bool withVideo ) {
  double audio_pts, video_pts;

  /* compute current audio and video time */
  if (withAudio) {
    if(audioStream)
      audio_pts = (double)audioStream->pts.val 
	* audioStream->time_base.num / audioStream->time_base.den;
    else
      audio_pts = 0.0;
  }
  
  if (withVideo) {
    if (videoStream)
      video_pts = (double)videoStream->pts.val 
	* videoStream->time_base.num / videoStream->time_base.den;
    else
      video_pts = 0.0;
  }

//   if ((!audioStream || audio_pts >= videoParam->filmDuration) && 
//       (!videoStream || video_pts >= videoParam->filmDuration) )
//     return;
  
  /* write interleaved audio and video frames */
  if( withAudio && (!videoStream || (videoStream 
				     && audioStream && audio_pts < video_pts)) ) {
    write_audio_frame();
  } 
  else {
    write_video_frame();
  }
}

