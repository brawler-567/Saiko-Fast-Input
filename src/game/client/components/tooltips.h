#ifndef GAME_CLIENT_COMPONENTS_TOOLTIPS_H
#define GAME_CLIENT_COMPONENTS_TOOLTIPS_H

#include <game/client/component.h>
#include <game/client/ui_rect.h>

#include <cstdint>
#include <functional>
#include <optional>
#include <unordered_map>

struct CTooltip
{
	const void *m_pId;
	CUIRect m_Rect;
	const char *m_pText;
	float m_WidthHint;
	bool m_OnScreen;  
	float m_FadeTime = 0.75f;
};

 
class CTooltips : public CComponent
{
	std::unordered_map<uintptr_t, CTooltip> m_Tooltips;
	std::optional<std::reference_wrapper<CTooltip>> m_ActiveTooltip;
	std::optional<std::reference_wrapper<CTooltip>> m_PreviousTooltip;
	int64_t m_HoverTime;

	 
	void SetActiveTooltip(CTooltip &Tooltip);

	inline void ClearActiveTooltip();

public:
	CTooltips();
	int Sizeof() const override { return sizeof(*this); }

	 
	void DoToolTip(const void *pId, const CUIRect *pNearRect, const char *pText, float WidthHint = -1.0f);

	void OnReset() override;
	void OnRender() override;

	 
	void SetFadeTime(const void *pId, float Time);
};

#endif
