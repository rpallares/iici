/*****************************************************************************
File: texpng.c

Rendu Graphique Avancé RGA
Master Professionnel d'informatique
Christian Jacquemin, Université Paris 11

This file is provided without support, instruction, or implied
warranty of any kind.  University Paris 11 makes no guarantee of its
fitness for a particular purpose and is not liable under any
circumstances for any damages or loss whatsoever arising from the use
or inability to use this file or items derived from it.
******************************************************************************/
#include <GL/gl.h>
#include <GL/glu.h>
#include <string.h>
#include <stdio.h>
#include <stdlib.h>
#include <png.h>
#include "texpng.h"
#include "ctype.h"
#include "cg-lib.h"

///////////////////////////////// LOAD PNG \\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\*
/////
/////	This loads the PNG file and returns its data in a tImagePNG struct
/////
///////////////////////////////// LOAD PNG \\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\*

// This is the interesting function here. It sets up a PNG image as a texture.
void loadPngTex(char * fname , struct tImagePNG * pImagePNG , int *hasAlpha )
{
  // The header of the file will be saved in here
  char           buf[PNG_BYTES_TO_CHECK];
  
  // images infos
  int            bit_depth;
  int            cType;
  double         gamma;
  FILE          *fp;

  // Open the file and check correct opening
  /* Open the prospective PNG file. */
  if ((fp = fopen(fname, "rb")) == NULL) {
    printf( "Error: Could not open the texture file [%s]!" , fname ); throw 336;
  }
  
  // Read the PNG header, which is 8 bytes long.
  /* Read in some of the signature bytes */
  if (fread(buf, 1, PNG_BYTES_TO_CHECK, fp) != PNG_BYTES_TO_CHECK) {
    printf( "Error: Incorrect PNG texture file [%s]!" , fname ); throw 336;
  }

  // Check whether the file is a PNG file
  // png_sig_cmp() checks the given PNG header and returns 0 if it could
  // be the start of a PNG file. We can use 8 bytes at max for
  // this comparison.
  if(png_sig_cmp((png_byte*)buf, (png_size_t)0, PNG_BYTES_TO_CHECK) != 0) {
    fclose( fp );
    printf( "Error: Not a PNG file [%s]!" , fname ); throw 337;
  }
  
  // Create / initialize the png_struct
  // The png_struct isn't directly accessed by the user (us).
  // We will later create a png_info from this to get access to the
  // PNG's infos.
  // The three 0's in the arg list could be pointers to user defined error
  // handling functions. 0 means we don't want to specify them, but use
  // libPNG's default error handling instead.
  png_infop info_ptr;
  png_structp png_ptr
    = png_create_read_struct(PNG_LIBPNG_VER_STRING,
			     NULL , NULL , NULL );
  if(!png_ptr) {
    fclose( fp );
    printf( "Error: Couldn't create PNG read structure [%s]!" , fname ); throw 338;
  }

  // Create / initialize the png_info
  // The png_info grants the user access to the PNG infos.
  info_ptr = png_create_info_struct(png_ptr);
  if(!info_ptr) {
    // We need to destroy the read_struct
    png_destroy_read_struct(&png_ptr, NULL, NULL);
    fclose( fp );
    printf( "Error: Couldn't create PNG info structure [%s]!" , fname ); throw 339;
  }
  
  // Setup error handler
  // This sets the point libPNG jumps back to is an error occurs.
  if (setjmp(png_jmpbuf(png_ptr))) {
      /* Free all of the memory associated with the png_ptr and info_ptr */
      png_destroy_read_struct(&png_ptr, &info_ptr, png_infopp_NULL);
      fclose(fp);
      /* If we get here, we had a problem reading the file */
      printf( "Error: Couldn't setup PNG error handler [%s]!" , fname ); throw 340;
   }
  
  /* Set up the input control if you are using standard C streams */
  png_init_io(png_ptr, fp);
  
  // This tells libPNG that we have already read 8 bytes from the start
  // of the file (for the header check above).
  png_set_sig_bytes(png_ptr, PNG_BYTES_TO_CHECK);

  // This reads the PNG file into the read and info structs
  /*
   * If you have enough memory to read in the entire image at once,
   * and you need to specify only transforms that can be controlled
   * with one of the PNG_TRANSFORM_* bits (this presently excludes
   * dithering, filling, setting background, and doing gamma
   * adjustment), then you can read the entire image (including
   * pixels) into the info structure with this call:
   */
  png_read_info(png_ptr, info_ptr);

  // Get some basic infos about the image from png_info structure
  // width & height in px, bit depth
  // interlace_method, compression_method, & filter_method are ignored
  png_get_IHDR(png_ptr, info_ptr, &(pImagePNG->sizeX), &(pImagePNG->sizeY), 
	       &bit_depth, &cType, 0, 0, 0);

  // COLOR TYPE read and possible corrections - then reread
  // Color type: we handle RGB and RGB_ALPHA (with Alpha) 
  // GRAY (luminance) and GRAY_ALPHA (luminance with Alpha)
  cType = png_get_color_type(png_ptr, info_ptr);
  // strip the pixels of a PNG stream with 16 bits per channel to 8 bits per channel
  if (bit_depth == 16) {
    png_set_strip_16(png_ptr);
  }
  // set transformation in png_ptr such that paletted images are expanded to RGB, 
  // grayscale images of bit-depth less than 8 are expanded to 8-bit images
  // tRNS chunks are expanded to alpha channels
  if (cType == PNG_COLOR_TYPE_PALETTE) {
    png_set_expand(png_ptr);
  }
  if (bit_depth < 8) {
    png_set_expand(png_ptr);
  }
  if (png_get_valid(png_ptr, info_ptr, PNG_INFO_tRNS)) {
    png_set_expand(png_ptr);
  }
  // if required set gamma conversion
  if (png_get_gAMA(png_ptr, info_ptr, &gamma)) {
    png_set_gamma(png_ptr, (double) 2.2, gamma);
  }

  // After the image transformations have been registered update info_ptr data
  png_read_update_info(png_ptr, info_ptr);
  // Gets again width, height and the new bit-depth and color-type
  png_get_IHDR(png_ptr, info_ptr, &(pImagePNG->sizeX), &(pImagePNG->sizeY), 
	       &bit_depth, &cType, 0, 0, 0);

  // We now calculate the *bytes* per pixel from the color type and the
  // bits per pixel.
  if((cType == PNG_COLOR_TYPE_RGB) && (bit_depth == 8)) {
    pImagePNG->bytesPerPixel = 3;
    *hasAlpha = false;
  }
  else if((cType == PNG_COLOR_TYPE_RGB_ALPHA) && (bit_depth == 8)) {
    pImagePNG->bytesPerPixel = 4;
    *hasAlpha = true;
  }
  else if((cType == PNG_COLOR_TYPE_GRAY) && (bit_depth == 8)) {
    pImagePNG->bytesPerPixel = 1;
    *hasAlpha = false;
  }
  else if((cType == PNG_COLOR_TYPE_GRAY_ALPHA) && (bit_depth == 8)) {
    pImagePNG->bytesPerPixel = 2;
    *hasAlpha = true;
  }
  else {
    /* clean up after the read, and free any memory allocated - REQUIRED */
    png_destroy_read_struct(&png_ptr, &info_ptr, png_infopp_NULL);
    fclose( fp );
    printf( "Error: PNG image [%s] type (%d) bit depth (%d) is unsupported!" , fname , cType , bit_depth ); throw 336;
  }
  
  // rowbytes is the width x number of channels. channels is not used currently
  unsigned int rowbytes = png_get_rowbytes(png_ptr, info_ptr);
  // unsigned int channels = png_get_channels(png_ptr, info_ptr);

  // Allocates memory to store the image
  pImagePNG->data = new GLubyte[pImagePNG->sizeY * rowbytes];

//   printf( "PNG image %ldx%ld (%d bitsperpixel/%d bytesPerPixel) size row %d total %ld\n" , 
// 	  pImagePNG->sizeX ,
// 	  pImagePNG->sizeY , bit_depth , pImagePNG->bytesPerPixel,
// 	  rowbytes , pImagePNG->sizeY * rowbytes);

  ////  Alternate solution without explicit row pointers
  //// and image read through png_get_rows
  // Returns a pointer to the array of array of png_bytes that holds the
  // image data.
  // png_byte **imageData = png_get_rows(png_ptr, info_ptr);
  //// must read row after row
  //   for( unsigned int i = 0 ; i < pImagePNG->sizeY ; i++ ) {
  //     printf( "Ind %d\n" , i );
  //     memcpy(&pImagePNG->data[(pImagePNG->sizeY - i - 1) * rowbytes],
  // 	   row[i], rowbytes);
  //  }

  ////  Alternate solution with explicit row pointers
  //// and image read through png_read_image
  // Allocates memory for an array of row-pointers
  png_byte ** row = new GLubyte * [pImagePNG->sizeY];
  // Sets the row-pointers to point at the correct offsets	
  for (unsigned int i = 0; i < pImagePNG->sizeY; i++) {
    row[i] = pImagePNG->data + (pImagePNG->sizeY - i - 1) * rowbytes;
  }
  // Reads the whole image
  png_read_image(png_ptr, row);
  // deallocate the now unuseful row pointers
  delete [] row;

  // Free the memory we used - we don't need it anymore
  /* clean up after the read, and free any memory allocated - REQUIRED */
  png_destroy_read_struct(&png_ptr, &info_ptr, png_infopp_NULL);
  fclose( fp );
}

