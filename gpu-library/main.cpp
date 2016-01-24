// Programme de test de lecture/ecriture d'un stream video + shader

// les define sont tous là
#include "main.h"

int CPU_IMAGE_PROCESSING = 0;

// unused
float CurrentClockTime = 0.0;
// input name (must be COLOR png)

// frame number
int FrameNo = 0;

char input_video[100];
char lut_image[100];

// buffer d'entree du traitement: c'est l'IMAGE d'ENTREE
GLubyte *ImageSrc;
// buffer de recuperation du traitement: c'est l'IMAGE DE SORTIE
GLubyte *ImageDest;

// Objet video principal
vc_Video *vid;

// les textures utilisees par les shaders 
unsigned int TextureID[ NB_MAX_TEXTURES ];

unsigned int g_Texture[NB_MAX_TEXTURES];

///////////////////////////////////////
// FBO frame buffer objects

GLuint g_frameBuffer[NB_PASSES - 1];


/////////////////////////////
// VBO vertex buffer object
GLuint vertexBuffer; 
float *pVertexBuffer;

int main(int argc, char **argv) {
 
  bool readStatus;
  
  // texture ID initialization for GPU rendering and quad texturing
  for( int i = 0 ; i < NB_MAX_TEXTURES ; i++ ) {
    TextureID[ i ] = NULL_ID;
  }

  vid = new  vc_Video;
  vc_AudioVideoData *lefilm = vid->filmAudioVideoIn;
  
  // argument = nom du flux d'entrée, par exemple /dev/video0

  sprintf(input_video,"%s", argv[1]);
  // second arg is LUT
  sprintf(lut_image,"%s", argv[2]);
  // third arg is CPU/GPU switch
  CPU_IMAGE_PROCESSING = atoi(argv[3]);

  // chargement et initialisation
  if (lefilm->loadFilm(input_video)) {
    fprintf(stderr,"input %s ok\n", input_video);
  }
  else
    exit(0);
  
  // un peu d'info
  int ImageWidth = lefilm->rgbFrame->sizeX;
  int ImageHeight = lefilm->rgbFrame->sizeY;
  
  fprintf(stderr, "frame dimensions %d %d\n" , ImageWidth, ImageHeight);

  
  // recuperaton du buffer du flux dans un format RGB
  GLubyte *buffer = (GLubyte *)lefilm->rgbFrame->avPicture->data[0];
  ImageSrc = buffer;

  // allocation de l'image de sortie
  ImageDest = new GLubyte[3*ImageWidth*ImageHeight];
    
  // video de sortie: ouvrir avec open_audio_video_out 
  // le flux de sortie: vid->filmAudioVideoOut 

  
  // routines GLUT

  // inits
  glutInit(&argc, argv);
  glutInitDisplayMode(GLUT_RGBA | GLUT_DOUBLE);
  glutInitWindowSize(ImageWidth, ImageHeight);
  glutInitWindowPosition(500, 500);

  // creation de la fenetre
  glutCreateWindow(argv[1]);

  // initialisation de OpenGL de Cg et de la scene
  
  // initialisations OpenGL
  if( CPU_IMAGE_PROCESSING == 1) {
    mygl_CPUprocessing_init();
  }
  else {
    mygl_GPUprocessing_init(ImageWidth,ImageHeight);
  }

  // debut de processing du flux
  readStatus = vid->start_video(true);
  fprintf(stderr,"start_video = %d\n", readStatus);
  
  //callbacks glut
  glutDisplayFunc(display);
  glutReshapeFunc(reshape);
  glutKeyboardFunc(keyboard);
  glutMotionFunc(motion);
  glutIdleFunc(readframe);


  // main loop
  glutMainLoop();
  // fermer le flux de sortie video ici
  return(0);
}

