 
 
#ifndef GAME_CLIENT_UI_RECT_H
#define GAME_CLIENT_UI_RECT_H

#include <base/color.h>
#include <base/vmath.h>

class IGraphics;

class CUIRect
{
	static IGraphics *ms_pGraphics;

public:
	static void Init(IGraphics *pGraphics) { ms_pGraphics = pGraphics; }

	float x, y, w, h;

	 
	void HSplitMid(CUIRect *pTop, CUIRect *pBottom, float Spacing = 0.0f) const;
	 
	void HSplitTop(float Cut, CUIRect *pTop, CUIRect *pBottom) const;
	 
	void HSplitBottom(float Cut, CUIRect *pTop, CUIRect *pBottom) const;
	 
	void VSplitMid(CUIRect *pLeft, CUIRect *pRight, float Spacing = 0.0f) const;
	 
	void VSplitLeft(float Cut, CUIRect *pLeft, CUIRect *pRight) const;
	 
	void VSplitRight(float Cut, CUIRect *pLeft, CUIRect *pRight) const;

	 
	void Margin(vec2 Cut, CUIRect *pOtherRect) const;
	 
	void Margin(float Cut, CUIRect *pOtherRect) const;
	 
	void VMargin(float Cut, CUIRect *pOtherRect) const;
	 
	void HMargin(float Cut, CUIRect *pOtherRect) const;

	 
	bool Inside(vec2 Point) const;

	 
	void Draw(ColorRGBA Color, int Corners, float Rounding) const;
	void Draw4(ColorRGBA ColorTopLeft, ColorRGBA ColorTopRight, ColorRGBA ColorBottomLeft, ColorRGBA ColorBottomRight, int Corners, float Rounding) const;

	 
	void DrawOutline(ColorRGBA Color) const;

	 
	vec2 TopLeft() const { return vec2(x, y); }
	 
	vec2 Size() const { return vec2(w, h); }
	 
	vec2 Center() const { return TopLeft() + Size() / 2.0f; }
};

#endif
