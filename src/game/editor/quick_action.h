#ifndef GAME_EDITOR_QUICK_ACTION_H
#define GAME_EDITOR_QUICK_ACTION_H

#include <functional>
#include <utility>

typedef std::function<void()> FButtonClickCallback;
typedef std::function<bool()> FButtonDisabledCallback;
typedef std::function<bool()> FButtonActiveCallback;
typedef std::function<int()> FButtonColorCallback;

class CQuickAction
{
private:
	const char *m_pLabel;
	const char *m_pDescription;

	FButtonClickCallback m_pfnCallback;
	FButtonDisabledCallback m_pfnDisabledCallback;
	FButtonActiveCallback m_pfnActiveCallback;
	FButtonColorCallback m_pfnColorCallback;

	const char m_ActionButtonId = 0;

public:
	CQuickAction(
		const char *pLabel,
		const char *pDescription,
		FButtonClickCallback pfnCallback,
		FButtonDisabledCallback pfnDisabledCallback,
		FButtonActiveCallback pfnActiveCallback,
		FButtonColorCallback pfnColorCallback) :
		m_pLabel(pLabel),
		m_pDescription(pDescription),
		m_pfnCallback(std::move(pfnCallback)),
		m_pfnDisabledCallback(std::move(pfnDisabledCallback)),
		m_pfnActiveCallback(std::move(pfnActiveCallback)),
		m_pfnColorCallback(std::move(pfnColorCallback))
	{
	}

	 
	void Call() const { m_pfnCallback(); }

	 
	bool Disabled() { return m_pfnDisabledCallback(); }

	 
	 
	bool Active() { return m_pfnActiveCallback(); }

	 
	 
	int Color() { return m_pfnColorCallback(); }

	const char *Label() const { return m_pLabel; }

	 
	 
	const char *LabelShort() const
	{
		const char *pShort = str_find(m_pLabel, ": ");
		if(!pShort)
			return m_pLabel;
		return pShort + 2;
	}

	const char *Description() const { return m_pDescription; }

	const void *ActionButtonId() const { return &m_ActionButtonId; }
};

#endif
