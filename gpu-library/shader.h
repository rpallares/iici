#define KEY_ESC 27

// program constants
#define WIDTH  800
#define HEIGHT 600

// frame no
int FrameNo = 0;
int latence = 0;
int LastFrame = 0;
double LastFrameTime;

// textures
#define    MAX_TEXTURES 2
#define    TEXTURE_INI 0
#define    TEXTURE_TRANSFER 1

unsigned int g_Texture[MAX_TEXTURES];

// default material: white
GLfloat white_transparent[4] = {1.0,1.0,1.0,0.0};

///////////////////////////////////////
// verification d erreur

void checkGLErrors(const char *label);
bool checkFramebufferStatus(char * chaux);
void cgErrorCallback(void);

///////////////////////////////////////

// donnees Cg

// contexte GC
CGcontext cgContext;

// profil GC
CGprofile cgVertexProfile;
CGprofile cgFragmentProfile;

// programmes GC
CGprogram cgQuad_VertexProgram;            // xxxx-vp.cg
CGprogram cgQuad_FragmentProgram;          // xxxx-fs.cg

// parametres Cg pour le programme de dessin 
CGparameter decal_quad = NULL;         // limage initiale pour la premiere et la sortie de la passe precedente pour les suivantes
CGparameter transfer_quad = NULL;               // transfer texture
///////////////////////////////////////

// protos de fonctions
void init_scene();
void render_scene();
GLvoid initGL();
double RealTime( void );
GLvoid initCg();
GLvoid window_display();
void window_timer( int step );
GLvoid window_mouseFunc(int button, int state, int x, int y);
GLvoid window_motionFunc(int x, int y);

// display lists
void MakeQuadRECT2D( void );

// creation de la texture vide
void CreateTexture( int textureId , int sizeX , int sizeY );
