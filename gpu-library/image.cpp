// Programme de test de lecture/ecriture d'un stream video
// fonctions glut

#include "main.h"


 #include <sys/time.h>

extern GLubyte *ImageSrc;
extern GLubyte *ImageDest;
extern vc_Video *vid;

// default material: white
GLfloat white_transparent[4] = {1.0,1.0,1.0,0.0};

// display list
int My_QuadRECT2D = 0;
int My_MaillageRECT2D = 1;

// contexte GC
CGcontext cgContext;

// profil GC
CGprofile cgVertexProfile;
CGprofile cgFragmentProfile;

// programmes GC
CGprogram cgQuad_VertexProgram[NB_PASSES];            // TD10-VP.cg
CGprogram cgQuad_FragmentProgram[NB_PASSES];          // TD10-fx[1..4]-FS.cg

// parametres Cg pour le programme de dessin 
CGparameter decal_quad[NB_PASSES];                 // l'image initiale + images intermediaires
CGparameter aux_quad = NULL;                    // l'image auxiliaire
CGparameter transfer_quad = NULL;               // transfer texture (LUT)
CGparameter image_size_x = NULL;
CGparameter image_size_y = NULL;

// empty texture for video
float *dataFloat = NULL;

///////////////////////////////////////
// ERROR CHECKING
void checkGLErrors(const char *label) {
  GLenum errCode;
  const GLubyte *errStr;
  if ((errCode = glGetError()) != GL_NO_ERROR) {
    errStr = gluErrorString(errCode);
    printf("OpenGL ERROR: %s, Label: %s\n" , (char *)errStr , label );
  }
}

// callback function
void cgErrorCallback(void) {
  CGerror lastError = cgGetError();
  if(lastError) {
    printf("Cg ERROR: %s, Context: %s\n" , 
	   cgGetErrorString(lastError) , 
	   cgGetLastListing(cgContext) );
  }
}

bool checkFramebufferStatus(char * chaux) {
  GLenum status;
  status=(GLenum)glCheckFramebufferStatusEXT(GL_FRAMEBUFFER_EXT);
  switch(status) {
  case GL_FRAMEBUFFER_COMPLETE_EXT:
    return true;
  case GL_FRAMEBUFFER_INCOMPLETE_ATTACHMENT_EXT:
    printf("%s Framebuffer incomplete, incomplete attachment\n" , chaux);
    return false;
  case GL_FRAMEBUFFER_UNSUPPORTED_EXT:
    printf("%s Unsupported framebuffer format\n" , chaux);
    return false;
  case GL_FRAMEBUFFER_INCOMPLETE_MISSING_ATTACHMENT_EXT:
    printf("%s Framebuffer incomplete, missing attachment\n" , chaux);
    return false;
  case GL_FRAMEBUFFER_INCOMPLETE_DIMENSIONS_EXT:
    printf("%s Framebuffer incomplete, attached images \
                    must have same dimensions\n" , chaux);
    return false;
  case GL_FRAMEBUFFER_INCOMPLETE_FORMATS_EXT:
    printf("%s Framebuffer incomplete, attached images \
                     must have same format\n" , chaux);
    return false;
  case GL_FRAMEBUFFER_INCOMPLETE_DRAW_BUFFER_EXT:
    printf("%s Framebuffer incomplete, missing draw buffer\n" , chaux);
    return false;
  case GL_FRAMEBUFFER_INCOMPLETE_READ_BUFFER_EXT:
    printf("%s Framebuffer incomplete, missing read buffer\n" , chaux);
    return false;
  }
  return false;
}

// compilation de la display list d'un quad
// de meme dimension que le volume de vue
// avec une texture RECT dans le FS 

