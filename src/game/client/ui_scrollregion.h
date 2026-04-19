 
 
#ifndef GAME_CLIENT_UI_SCROLLREGION_H
#define GAME_CLIENT_UI_SCROLLREGION_H

#include "ui.h"

struct CScrollRegionParams
{
	float m_ScrollbarWidth;
	float m_ScrollbarMargin;
	bool m_ScrollbarNoMarginRight;
	float m_SliderMinHeight;
	float m_ScrollUnit;
	ColorRGBA m_ClipBgColor;
	ColorRGBA m_ScrollbarBgColor;
	ColorRGBA m_RailBgColor;
	ColorRGBA m_SliderColor;
	ColorRGBA m_SliderColorHover;
	ColorRGBA m_SliderColorGrabbed;
	unsigned m_Flags;

	enum
	{
		FLAG_CONTENT_STATIC_WIDTH = 1 << 0,
	};

	CScrollRegionParams()
	{
		m_ScrollbarWidth = 20.0f;
		m_ScrollbarMargin = 5.0f;
		m_ScrollbarNoMarginRight = false;
		m_SliderMinHeight = 25.0f;
		m_ScrollUnit = 10.0f;
		m_ClipBgColor = ColorRGBA(0.0f, 0.0f, 0.0f, 0.0f);
		m_ScrollbarBgColor = ColorRGBA(0.0f, 0.0f, 0.0f, 0.0f);
		m_RailBgColor = ColorRGBA(1.0f, 1.0f, 1.0f, 0.25f);
		m_SliderColor = ColorRGBA(0.8f, 0.8f, 0.8f, 1.0f);
		m_SliderColorHover = ColorRGBA(1.0f, 1.0f, 1.0f, 1.0f);
		m_SliderColorGrabbed = ColorRGBA(0.9f, 0.9f, 0.9f, 1.0f);
		m_Flags = 0;
	}

	ColorRGBA SliderColor(bool Active, bool Hovered) const
	{
		if(Active)
			return m_SliderColorGrabbed;
		else if(Hovered)
			return m_SliderColorHover;
		return m_SliderColor;
	}
};

 

 
class CScrollRegion : private CUIElementBase
{
public:
	 
	 
	static constexpr float HEIGHT_MAGIC_FIX = 1.0f;

	enum EScrollRelative
	{
		SCROLLRELATIVE_UP = -1,
		SCROLLRELATIVE_NONE = 0,
		SCROLLRELATIVE_DOWN = 1,
	};

private:
	float m_ScrollY;
	float m_ContentH;
	float m_RequestScrollY;  
	EScrollRelative m_ScrollDirection;
	float m_ScrollSpeedMultiplier;

	float m_AnimTimeMax;
	float m_AnimTime;
	float m_AnimInitScrollY;
	float m_AnimTargetScrollY;

	CUIRect m_ClipRect;
	CUIRect m_RailRect;
	CUIRect m_LastAddedRect;  
	float m_SliderGrabPos;  
	vec2 m_ContentScrollOff;
	CScrollRegionParams m_Params;

public:
	enum EScrollOption
	{
		SCROLLHERE_KEEP_IN_VIEW = 0,
		SCROLLHERE_TOP,
		SCROLLHERE_BOTTOM,
	};

	CScrollRegion();
	void Reset();

	void Begin(CUIRect *pClipRect, vec2 *pOutOffset, const CScrollRegionParams *pParams = nullptr);
	void End();
	bool AddRect(const CUIRect &Rect, bool ShouldScrollHere = false);  
	void ScrollHere(EScrollOption Option = SCROLLHERE_KEEP_IN_VIEW);
	void ScrollRelative(EScrollRelative Direction, float SpeedMultiplier = 1.0f);
	void ScrollRelativeDirect(float ScrollAmount);
	const CUIRect *ClipRect() const { return &m_ClipRect; }
	void DoEdgeScrolling();
	bool RectClipped(const CUIRect &Rect) const;
	bool ScrollbarShown() const;
	bool Animating() const;
	bool Active() const;
	const CScrollRegionParams &Params() const { return m_Params; }
};

#endif
