#include "PngWrite.h"
#include "png.h"
#include <stdlib.h>
#include <iostream>

bool WritePNG(int m_width, int m_height, unsigned char* m_pData, int m_bytesPerPixel, colorType_e m_colorType, int m_clevel, const char* in_sName)
{
  int colorType, bitDepthPerChannel;

  FILE *pFile = NULL;	
  pFile = fopen(in_sName,"wb");

  if(!pFile)
    return false;

  png_structp pPng = png_create_write_struct(PNG_LIBPNG_VER_STRING, NULL, NULL, NULL);
  if (!pPng)
  {
    fclose(pFile);
    return false;   
  }

  png_infop pPngInfo = png_create_info_struct(pPng);
  if (!pPngInfo) 
  {
    fclose(pFile);
    png_destroy_write_struct(&pPng, NULL);
    return false;   
  }

  // it's a goto (in case png lib hits the bucket)
  if (setjmp(png_jmpbuf(pPng))) 
  {
    fclose(pFile);
    png_destroy_write_struct(&pPng, &pPngInfo);
    return false;
  }

  png_init_io(pPng, pFile);

  // compression level 0(none)-9(best compression) 
  png_set_compression_level(pPng, m_clevel);

  switch(m_colorType)
  {
  case COLOR_GRAY:
    bitDepthPerChannel = m_bytesPerPixel*8;
    colorType = PNG_COLOR_TYPE_GRAY;
    break;

  case COLOR_RGB:
    bitDepthPerChannel = (m_bytesPerPixel/3)*8; 
    colorType = PNG_COLOR_TYPE_RGB;
    break;
  
  case COLOR_RGBA:
  case VARICOLOR_BASE:
  case VARICOLOR_MASK:
  case WONDERDRAFT_SYMBOL:
    bitDepthPerChannel = (m_bytesPerPixel/4)*8; 
    colorType = PNG_COLOR_TYPE_RGB_ALPHA;
    break;
  
  // Unknown color type - bail out
  default:
    // Report error
    std::cerr << "Unknown color type " << m_colorType << " specified for PNG writing." << std::endl;
    fclose(pFile);
    png_destroy_write_struct(&pPng, &pPngInfo);
    return false;
  }

  png_set_IHDR(pPng, pPngInfo, (ULONG)m_width, (ULONG)m_height,
               bitDepthPerChannel, colorType, PNG_INTERLACE_NONE,
               PNG_COMPRESSION_TYPE_DEFAULT, PNG_FILTER_TYPE_DEFAULT);
			   
  png_write_info(pPng, pPngInfo);
  
  if(bitDepthPerChannel == 16) //reverse endian order (PNG is Big Endien)
    png_set_swap(pPng);

  int bytesPerRow = m_width*m_bytesPerPixel;
  BYTE* pImgData = m_pData;

  // Create RGB buffer
  char *rgbRawData =  (char *)malloc(bytesPerRow * m_height);
  BYTE* rgbImgData = (BYTE *)rgbRawData;
  
  // Convert input data to RGB/RGBA format for PNG writing
  for(int row=0; row < m_height; ++row) {
	  for (int pixel=0; pixel < m_width; ++pixel) {

      BYTE shifted = pImgData[pixel] << 1; // twice as light
      if (shifted < pImgData[pixel]) // check overflow
        shifted = 255;

      switch (m_colorType) {

      case VARICOLOR_MASK:
      // The RGB values aren't really used in the mask, but set them to alpha for visualization
        rgbImgData[pixel*4]     = pImgData[pixel];   // red
        rgbImgData[pixel*4 + 1] = pImgData[pixel];   // green
        rgbImgData[pixel*4 + 2] = pImgData[pixel];   // blue
        rgbImgData[pixel*4 + 3] = pImgData[pixel];   // alpha
        break;

      case VARICOLOR_BASE:
        // Make the base color shifted to a lighter hue so when the 
        // mask in CC3+ is applied the color doesn't get too dark and muddied.
        rgbImgData[pixel*4]     = shifted;     // red
        rgbImgData[pixel*4 + 1] = shifted;     // green
        rgbImgData[pixel*4 + 2] = shifted;     // blue
        rgbImgData[pixel*4 + 3] = pImgData[pixel]; // alpha
        break;

      case WONDERDRAFT_SYMBOL:
        // Wonderdraft symbols use red channel for symbol custom coloring, alpha for transparency
      	rgbImgData[pixel*4]   = pImgData[pixel];   // red
	  	  rgbImgData[pixel*4 + 1] = 0;  // green
	  	  rgbImgData[pixel*4 + 2] = 0;  // blue
			  rgbImgData[pixel*4 + 3] = pImgData[pixel]; // alpha
        break;

      case COLOR_RGBA:
	  	  rgbImgData[pixel*4]     = 255 - pImgData[pixel]; // red
        rgbImgData[pixel*4 + 1] = 255 - pImgData[pixel]; // green
        rgbImgData[pixel*4 + 2] = 255 - pImgData[pixel]; // blue
        rgbImgData[pixel*4 + 3] = pImgData[pixel]; // alpha
        break;

      case COLOR_RGB:
        rgbImgData[pixel*3]     = pImgData[pixel];     // red
        rgbImgData[pixel*3 + 1] = pImgData[pixel]; // green
        rgbImgData[pixel*3 + 2] = pImgData[pixel]; // blue
        break;

      case COLOR_GRAY:
        rgbImgData[pixel] = pImgData[pixel]; // grayscale
        break;
      }	  
    }

	  pImgData += m_width;  // grayscale is 1 byte per pixel
    rgbImgData += bytesPerRow;
  }
  
  rgbImgData = (BYTE *)rgbRawData;
  
  //write non-interlaced buffer  
  for(int row=0; row < m_height; ++row) {
    png_write_row(pPng, rgbImgData);
    rgbImgData += bytesPerRow;
  }

  png_write_end(pPng, NULL);

  
  png_destroy_write_struct(&pPng, &pPngInfo);
  free(rgbRawData);
  fclose(pFile);

  return true;
}