void MakeQuadRECT2D( void ) {
  if( ! My_QuadRECT2D ) {
    My_QuadRECT2D = glGenLists( 1 );
  }

  int ImageWidth = vid->filmAudioVideoIn->rgbFrame->sizeX;
  int ImageHeight = vid->filmAudioVideoIn->rgbFrame->sizeY;

  glNewList(My_QuadRECT2D, GL_COMPILE);
  {
    glBegin(GL_QUADS);
    {
      glTexCoord2f( 0.0 , (float)ImageHeight );
      glVertex3f( 0.0 , 0.0 , 0.0 ); 
      
      glTexCoord2f( (float)ImageWidth , (float)ImageHeight );
      glVertex3f( (float)ImageWidth , 0.0 , 0.0);
      
      glTexCoord2f( (float)ImageWidth , 0.0 );
      glVertex3f( (float)ImageWidth , (float)ImageHeight , 0.0);
      
      glTexCoord2f( 0.0 , 0.0 );
      glVertex3f( 0.0 , (float)ImageHeight , 0.0);
    }
    glEnd();
  }
  glEndList();
}

void MakeMaillageRECT2D (int maxI, int maxJ) {
  if(!My_MaillageRECT2D ) {
    My_MaillageRECT2D = glGenLists( 1 );
  }
  
  int ImageWidth = vid->filmAudioVideoIn->rgbFrame->sizeX;
  int ImageHeight = vid->filmAudioVideoIn->rgbFrame->sizeY;
  
  int sizeI = (float)ImageWidth / (float)maxI;
  int sizeJ = (float)ImageHeight / (float)maxJ;
  
  glNewList(My_MaillageRECT2D, GL_COMPILE);
  {
    for(int i=0; i<maxI; i++) {
      for(int j=0; j<maxJ; j++) {
	glBegin(GL_QUADS);
	{
	  glTexCoord2f(i*sizeI, (j+1)*sizeJ);
	  glVertex3f( i*sizeI , j*sizeJ , 0.0 );

	  glTexCoord2f((i+1)*sizeI, (j+1)*sizeJ);
	  glVertex3f( (i+1)*sizeI , j*sizeJ , 0.0 );

	  glTexCoord2f((i+1)*sizeI, j*sizeJ);
	  glVertex3f( (i+1)*sizeI , (j+1)*sizeJ , 0.0 );

	  glTexCoord2f(i*sizeI, j*sizeJ);
	  glVertex3f( i*sizeI , (j+1)*sizeJ , 0.0 );
	}
	glEnd();
      }
    }
  }
  glEndList();
  
}


///////////////////////////////////////
// INITIALISATION DE Cg

