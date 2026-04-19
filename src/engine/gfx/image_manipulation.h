#ifndef ENGINE_GFX_IMAGE_MANIPULATION_H
#define ENGINE_GFX_IMAGE_MANIPULATION_H

#include <engine/image.h>

#include <cstdint>

 
bool ConvertToRgba(uint8_t *pDest, const CImageInfo &SourceImage);
 
bool ConvertToRgbaAlloc(uint8_t *&pDest, const CImageInfo &SourceImage);
 
bool ConvertToRgba(CImageInfo &Image);

 
void ConvertToGrayscale(const CImageInfo &Image);

 
void DilateImage(uint8_t *pImageBuff, int w, int h);
void DilateImage(const CImageInfo &Image);
void DilateImageSub(uint8_t *pImageBuff, int w, int h, int x, int y, int SubWidth, int SubHeight);

 
uint8_t *ResizeImage(const uint8_t *pImageData, int Width, int Height, int NewWidth, int NewHeight, int BPP);
 
void ResizeImage(CImageInfo &Image, int NewWidth, int NewHeight);

int HighestBit(int OfVar);

#endif  
