#ifndef ENGINE_IMAGE_H
#define ENGINE_IMAGE_H

#include <base/color.h>

#include <cstdint>

 
class CImageInfo
{
public:
	 
	enum EImageFormat
	{
		FORMAT_UNDEFINED = -1,
		FORMAT_RGB = 0,
		FORMAT_RGBA = 1,
		FORMAT_R = 2,
		FORMAT_RA = 3,
	};

	CImageInfo() = default;

	 
	CImageInfo &operator=(CImageInfo &&Other);

	 
	CImageInfo(CImageInfo &&Other);

	 
	size_t m_Width = 0;

	 
	size_t m_Height = 0;

	 
	EImageFormat m_Format = FORMAT_UNDEFINED;

	 
	uint8_t *m_pData = nullptr;

	 
	void Free();

	 
	static size_t PixelSize(EImageFormat Format);

	 
	static const char *FormatName(EImageFormat Format);

	 
	size_t PixelSize() const;

	 
	const char *FormatName() const;

	 
	size_t DataSize() const;

	 
	bool DataEquals(const CImageInfo &Other) const;

	 
	ColorRGBA PixelColor(size_t x, size_t y) const;

	 
	void SetPixelColor(size_t x, size_t y, ColorRGBA Color) const;

	 
	void CopyRectFrom(const CImageInfo &SrcImage, size_t SrcX, size_t SrcY, size_t Width, size_t Height, size_t DestX, size_t DestY) const;

	 
	CImageInfo DeepCopy() const;
};

#endif
