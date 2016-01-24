// libtiff include files
#include <tiffio.h>     /* Sam Leffler's libtiff library. */
// libjpeg include files
#ifndef _WIN32
extern "C" {
#include <jpeglib.h>
#include <jerror.h>
}
#else
//#define JPEG_INTERNALS      /* Hard includes instead of using the JPEG6-LIBRARY:     */
extern "C" {
#include <jpeglib.h>
#include <jerror.h>
}
#endif

///////////////////////////////////////////////////////


#include <stdio.h>
#include <stdlib.h>
#include <pthread.h>
#include <string.h>
#include <unistd.h> 
#include <values.h>
#include <math.h>
#include <sys/time.h>
#include <png.h>


#define GL_GLEXT_PROTOTYPES
#include <GL/gl.h>           
#include <GL/glu.h>         
#include <GL/glut.h>    
#include <GL/glx.h>
#include <GL/glext.h>



#include <Cg/cgGL.h>

#include "cg-lib.h"
#include "texpng.h"

#include "vc-video.h"
#include "image.h"

#define NB_PASSES 2

#define NB_MAX_TEXTURES 10 // (NB_PASSES + 1)
#define TEXTURE_INI 0
#define TEXTURE_AUX 1
#define    TEXTURE_TRANSFER (NB_PASSES)


///////////////////////////////////////
// FBO frame buffer objects

extern GLuint g_frameBuffer[NB_PASSES - 1];


/////////////////////////////
// VBO vertex buffer object
extern GLuint vertexBuffer; 
extern float *pVertexBuffer;

#define BUFFER_OFFSET(i) ((char *)NULL + (i))

#define NULL_ID   0

extern int FrameNo;
extern float CurrentClockTime;
extern unsigned int TextureID[ NB_MAX_TEXTURES ];
extern char lut_image[100];
extern int CPU_IMAGE_PROCESSING;


  
