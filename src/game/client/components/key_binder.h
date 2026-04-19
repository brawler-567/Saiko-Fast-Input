 
 
#ifndef GAME_CLIENT_COMPONENTS_KEY_BINDER_H
#define GAME_CLIENT_COMPONENTS_KEY_BINDER_H

#include <game/client/component.h>
#include <game/client/components/binds.h>
#include <game/client/ui.h>

 
class CKeyBinder : public CComponent
{
public:
	int Sizeof() const override { return sizeof(*this); }
	bool OnInput(const IInput::CEvent &Event) override;

	class CKeyReaderResult
	{
	public:
		CBindSlot m_Bind;
		bool m_Aborted;
	};
	CKeyReaderResult DoKeyReader(CButtonContainer *pReaderButton, CButtonContainer *pClearButton, const CUIRect *pRect, const CBindSlot &CurrentBind, bool Activate);
	bool IsActive() const;

private:
	const CButtonContainer *m_pKeyReaderId = nullptr;
	bool m_TakeKey = false;
	std::optional<CBindSlot> m_Key;
};

#endif