GLvoid initCg()  {
  char filename[256];
  char filename2[256];

  // register the error callback once the context has been created
  cgSetErrorCallback(cgErrorCallback);

  // creation d'un contexte pour les programmes Cg
  cgContext = cgCreateContext();
  // + verification
  if (cgContext == NULL) {
    printf( "Erreur de chargement du contexte Cg\n" ); exit( 0 );
  }
  cgGLRegisterStates(cgContext);
  cgGLSetManageTextureParameters(cgContext,CG_TRUE);


  // le dernier profil pour les vertex
  cgVertexProfile = cgGLGetLatestProfile(CG_GL_VERTEX);
  cgVertexProfile = cgGetProfile("vp40");
  // + verification
  if (cgVertexProfile == CG_PROFILE_UNKNOWN) {
    printf( "Profil de vertex invalide\n" ); exit( 0 );
  }
  cgGLSetOptimalOptions(cgVertexProfile);

  // le dernier profil pour les fragments
  cgFragmentProfile = cgGLGetLatestProfile(CG_GL_FRAGMENT);
  cgFragmentProfile = cgGetProfile("fp40");
  // + verification
  if (cgFragmentProfile == CG_PROFILE_UNKNOWN) {
    printf( "Profil de fragment invalide\n" ); exit( 0 );
  }
  cgGLSetOptimalOptions(cgFragmentProfile);

  // compile le programme de vertex a partir d'un fichier


  for( int pass = 0 ; pass < NB_PASSES ; pass++ ) {
    // compile le programme de vertex a partir d'un fichier
    sprintf( filename , "VPM-videofx-VP.cg" );
    //sprintf( filename , "THAUT-videofx-VP.cg" );
    printf( "Chargement du programme de vertex %s\n" , filename );
    cgQuad_VertexProgram[ pass ] 
      = cgCreateProgramFromFile( cgContext,        // le context Cg courant
				CG_SOURCE,        // programme source
				// vs. CG_OBJECT pour un
				// programme objet
				filename,         // le nom du fichier
				cgVertexProfile,  // le profil
				"main",           // le programme d'entree
				NULL );           // autres parametres
    // + verification
    if( cgQuad_VertexProgram[ pass ]  == NULL ) {
      // Cg nous renvoie un pointeur vers une erreur
      CGerror Error = cgGetError();
      
      // dont on affiche la chaine - generalement peu informative...
      fprintf(stderr,
	      "Chargement incorrect du programme de vertex de la passe %d: %s (%s)\n" ,
	      pass + 1 , filename , cgGetErrorString(Error) ); 
      
      exit( 0 );
    }
    
    if( pass == NB_PASSES - 1 ) {
      sprintf( filename , "VPM-videofx-out_FS.cg" );
    }
    else {
      sprintf( filename , "VPM-videofx-%d_FS.cg" , pass + 1 );
      }
    
    //top haut
    /*if( pass == NB_PASSES - 1 ) {
      sprintf( filename , "THAUT-videofx-out_FS.cg" );
    }
    else {
      sprintf( filename , "THAUT-videofx-%d_FS.cg" , pass + 1 );
      }*/
    
    printf( "Chargement du programme de fragment %s\n" , filename );
    cgQuad_FragmentProgram  [ pass ]  
      = cgCreateProgramFromFile( cgContext,        // le context Cg courant
				CG_SOURCE,        // programme source
				// vs. CG_OBJECT pour un
				// programme objet
				filename,         // le nom du fichier
				cgFragmentProfile,  // le profil
				"main",           // le programme d'entree
				NULL );           // autres parametres
    // + verification
    if( cgQuad_FragmentProgram[ pass ]  == NULL ) {
      // Cg nous renvoie un pointeur vers une erreur
      CGerror Error = cgGetError();
      
      // dont on affiche la chaine - generalement peu informative...
      printf( "Chargement incorrect du programme de fragment %s (%s)\n" ,
	     filename , cgGetErrorString(Error) ); 
      exit( 0 );
    }
  }
  
  
  /////////////////////////////////////////////////
    // chargement des programmmes de vertex et de fragment d'affichage
    // chargement du programme de vertex de quad
    for( int pass = 0 ; pass < NB_PASSES ; pass++ ) {
      cgGLLoadProgram(cgQuad_VertexProgram[ pass ]);
      
      // chargement du programme de fragment 
      cgGLLoadProgram(cgQuad_FragmentProgram[ pass ]);
      
      
      // le fragment shader est lie a la texture image
      image_size_x
	= cgGetNamedParameter(cgQuad_FragmentProgram[ pass ], "image_size_x");
      image_size_y
	= cgGetNamedParameter(cgQuad_FragmentProgram[ pass ], "image_size_y");
      decal_quad[ pass ] 
	= cgGetNamedParameter(cgQuad_FragmentProgram[ pass ], "decal");
      aux_quad
	= cgGetNamedParameter(cgQuad_FragmentProgram[ pass ], "aux");
      if ( !decal_quad[ pass ] || !aux_quad || !image_size_x || !image_size_y) {
	printf("Erreur dans le chargement des parametres du programme de fragment de dessin de la passe %d\n" , pass + 1 );
	exit(0);
      }
      // le dernier filtre prend une texture de transfert en entree
	if( pass == NB_PASSES - 1 ) {
	  transfer_quad
	    = cgGetNamedParameter(cgQuad_FragmentProgram[ pass ], "transfer");
	  if ( !transfer_quad ) {
	    printf("Erreur dans le chargement des parametres du programme de fragment de dessin de la passe %d\n" , pass + 1 );
	    exit(0);
	  }
	}
    }
}	

