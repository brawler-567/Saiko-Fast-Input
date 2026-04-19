#ifndef GAME_EDITOR_COMPONENT_H
#define GAME_EDITOR_COMPONENT_H

#include <engine/input.h>

#include <game/client/ui_rect.h>
#include <game/editor/editor_object.h>

#include <functional>
#include <vector>

class CEditorComponent : public CEditorObject
{
public:
	 
	void OnInit(CEditor *pEditor) override;

	virtual void OnReset();

	virtual void OnMapLoad();

	 
	virtual bool OnInput(const IInput::CEvent &Event);

	virtual void OnUpdate();

	virtual void OnRender(CUIRect View);

	 
	void InitSubComponents();

	 
	void RegisterSubComponent(CEditorComponent &Component);

private:
	std::vector<std::reference_wrapper<CEditorComponent>> m_vSubComponents;
};

#endif
