/////// * /////////// * /////////// * NEW * /////// * /////////// * /////////// *

// This loads a jpeg file and returns it's data
// This stores the important jpeg data
struct tImageJPG
{
	int rowSpan;
	int sizeX;
	int sizeY;
	unsigned char *data;
};

#define UINT unsigned int
#define LPSTR char *

struct tImageJPG *LoadJPG(const char *filename);

/////// * /////////// * /////////// * NEW * /////// * /////////// * /////////// *


// This loads a texture into OpenGL from a file (IE, "image.jpg")
void CreateTexture(UINT *textureId, LPSTR strFileName);