// OpenGL initialization
// for CPU processing
void mygl_CPUprocessing_init(void)
{
   glClearColor (0.0, 0.0, 0.0, 0.0);
   glShadeModel(GL_FLAT);
   glPixelStorei(GL_UNPACK_ALIGNMENT, 1);   
}

///////////////////////////////////////
// creation d'une texture 

void CreateTexture( int textureId , int sizeX , int sizeY ) {
  /////////////////////////////////////////////////////////////////////
  // textures associees aux FBO de stockage des sorties des shaders
  // textures rectangle de format GL_RGBA_FLOAT32_ATI

  GLfloat * dataFloat = NULL; 
  int memsize = sizeX * sizeY * 4;
  dataFloat = new GLfloat[ memsize ];
  
  // If we can't load the file, quit!
  if(dataFloat == NULL) {		  
    printf( "Texture allocation error!" ); throw 1; 
  }
  
  // init à 0
  memset(dataFloat, 0, memsize);

  // Generate a texture with the associative texture ID stored in the array
  glGenTextures(1, &TextureID[textureId] );
  printf( "Generated Texture No %d with OPENGL ID = %d\n" , textureId , TextureID[textureId] );
   
  // Bind the texture to the texture arrays index and init the texture
  glEnable( GL_TEXTURE_RECTANGLE_ARB );
  glBindTexture( GL_TEXTURE_RECTANGLE_ARB, TextureID[textureId]);
  
  /////////////////////////////////////////////////////////////////////
  // FBO des sorties des shaders de chacune des passes 
  // initialisee par les valeurs stockees dans dataFloat
  glTexImage2D( GL_TEXTURE_RECTANGLE_ARB , 0, GL_RGBA_FLOAT32_ATI ,  
		sizeX, sizeY, 
		0, GL_RGBA, 
		GL_FLOAT, (const void *)dataFloat );
  
  // parametres de rendu de texture
  glTexParameteri( GL_TEXTURE_RECTANGLE_ARB, GL_TEXTURE_MAG_FILTER, 
		   GL_NEAREST );
  glTexParameteri( GL_TEXTURE_RECTANGLE_ARB, GL_TEXTURE_MIN_FILTER, 
		   GL_NEAREST );
  glTexParameterf(GL_TEXTURE_RECTANGLE_ARB, GL_TEXTURE_WRAP_S, GL_CLAMP);
  glTexParameterf(GL_TEXTURE_RECTANGLE_ARB, GL_TEXTURE_WRAP_T, GL_CLAMP);
  glBindTexture( GL_TEXTURE_RECTANGLE_ARB, 0);

  // Now we need to free the image data that we loaded since OpenGL 
  // stored it as a texture
  if (dataFloat) {	        // If there is texture data
    free(dataFloat);         // Free the texture data, we don't need it anymore
  }
}

