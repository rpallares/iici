#include <GL/gl.h>
#include <GL/glu.h>
#include <string.h>
#include <stdio.h>
#include <stdlib.h>
extern "C" {
#include <jpeglib.h>
#include <jerror.h>
}
#include "ctype.h"
#include "texjpeg.h"

///////////////////////////////// DECODE JPG \\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\*
/////
/////	This decodes the jpeg and fills in the tImageJPG structure
/////
///////////////////////////////// DECODE JPG \\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\*

void DecodeJPG(jpeg_decompress_struct* cinfo, tImageJPG *pImageData)
{
	// Read in the header of the jpeg file
	jpeg_read_header(cinfo, TRUE);
	
	// Start to decompress the jpeg file with our compression info
	jpeg_start_decompress(cinfo);

	// Get the image dimensions and row span to read in the pixel data
	pImageData->rowSpan = cinfo->image_width * cinfo->num_components;
	pImageData->sizeX   = cinfo->image_width;
	pImageData->sizeY   = cinfo->image_height;
	
	// Allocate memory for the pixel buffer
	pImageData->data = new unsigned char[pImageData->rowSpan * pImageData->sizeY];
		
	// Here we use the library's state variable cinfo.output_scanline as the
	// loop counter, so that we don't have to keep track ourselves.
	
	// Create an array of row pointers
	unsigned char** rowPtr = new unsigned char*[pImageData->sizeY];
	for (int i = 0; i < pImageData->sizeY; i++)
		rowPtr[i] = &(pImageData->data[i*pImageData->rowSpan]);

	// Now comes the juice of our work, here we extract all the pixel data
	int rowsRead = 0;
	while (cinfo->output_scanline < cinfo->output_height) 
	{
		// Read in the current row of pixels and increase the rowsRead count
		rowsRead += jpeg_read_scanlines(cinfo, &rowPtr[rowsRead], cinfo->output_height - rowsRead);
	}
	
	// Delete the temporary row pointers
	delete [] rowPtr;

	// Finish decompressing the data
	jpeg_finish_decompress(cinfo);
}


///////////////////////////////// LOAD JPG \\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\*
/////
/////	This loads the JPG file and returns it's data in a tImageJPG struct
/////
///////////////////////////////// LOAD JPG \\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\*

tImageJPG *LoadJPG(const char *filename)
{
	struct jpeg_decompress_struct cinfo;
	tImageJPG *pImageData = NULL;
	FILE *pFile;
	
	// This is the only function you should care about.  You don't need to
	// really know what all of this does (since you can't cause it's a library!) :)
	// Just know that you need to pass in the jpeg file name, and get a pointer
	// to a tImageJPG structure which contains the width, height and pixel data.
	// Be sure to free the data after you are done with it, just like a bitmap.
	
	// Open a file pointer to the jpeg file and check if it was found and opened 
	if((pFile = fopen(filename, "rb")) == NULL) 
	{
		// Display an error message saying the file was not found, then return NULL
		fprintf( stderr , "Unable to load JPG File!");
		return NULL;
	}
	
	// Create an error handler
	jpeg_error_mgr jerr;

	// Have our compression info object point to the error handler address
	cinfo.err = jpeg_std_error(&jerr);
	
	// Initialize the decompression object
	jpeg_create_decompress(&cinfo);
	
	// Specify the data source (Our file pointer)	
	jpeg_stdio_src(&cinfo, pFile);
	
	// Allocate the structure that will hold our eventual jpeg data (must free it!)
	pImageData = new tImageJPG;

	// Decode the jpeg file and fill in the image data structure to pass back
	DecodeJPG(&cinfo, pImageData);
	
	// This releases all the stored memory for reading and decoding the jpeg
	jpeg_destroy_decompress(&cinfo);
	
	// Close the file pointer that opened the file
	fclose(pFile);

	// Return the jpeg data (remember, you must free this data after you are done)
	return pImageData;
}

/////// * /////////// * /////////// * NEW * /////// * /////////// * /////////// *


///////////////////////////////// CREATE TEXTURE \\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\*
/////
/////	This creates a texture in OpenGL that we can use as a texture map
/////
///////////////////////////////// CREATE TEXTURE \\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\*

void CreateTexture(UINT textureArray[], LPSTR strFileName, int textureID)
{
	if(!strFileName)									// Return from the function if no file name was passed in
		return;
	

/////// * /////////// * /////////// * NEW * /////// * /////////// * /////////// *	
	
	tImageJPG *pImage = LoadJPG(strFileName);			// Load the image and store the data

/////// * /////////// * /////////// * NEW * /////// * /////////// * /////////// *
	

	if(pImage == NULL) {									// If we can't load the file, quit!
	  printf( "Texture allocation error!\n" ) ; throw 0;
	}

	// Generate a texture with the associative texture ID stored in the array
	glGenTextures(1, &textureArray[textureID]);

	// Bind the texture to the texture arrays index and init the texture
	glBindTexture(GL_TEXTURE_2D, textureArray[textureID]);

   	glTexImage2D(GL_TEXTURE_2D, 0, GL_RGB, pImage->sizeX, pImage->sizeY,  
  		     0, GL_RGB, GL_UNSIGNED_BYTE, pImage->data); 
 	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR);
 	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);

	// Build Mipmaps (builds different versions of the picture for distances - looks better)
	// gluBuild2DMipmaps(GL_TEXTURE_2D, 3, pImage->sizeX, pImage->sizeY, GL_RGB, GL_UNSIGNED_BYTE, pImage->data);

	// Lastly, we need to tell OpenGL the quality of our texture map.  GL_LINEAR_MIPMAP_LINEAR
	// is the smoothest.  GL_LINEAR_MIPMAP_NEAREST is faster than GL_LINEAR_MIPMAP_LINEAR, 
	// but looks blochy and pixilated.  Good for slower computers though. 
		
//  	glTexParameteri(GL_TEXTURE_2D,GL_TEXTURE_MIN_FILTER,GL_LINEAR_MIPMAP_NEAREST);
//  	glTexParameteri(GL_TEXTURE_2D,GL_TEXTURE_MAG_FILTER,GL_LINEAR_MIPMAP_LINEAR);	

	// Now we need to free the image data that we loaded since OpenGL stored it as a texture

	if (pImage)										// If we loaded the image
	{
		if (pImage->data)							// If there is texture data
		{
			free(pImage->data);						// Free the texture data, we don't need it anymore
		}

		free(pImage);								// Free the image structure
	}
}