///////////////////////////// CREATE TEXTURE \\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\*
/////
/////	This creates a texture in OpenGL that we can use as a texture map
/////
///////////////////////////// CREATE TEXTURE \\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\*

void ReverseImage( int width , int height , unsigned char *data , int bytesPerPixel ) {
  unsigned char *dataaux = (unsigned char*)malloc(width * height * bytesPerPixel * sizeof(unsigned char));
  int indrow;
  for( indrow = 0 ; indrow < height ; indrow++ ) {
    memcpy(&dataaux[indrow * width * bytesPerPixel],
	   &data[(height - indrow - 1) * width * bytesPerPixel], width * bytesPerPixel);
  }
  for( indrow = 0 ; indrow < height ; indrow++ ) {
    memcpy(&data[indrow * width * bytesPerPixel],
	   &dataaux[indrow * width * bytesPerPixel], width * bytesPerPixel);
  }
  free(dataaux);
}

// The image dimensions must be powers of 2.
void CheckImageDimensions( int sizeX , int sizeY , char * strFileName) {  
  // Check the image width
  int comp = 2;
  int isValid = 0;
  int i;
  for( i = 0; i < 10; i++) {
    comp <<= 1;
    if(sizeX == comp)
      isValid = 1;
  }
  if(!isValid) {
    printf( "Error: The image width is not a power of 2 [%s]!" , strFileName ); 
    exit( 0 );
  }  
  
  // Check the image height
  comp = 2;
  isValid = 0;
  for( i = 0; i < 10; i++) {
    comp <<= 1;
    if(sizeY == comp)
      isValid = 1;
  }
  if(!isValid) {
    printf( "Error: The image height is not a power of 2 [%s]!" , strFileName );
    exit( 0 );
  }  
}