// OpenGL initialization
// for GPU processing
void mygl_GPUprocessing_init(int sizeX,int sizeY)
{
  int ImageWidth = vid->filmAudioVideoIn->rgbFrame->sizeX;
  int ImageHeight = vid->filmAudioVideoIn->rgbFrame->sizeY;


  //////////////////////////////////////////////////////
  // FBO initialization (frame buffer object ) with
  // empty texture for data storage
  for( int pass = 0 ; pass < NB_PASSES - 1 ; pass++ ) {
    glGenFramebuffersEXT( 1, &(g_frameBuffer[ pass ]) );
    
    glBindFramebufferEXT( GL_FRAMEBUFFER_EXT, g_frameBuffer[ pass ] ); // bind
    CreateTexture( TEXTURE_INI + 1 + pass , ImageWidth, ImageHeight);
    glBindFramebufferEXT( GL_FRAMEBUFFER_EXT, 0 ); // unbind 
  }


  /////////////////////////////////////////////////////
  // empty texture allocation
  // Bind the texture to the texture arrays index and init the texture
  glGenTextures(1, &(TextureID[TEXTURE_INI]));
  glEnable( GL_TEXTURE_RECTANGLE_ARB );
  glBindTexture( GL_TEXTURE_RECTANGLE_ARB, TextureID[TEXTURE_INI]);
   
  /////////////////////////////////////////////////////////////////////
  // memory allocation
  dataFloat = new GLfloat[ sizeX * sizeY * 4 ];
      
  // If we can't load the file, quit!
  if(dataFloat == NULL) {		  
    printf( "Texture allocation error!" ); throw 425;
  }
  
  // gl texture
  // create
  glTexImage2D( GL_TEXTURE_RECTANGLE_ARB , 0, GL_RGB8 , 
		sizeX, sizeY, 
		0, GL_RGBA, GL_UNSIGNED_BYTE, (const void *)dataFloat );
  // set params
  glTexParameteri( GL_TEXTURE_RECTANGLE_ARB, GL_TEXTURE_MAG_FILTER, 
		   GL_LINEAR );
  glTexParameteri( GL_TEXTURE_RECTANGLE_ARB, GL_TEXTURE_MIN_FILTER, 
		   GL_LINEAR );
  glTexParameterf(GL_TEXTURE_RECTANGLE_ARB, GL_TEXTURE_WRAP_S, GL_CLAMP);
  glTexParameterf(GL_TEXTURE_RECTANGLE_ARB, GL_TEXTURE_WRAP_T, GL_CLAMP);
  
  printf( "Input Texture  No %d with OPENGL ID  = %d\n" , TEXTURE_INI , 
	  TextureID[TEXTURE_INI] );

  /////////////////////////////////////////////////////
  // png image texture allocation
  CreateTexturePNG( &(TextureID[ TEXTURE_TRANSFER ]) , 
		    lut_image);
  printf( "transfer Texture has ID %d and OPENGL ID = %d \n" , TEXTURE_TRANSFER , 
	  TextureID[TEXTURE_TRANSFER] );

  // initialise les display lists 
  //MakeQuadRECT2D();
  MakeMaillageRECT2D(100,100);

  // initialise les shaders et les pointeurs vers les paramètres
  initCg();
}

