#define PNG_BYTES_TO_CHECK 4
typedef unsigned long png_uint_32;
// This stores png data
struct tImagePNG
{
	int bytesPerPixel;
	png_uint_32 sizeX;
	png_uint_32 sizeY;
	unsigned char *data;
};

typedef struct tImagePNG   *PtImagePNG;

void loadPngTex(char * fname , struct tImagePNG * pImagePNG , int *hasAlpha );
void ReverseImage( int width , int height , unsigned char *data , int bytesPerPixel );
void CheckImageDimensions( int sizeX , int sizeY , char * strFileName);
void CreateTexturePNG(unsigned int *textureId, char * strFileName);
void CreateTextureVPPNG(unsigned int *textureId, char * strFileName);
void CreateTextureRECTPNG(unsigned int *textureId, char * strFileName);
void CreateBumpTexturePNG(unsigned int *textureId, char * strFileName);