///////////////////////////////// CREATE TEXTURE \\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\*
/////
/////	This creates a texture in OpenGL that we can use as a texture map
/////
///////////////////////////////// CREATE TEXTURE \\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\*

void CreateTexturePNG(unsigned int *textureId, char * strFileName)
{

  if(!strFileName)			
    // Return from the function if no file name was passed in
    return;
  
  
  // Load the image and store the data
  struct tImagePNG *pImagePNG;
  int hasAlpha = 0;
  pImagePNG = new tImagePNG;
  loadPngTex(strFileName , pImagePNG , &hasAlpha );  
	
  // Generate a texture with the associative texture ID stored in the array
  glGenTextures(1, textureId);

  // Bind the texture to the texture arrays index and init the texture
  glBindTexture(GL_TEXTURE_2D, *textureId);

  // Build Mipmaps (builds different versions of the picture for distances - looks better)
  if( hasAlpha ) {
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER,  
		    GL_LINEAR_MIPMAP_LINEAR);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, 
		    GL_LINEAR);
    gluBuild2DMipmaps(GL_TEXTURE_2D, GL_RGBA8,
		      pImagePNG->sizeX, pImagePNG->sizeY,  
		      GL_RGBA, 
		      GL_UNSIGNED_BYTE, (const void *)pImagePNG->data); 
  }
  else {
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER,  
		    GL_LINEAR_MIPMAP_LINEAR);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, 
		    GL_LINEAR);
    gluBuild2DMipmaps(GL_TEXTURE_2D, GL_RGB8,
		      pImagePNG->sizeX, pImagePNG->sizeY,  
		      GL_RGB, 
		      GL_UNSIGNED_BYTE, (const void *)pImagePNG->data); 
  }

  fprintf(stderr, "Loading [%s] %dx%d!\n" , strFileName , 
	  (int)pImagePNG->sizeX, (int)pImagePNG->sizeY ); 

  // Now we need to free the image data that we loaded since OpenGL stored it as a texture

  if (pImagePNG->data) 	        // If there is texture data
    {
      free(pImagePNG->data);	// Free the texture data, we don't need it anymore
    }
}