// fonction de draw, appelée à chaque refresh écran
void display(void)
{
  int ImageWidth = vid->filmAudioVideoIn->rgbFrame->sizeX;
  int ImageHeight = vid->filmAudioVideoIn->rgbFrame->sizeY;

  // display with CPU

  if( CPU_IMAGE_PROCESSING == 1) {
    glClear(GL_COLOR_BUFFER_BIT);

    glRasterPos2i(0, 0);
    if (ImageDest != NULL) {
      // on écrit le buffer de sortie dans le buffer opengl
      glDrawPixels(ImageWidth, ImageHeight, GL_RGB, 
		   GL_UNSIGNED_BYTE, ImageDest);
    }
  }
  else { // GPU processing here
    for( int pass = 0 ; pass < NB_PASSES ; pass++ ) {
      // fprintf(stderr,"display\n");
      // si la passe n'est pas la derniere passe,
      // attachement du FBO g_frameBuffer[ pass ] 
      // associé à la texture g_Texture[TEXTURE_INI + 1 + pass] (entree passe suivante)
      // à la sortie du fragment shader de la passe courante
      // sinon sortie video, donc attachement au FBO nul
      if( pass < NB_PASSES - 1 ) {
	
       glBindFramebufferEXT( GL_FRAMEBUFFER_EXT, g_frameBuffer[ pass ] ); // bind
       
       glFramebufferTexture2DEXT( GL_FRAMEBUFFER_EXT, 
				 GL_COLOR_ATTACHMENT0_EXT,
				 GL_TEXTURE_RECTANGLE_ARB,
				 TextureID[ TEXTURE_INI+1+pass ],0);

       glDrawBuffer(GL_COLOR_ATTACHMENT0_EXT); // on écrit la sortie courante du shader dans le FBO
       checkGLErrors("Beginning of Pass");
       checkFramebufferStatus((char *)"Beginning of Pass");
      }
      else {
	// derniere passe
	glBindFramebufferEXT( GL_FRAMEBUFFER_EXT, 0 ); // unbind 
	checkGLErrors("Beginning of Pass");
      }
    
    // opaque black background
    glClearColor(0,0,0,1);        
    // clears the buffer bits
    glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT); 
    
    // reinitialise la projection
    glMatrixMode(GL_PROJECTION);
    glLoadIdentity();
    glOrtho( 0, ImageWidth , 0 , ImageHeight , -10.0, 10.00 );
    
    
    // blend de transparence
    glEnable(GL_BLEND);
    glBlendFunc(GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA);
    glTexEnvfv(GL_TEXTURE_ENV, GL_TEXTURE_ENV_COLOR, white_transparent );
    glTexEnvf(GL_TEXTURE_ENV, GL_TEXTURE_ENV_MODE, GL_BLEND); 
    
    // pas d'eclairement, c'est fait par les shaders
    glDisable(GL_LIGHTING);
    glEnable(GL_DEPTH_TEST);
    
    // toutes les transformations suivantes s´appliquent au modele de vue 
    glMatrixMode(GL_MODELVIEW);
    glLoadIdentity(); // loads new identity
    
    // programmes de dessin (vertex et fragment programmes)
    cgGLEnableProfile(cgVertexProfile);
    cgGLEnableProfile(cgFragmentProfile);
    cgGLBindProgram( cgQuad_VertexProgram[ pass ] );
    cgGLBindProgram( cgQuad_FragmentProgram[ pass ] );
    
    // lie les parametres uniformes au programme de vertex
    cgGLSetStateMatrixParameter(cgGetNamedParameter(cgQuad_VertexProgram[ pass ], 
						    "ModelViewProj"),
				CG_GL_MODELVIEW_PROJECTION_MATRIX,
				CG_GL_MATRIX_IDENTITY);
    cgGLSetStateMatrixParameter(cgGetNamedParameter(cgQuad_VertexProgram[ pass ], 
						    "ModelView"),
				CG_GL_MODELVIEW_MATRIX,
				CG_GL_MATRIX_IDENTITY);
    
    // dessine le quad 
    glPushMatrix();
    {
      cgGLSetParameter1f(image_size_x, (float)ImageWidth);
      cgGLSetParameter1f(image_size_y, (float)ImageHeight);
      
      /////////////////////////////////////////////////////////
      // les deux textures de travail
      cgGLSetTextureParameter( decal_quad[ pass ], 
			       TextureID[TEXTURE_INI + pass] );
      cgGLEnableTextureParameter( decal_quad[ pass ] );
      
      // la texture de transfer liee aussi a une image fixe
      if( pass == NB_PASSES - 1 && transfer_quad ) {
	cgGLSetTextureParameter( transfer_quad, 
				TextureID[TEXTURE_TRANSFER] );
	cgGLEnableTextureParameter( transfer_quad );
      }
      
      // call display list for display
      //glCallList( My_QuadRECT2D );
      glCallList( My_MaillageRECT2D );
      
      // desactivation des textures
      cgGLDisableTextureParameter( decal_quad[ pass ] );
      cgGLDisableTextureParameter( aux_quad );
      if( pass == NB_PASSES - 1 && transfer_quad ) {
	cgGLDisableTextureParameter( transfer_quad );
      }
    }
    glPopMatrix();
    
    cgGLUnbindProgram(cgVertexProfile);
    cgGLUnbindProgram(cgFragmentProfile);
    
    checkGLErrors("End of Pass" );
    //
    // Unbind the frame-buffer objects.
    //
    glBindFramebufferEXT( GL_FRAMEBUFFER_EXT, 0 );
    }

  glFlush();
  glutSwapBuffers();
  }
}


// fonction de reshape de la fenetre graphique
void reshape(int w, int h)
{
   glViewport(0, 0, (GLsizei) w, (GLsizei) h);
   glMatrixMode(GL_PROJECTION);
   glLoadIdentity();
   gluOrtho2D(0.0, (GLdouble) w, 0.0, (GLdouble) h);
}



void keyboard(unsigned char key, int x, int y)
{
   switch (key) {
      case 27:
         exit(0);
         break;
      case 'r':
	fprintf(stderr,"touch r\n");
         break;
      default:
         break;
   }
}

void motion(int x, int y)
{
   glFlush ();
}


// fonction d'idle, appelée quand aucun évènement glut (interaction, draw...) n'a lieu.

void readframe() {

  int i,j;
  GLubyte pix[3];
  struct timeval begint, endt;
  struct timezone timez;

  long duration=0;

  vc_AudioVideoData *lefilm = vid->filmAudioVideoIn;
  int ImageWidth = lefilm->rgbFrame->sizeX;
  int ImageHeight = lefilm->rgbFrame->sizeY;

  gettimeofday(&begint, &timez);
  // recuperation de la frame suivante
  bool readStatus = lefilm->GoToNextFrame();
  // fprintf(stderr,"idle, read procedure called\n");
  if (readStatus != VC_READ_FRAME_ERROR ) {
    // fprintf(stderr,"got  next frame !\n");
    if (readStatus == VC_GOT_VIDEO_FRAME) {
      gettimeofday(&endt, &timez);
      duration = (endt.tv_sec - begint.tv_sec) * 1000000 + endt.tv_usec - begint.tv_usec;
      
      // fprintf(stderr,"frame processing =%d microsecs freq  = %.3f fps\n", (int)duration, 1000000. / duration);
      if( CPU_IMAGE_PROCESSING == 1 ) {
	// processing: nothing
	for (j=0;j<ImageHeight;j++) {
	  for (i=0;i<ImageWidth;i++) {
	    get_pixel(pix, ImageSrc, ImageWidth, i,j);
	    set_pixel(pix,ImageDest,ImageWidth, i, j);
	  }
	} 

	// ici, optionnellement on peut recuperer la frame OpenGl courante pour la sauver via 
	// vid->filmAudioVideoOut->write_frame
      }
      else {
	// load current frame in texture id=0
	vid->LoadVideoFrameInGPUMemory( TEXTURE_INI );
      }
    }
  }
  else {
    if (vid->videoOn == false) {
      fprintf(stderr,"THE END !!\n");
      exit(0);
    }
  }
  
  // pourquoi y-a-il cela ?? Indice: regarder l'activité cpu
  if (duration < 10000) {
    usleep(10000-duration);
  }
  // refresh eventuel (si le temps de processing est plus petit que le temps d'affichage)
  FrameNo++;
  glutPostRedisplay();
}

// recuperation / mis a jour d'un pixel en CPU

void get_pixel(GLubyte *val, GLubyte *buff, int width, int i, int j ) {

  val[0] = buff[3*(width*j+i)];
  val[1] = buff[3*(width*j+i)+1];
  val[2] = buff[3*(width*j+i)+2];

}
 
void set_pixel(GLubyte *val, GLubyte *buff, int width, int i, int j ) {

  buff[3*(width*j+i)] = val[0];
  buff[3*(width*j+i)+1] = val[1];
  buff[3*(width*j+i)+2] = val[2];

}
  