// encoding for Vertex Texture Fetch

void CreateTextureRECTPNG(unsigned int *textureId, char * strFileName)
{
  if(!strFileName)			
    // Return from the function if no file name was passed in
    return;
  
  
  // Load the image and store the data
  struct tImagePNG *pImagePNG;
  int hasAlpha = 0;
  pImagePNG = new tImagePNG;
  loadPngTex(strFileName , pImagePNG , &hasAlpha );  
	
  // Generate a texture with the associative texture ID stored in the array
  glGenTextures(1, textureId);

  // Bind the texture to the texture arrays index and init the texture
  glEnable( GL_TEXTURE_RECTANGLE_ARB );
  glBindTexture(GL_TEXTURE_RECTANGLE_ARB, *textureId);


  glTexParameteri(GL_TEXTURE_RECTANGLE_ARB, GL_TEXTURE_MIN_FILTER,  
		  GL_NEAREST_MIPMAP_NEAREST);
  glTexParameteri(GL_TEXTURE_RECTANGLE_ARB, GL_TEXTURE_MAG_FILTER, 
		  GL_NEAREST);
  if( hasAlpha ) {
    glTexImage2D(GL_TEXTURE_RECTANGLE_ARB, 0, GL_RGBA8 ,
		 pImagePNG->sizeX, pImagePNG->sizeY, 0, 
		 GL_RGBA, GL_UNSIGNED_BYTE, (const void *)pImagePNG->data );
  }
  else {
    glTexImage2D(GL_TEXTURE_RECTANGLE_ARB, 0, GL_RGB8 ,
		 pImagePNG->sizeX, pImagePNG->sizeY, 0, 
		 GL_RGB, GL_UNSIGNED_BYTE, (const void *)pImagePNG->data );
  }
  
  printf( "Loading [%s] %dx%d!\n" , strFileName , 
	  (int)pImagePNG->sizeX, (int)pImagePNG->sizeY ); 

  // Now we need to free the image data that we loaded since OpenGL stored it as a texture

  if (pImagePNG->data) 	        // If there is texture data
    {
      free(pImagePNG->data);	// Free the texture data, we don't need it anymore
    }
}

// rectangle encoding

void CreateTextureVPPNG(unsigned int *textureId, char * strFileName)
{
  if(!strFileName)			
    // Return from the function if no file name was passed in
    return;
  
  
  printf( "Loading [%s]!\n" , strFileName ); 

  // Load the image and store the data
  struct tImagePNG *pImagePNG;
  int hasAlpha = 0;
  pImagePNG = new tImagePNG;
  loadPngTex(strFileName , pImagePNG , &hasAlpha );  
	
  // Generate a texture with the associative texture ID stored in the array
  glGenTextures(1, textureId);

  // Bind the texture to the texture arrays index and init the texture
  glBindTexture(GL_TEXTURE_2D, *textureId);

  // Build Mipmaps (builds different versions of the picture for distances - looks better)
  if( hasAlpha ) {
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER,  
		    GL_NEAREST_MIPMAP_NEAREST);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, 
		    GL_NEAREST);
    glTexImage2D( GL_TEXTURE_2D , 0, GL_RGBA_FLOAT32_ATI , 
	          pImagePNG->sizeX, pImagePNG->sizeY, 
		  0, GL_RGBA, GL_UNSIGNED_BYTE, (const void *)pImagePNG->data );
  }
  else {
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER,  
		    GL_NEAREST_MIPMAP_NEAREST);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, 
		    GL_NEAREST);
    glTexImage2D( GL_TEXTURE_2D , 0, GL_RGB_FLOAT32_ATI , 
	          pImagePNG->sizeX, pImagePNG->sizeY, 
		  0, GL_RGB, GL_UNSIGNED_BYTE, (const void *)pImagePNG->data );
  }

  // Now we need to free the image data that we loaded since OpenGL stored it as a texture

  if (pImagePNG->data) 	        // If there is texture data
    {
      free(pImagePNG->data);	// Free the texture data, we don't need it anymore
    }
}
