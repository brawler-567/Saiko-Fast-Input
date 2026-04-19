#include "editor_server_settings.h"

#include "editor.h"

#include <base/color.h>
#include <base/system.h>

#include <engine/keys.h>
#include <engine/shared/config.h>
#include <engine/textrender.h>

#include <game/client/gameclient.h>
#include <game/client/lineinput.h>
#include <game/client/ui.h>
#include <game/client/ui_listbox.h>
#include <game/editor/editor_actions.h>
#include <game/editor/editor_history.h>

#include <iterator>

using namespace FontIcons;

static const int FONT_SIZE = 12.0f;

struct IMapSetting
{
	enum EType
	{
		SETTING_INT,
		SETTING_COMMAND,
	};
	const char *m_pName;
	const char *m_pHelp;
	EType m_Type;

	IMapSetting(const char *pName, const char *pHelp, EType Type) :
		m_pName(pName), m_pHelp(pHelp), m_Type(Type) {}
};
struct SMapSettingInt : public IMapSetting
{
	int m_Default;
	int m_Min;
	int m_Max;

	SMapSettingInt(const char *pName, const char *pHelp, int Default, int Min, int Max) :
		IMapSetting(pName, pHelp, IMapSetting::SETTING_INT), m_Default(Default), m_Min(Min), m_Max(Max) {}
};
struct SMapSettingCommand : public IMapSetting
{
	const char *m_pArgs;

	SMapSettingCommand(const char *pName, const char *pHelp, const char *pArgs) :
		IMapSetting(pName, pHelp, IMapSetting::SETTING_COMMAND), m_pArgs(pArgs) {}
};

void CEditor::RenderServerSettingsEditor(CUIRect View, bool ShowServerSettingsEditorLast)
{
	static int s_CommandSelectedIndex = -1;
	static CListBox s_ListBox;
	s_ListBox.SetActive(!m_MapSettingsCommandContext.m_DropdownContext.m_ListBox.Active() && m_Dialog == DIALOG_NONE && !Ui()->IsPopupOpen());

	bool GotSelection = s_ListBox.Active() && s_CommandSelectedIndex >= 0 && (size_t)s_CommandSelectedIndex < Map()->m_vSettings.size();
	const bool CurrentInputValid = m_MapSettingsCommandContext.Valid();  

	CUIRect ToolBar, Button, Label, List, DragBar;
	View.HSplitTop(22.0f, &DragBar, nullptr);
	DragBar.y -= 2.0f;
	DragBar.w += 2.0f;
	DragBar.h += 4.0f;
	DoEditorDragBar(View, &DragBar, EDragSide::SIDE_TOP, &m_aExtraEditorSplits[EXTRAEDITOR_SERVER_SETTINGS]);
	View.HSplitTop(20.0f, &ToolBar, &View);
	View.HSplitTop(2.0f, nullptr, &List);
	ToolBar.HMargin(2.0f, &ToolBar);

	 
	ToolBar.VSplitRight(25.0f, &ToolBar, &Button);
	ToolBar.VSplitRight(5.0f, &ToolBar, nullptr);
	static int s_DeleteButton = 0;
	if(DoButton_FontIcon(&s_DeleteButton, FONT_ICON_TRASH, GotSelection ? 0 : -1, &Button, BUTTONFLAG_LEFT, "[Delete] Delete the selected command from the command list.", IGraphics::CORNER_ALL, 9.0f) || (GotSelection && CLineInput::GetActiveInput() == nullptr && m_Dialog == DIALOG_NONE && Ui()->ConsumeHotkey(CUi::HOTKEY_DELETE)))
	{
		Map()->m_ServerSettingsHistory.RecordAction(std::make_shared<CEditorCommandAction>(Map(), CEditorCommandAction::EType::DELETE, &s_CommandSelectedIndex, s_CommandSelectedIndex, Map()->m_vSettings[s_CommandSelectedIndex].m_aCommand));

		Map()->m_vSettings.erase(Map()->m_vSettings.begin() + s_CommandSelectedIndex);
		if(s_CommandSelectedIndex >= (int)Map()->m_vSettings.size())
			s_CommandSelectedIndex = Map()->m_vSettings.size() - 1;
		if(s_CommandSelectedIndex >= 0)
			m_SettingsCommandInput.Set(Map()->m_vSettings[s_CommandSelectedIndex].m_aCommand);
		else
			m_SettingsCommandInput.Clear();
		Map()->OnModify();
		m_MapSettingsCommandContext.Update();
		s_ListBox.ScrollToSelected();
	}

	 
	ToolBar.VSplitRight(25.0f, &ToolBar, &Button);
	const bool CanMoveDown = GotSelection && s_CommandSelectedIndex < (int)Map()->m_vSettings.size() - 1;
	static int s_DownButton = 0;
	if(DoButton_FontIcon(&s_DownButton, FONT_ICON_SORT_DOWN, CanMoveDown ? 0 : -1, &Button, BUTTONFLAG_LEFT, "[Alt+Down] Move the selected command down.", IGraphics::CORNER_R, 11.0f) || (CanMoveDown && Input()->AltIsPressed() && Ui()->ConsumeHotkey(CUi::HOTKEY_DOWN)))
	{
		Map()->m_ServerSettingsHistory.RecordAction(std::make_shared<CEditorCommandAction>(Map(), CEditorCommandAction::EType::MOVE_DOWN, &s_CommandSelectedIndex, s_CommandSelectedIndex));

		std::swap(Map()->m_vSettings[s_CommandSelectedIndex], Map()->m_vSettings[s_CommandSelectedIndex + 1]);
		s_CommandSelectedIndex++;
		Map()->OnModify();
		s_ListBox.ScrollToSelected();
	}

	 
	ToolBar.VSplitRight(25.0f, &ToolBar, &Button);
	ToolBar.VSplitRight(5.0f, &ToolBar, nullptr);
	const bool CanMoveUp = GotSelection && s_CommandSelectedIndex > 0;
	static int s_UpButton = 0;
	if(DoButton_FontIcon(&s_UpButton, FONT_ICON_SORT_UP, CanMoveUp ? 0 : -1, &Button, BUTTONFLAG_LEFT, "[Alt+Up] Move the selected command up.", IGraphics::CORNER_L, 11.0f) || (CanMoveUp && Input()->AltIsPressed() && Ui()->ConsumeHotkey(CUi::HOTKEY_UP)))
	{
		Map()->m_ServerSettingsHistory.RecordAction(std::make_shared<CEditorCommandAction>(Map(), CEditorCommandAction::EType::MOVE_UP, &s_CommandSelectedIndex, s_CommandSelectedIndex));

		std::swap(Map()->m_vSettings[s_CommandSelectedIndex], Map()->m_vSettings[s_CommandSelectedIndex - 1]);
		s_CommandSelectedIndex--;
		Map()->OnModify();
		s_ListBox.ScrollToSelected();
	}

	 
	ToolBar.VSplitRight(25.0f, &ToolBar, &Button);
	static int s_RedoButton = 0;
	if(DoButton_FontIcon(&s_RedoButton, FONT_ICON_REDO, Map()->m_ServerSettingsHistory.CanRedo() ? 0 : -1, &Button, BUTTONFLAG_LEFT, "[Ctrl+Y] Redo the last command edit.", IGraphics::CORNER_R, 11.0f))
	{
		Map()->m_ServerSettingsHistory.Redo();
	}

	 
	ToolBar.VSplitRight(25.0f, &ToolBar, &Button);
	ToolBar.VSplitRight(5.0f, &ToolBar, nullptr);
	static int s_UndoButton = 0;
	if(DoButton_FontIcon(&s_UndoButton, FONT_ICON_UNDO, Map()->m_ServerSettingsHistory.CanUndo() ? 0 : -1, &Button, BUTTONFLAG_LEFT, "[Ctrl+Z] Undo the last command edit.", IGraphics::CORNER_L, 11.0f))
	{
		Map()->m_ServerSettingsHistory.Undo();
	}

	GotSelection = s_ListBox.Active() && s_CommandSelectedIndex >= 0 && (size_t)s_CommandSelectedIndex < Map()->m_vSettings.size();

	int CollidingCommandIndex = -1;
	ECollisionCheckResult CheckResult = ECollisionCheckResult::ERROR;
	if(CurrentInputValid)
		CollidingCommandIndex = m_MapSettingsCommandContext.CheckCollision(CheckResult);

	 
	ToolBar.VSplitRight(25.0f, &ToolBar, &Button);
	const bool CanAdd = CheckResult == ECollisionCheckResult::ADD;
	const bool CanReplace = CheckResult == ECollisionCheckResult::REPLACE;

	const bool CanUpdate = GotSelection && CurrentInputValid && str_comp(Map()->m_vSettings[s_CommandSelectedIndex].m_aCommand, m_SettingsCommandInput.GetString()) != 0;

	static int s_UpdateButton = 0;
	if(DoButton_FontIcon(&s_UpdateButton, FONT_ICON_PENCIL, CanUpdate ? 0 : -1, &Button, BUTTONFLAG_LEFT, "[Alt+Enter] Update the selected command based on the entered value.", IGraphics::CORNER_R, 9.0f) || (CanUpdate && Input()->AltIsPressed() && m_Dialog == DIALOG_NONE && m_SettingsCommandInput.IsActive() && Ui()->ConsumeHotkey(CUi::HOTKEY_ENTER)))
	{
		if(CollidingCommandIndex == -1)
		{
			bool Found = false;
			int i;
			for(i = 0; i < (int)Map()->m_vSettings.size(); ++i)
			{
				if(i != s_CommandSelectedIndex && !str_comp(Map()->m_vSettings[i].m_aCommand, m_SettingsCommandInput.GetString()))
				{
					Found = true;
					break;
				}
			}
			if(Found)
			{
				Map()->m_ServerSettingsHistory.RecordAction(std::make_shared<CEditorCommandAction>(Map(), CEditorCommandAction::EType::DELETE, &s_CommandSelectedIndex, s_CommandSelectedIndex, Map()->m_vSettings[s_CommandSelectedIndex].m_aCommand));
				Map()->m_vSettings.erase(Map()->m_vSettings.begin() + s_CommandSelectedIndex);
				s_CommandSelectedIndex = i > s_CommandSelectedIndex ? i - 1 : i;
			}
			else
			{
				const char *pStr = m_SettingsCommandInput.GetString();
				Map()->m_ServerSettingsHistory.RecordAction(std::make_shared<CEditorCommandAction>(Map(), CEditorCommandAction::EType::EDIT, &s_CommandSelectedIndex, s_CommandSelectedIndex, Map()->m_vSettings[s_CommandSelectedIndex].m_aCommand, pStr));
				str_copy(Map()->m_vSettings[s_CommandSelectedIndex].m_aCommand, pStr);
			}
		}
		else
		{
			if(s_CommandSelectedIndex == CollidingCommandIndex)
			{  
				const char *pStr = m_SettingsCommandInput.GetString();
				Map()->m_ServerSettingsHistory.RecordAction(std::make_shared<CEditorCommandAction>(Map(), CEditorCommandAction::EType::EDIT, &s_CommandSelectedIndex, s_CommandSelectedIndex, Map()->m_vSettings[s_CommandSelectedIndex].m_aCommand, pStr));
				str_copy(Map()->m_vSettings[s_CommandSelectedIndex].m_aCommand, pStr);
			}
			else
			{  
				const char *pStr = m_SettingsCommandInput.GetString();

				char aBuf[256];
				str_format(aBuf, sizeof(aBuf), "Delete command %d; Edit command %d", CollidingCommandIndex, s_CommandSelectedIndex);

				Map()->m_ServerSettingsHistory.BeginBulk();
				 
				Map()->m_ServerSettingsHistory.RecordAction(std::make_shared<CEditorCommandAction>(Map(), CEditorCommandAction::EType::DELETE, &s_CommandSelectedIndex, CollidingCommandIndex, Map()->m_vSettings[CollidingCommandIndex].m_aCommand));
				Map()->m_vSettings.erase(Map()->m_vSettings.begin() + CollidingCommandIndex);
				 
				s_CommandSelectedIndex = s_CommandSelectedIndex > CollidingCommandIndex ? s_CommandSelectedIndex - 1 : s_CommandSelectedIndex;
				Map()->m_ServerSettingsHistory.RecordAction(std::make_shared<CEditorCommandAction>(Map(), CEditorCommandAction::EType::EDIT, &s_CommandSelectedIndex, s_CommandSelectedIndex, Map()->m_vSettings[s_CommandSelectedIndex].m_aCommand, pStr));
				str_copy(Map()->m_vSettings[s_CommandSelectedIndex].m_aCommand, pStr);

				Map()->m_ServerSettingsHistory.EndBulk(aBuf);
			}
		}

		Map()->OnModify();
		s_ListBox.ScrollToSelected();
		m_SettingsCommandInput.Clear();
		m_MapSettingsCommandContext.Reset();  
		Ui()->SetActiveItem(&m_SettingsCommandInput);
	}

	 
	ToolBar.VSplitRight(25.0f, &ToolBar, &Button);
	ToolBar.VSplitRight(100.0f, &ToolBar, nullptr);

	static int s_AddButton = 0;
	if(DoButton_FontIcon(&s_AddButton, CanReplace ? FONT_ICON_ARROWS_ROTATE : FONT_ICON_PLUS, CanAdd || CanReplace ? 0 : -1, &Button, BUTTONFLAG_LEFT, CanReplace ? "[Enter] Replace the corresponding command in the command list." : "[Enter] Add a command to the command list.", IGraphics::CORNER_L) || ((CanAdd || CanReplace) && !Input()->AltIsPressed() && m_Dialog == DIALOG_NONE && m_SettingsCommandInput.IsActive() && Ui()->ConsumeHotkey(CUi::HOTKEY_ENTER)))
	{
		if(CanReplace)
		{
			dbg_assert(CollidingCommandIndex != -1, "Could not replace command");
			s_CommandSelectedIndex = CollidingCommandIndex;

			const char *pStr = m_SettingsCommandInput.GetString();
			Map()->m_ServerSettingsHistory.RecordAction(std::make_shared<CEditorCommandAction>(Map(), CEditorCommandAction::EType::EDIT, &s_CommandSelectedIndex, s_CommandSelectedIndex, Map()->m_vSettings[s_CommandSelectedIndex].m_aCommand, pStr));
			str_copy(Map()->m_vSettings[s_CommandSelectedIndex].m_aCommand, pStr);
		}
		else if(CanAdd)
		{
			Map()->m_vSettings.emplace_back(m_SettingsCommandInput.GetString());
			s_CommandSelectedIndex = Map()->m_vSettings.size() - 1;
			Map()->m_ServerSettingsHistory.RecordAction(std::make_shared<CEditorCommandAction>(Map(), CEditorCommandAction::EType::ADD, &s_CommandSelectedIndex, s_CommandSelectedIndex, Map()->m_vSettings[s_CommandSelectedIndex].m_aCommand));
		}

		Map()->OnModify();
		s_ListBox.ScrollToSelected();
		m_SettingsCommandInput.Clear();
		m_MapSettingsCommandContext.Reset();  
		Ui()->SetActiveItem(&m_SettingsCommandInput);
	}

	 
	if(!ShowServerSettingsEditorLast)  
		Ui()->SetActiveItem(&m_SettingsCommandInput);
	m_SettingsCommandInput.SetEmptyText("Command");

	TextRender()->TextColor(TextRender()->DefaultTextColor());

	 
	s_ListBox.DoStart(15.0f, Map()->m_vSettings.size(), 1, 3, s_CommandSelectedIndex, &List);

	for(size_t i = 0; i < Map()->m_vSettings.size(); i++)
	{
		const CListboxItem Item = s_ListBox.DoNextItem(&Map()->m_vSettings[i], s_CommandSelectedIndex >= 0 && (size_t)s_CommandSelectedIndex == i);
		if(!Item.m_Visible)
			continue;

		Item.m_Rect.VMargin(5.0f, &Label);

		SLabelProperties Props;
		Props.m_MaxWidth = Label.w;
		Props.m_EllipsisAtEnd = true;
		Ui()->DoLabel(&Label, Map()->m_vSettings[i].m_aCommand, 10.0f, TEXTALIGN_ML, Props);
	}

	const int NewSelected = s_ListBox.DoEnd();
	if(s_CommandSelectedIndex != NewSelected || s_ListBox.WasItemSelected())
	{
		s_CommandSelectedIndex = NewSelected;
		if(m_SettingsCommandInput.IsEmpty() || !Input()->ModifierIsPressed())  
		{
			m_SettingsCommandInput.Set(Map()->m_vSettings[s_CommandSelectedIndex].m_aCommand);
			m_MapSettingsCommandContext.Update();
			m_MapSettingsCommandContext.UpdateCursor(true);
		}
		m_MapSettingsCommandContext.m_DropdownContext.m_ShouldHide = true;
		Ui()->SetActiveItem(&m_SettingsCommandInput);
	}

	 
	DoMapSettingsEditBox(&m_MapSettingsCommandContext, &ToolBar, FONT_SIZE, List.h);
}

void CEditor::DoMapSettingsEditBox(CMapSettingsBackend::CContext *pContext, const CUIRect *pRect, float FontSize, float DropdownMaxHeight, int Corners, const char *pToolTip)
{
	 

	auto *pLineInput = pContext->LineInput();
	auto &Context = *pContext;
	Context.SetFontSize(FontSize);

	 
	 
	const float PartMargin = 4.0f;
	auto &&RenderFloatingPart = [&](CUIRect *pInputRect, float x, const char *pStr) {
		CUIRect Background;
		Background.x = x - PartMargin;
		Background.y = pInputRect->y - pInputRect->h - 6.0f;
		Background.w = TextRender()->TextWidth(FontSize, pStr) + 2 * PartMargin;
		Background.h = pInputRect->h;
		Background.Draw(ColorRGBA(0, 0, 0, 0.9f), IGraphics::CORNER_ALL, 3.0f);

		CUIRect Label;
		Background.VSplitLeft(PartMargin, nullptr, &Label);
		TextRender()->TextColor(0.8f, 0.8f, 0.8f, 1.0f);
		Ui()->DoLabel(&Label, pStr, FontSize, TEXTALIGN_ML);
		TextRender()->TextColor(TextRender()->DefaultTextColor());
	};

	 
	if(Context.CommandIsValid() && pLineInput->IsActive() && Ui()->HotItem() == nullptr)
	{
		Context.GetCommandHelpText(m_aTooltip, sizeof(m_aTooltip));
		str_append(m_aTooltip, ".");
	}

	CUIRect ToolBar = *pRect;
	CUIRect Button;
	ToolBar.VSplitRight(ToolBar.h, &ToolBar, &Button);

	 
	if(DoButton_FontIcon(&Context.m_AllowUnknownCommands, FONT_ICON_QUESTION, Context.m_AllowUnknownCommands, &Button, BUTTONFLAG_LEFT, "Disallow/allow unknown or invalid commands.", IGraphics::CORNER_R))
	{
		Context.m_AllowUnknownCommands = !Context.m_AllowUnknownCommands;
		Context.Update();
	}

	 
	std::vector<STextColorSplit> vColorSplits;
	Context.ColorArguments(vColorSplits);

	 
	if(DoClearableEditBox(pLineInput, &ToolBar, FontSize, IGraphics::CORNER_L, "Enter a server setting. Press ctrl+space to show available settings.", vColorSplits))
	{
		Context.Update();  
		Context.m_DropdownContext.m_ShouldHide = false;
	}

	 
	if(Context.UpdateCursor())
		Context.m_DropdownContext.m_ShouldHide = false;

	 
	float x = ToolBar.x + Context.CurrentArgPos() - pLineInput->GetScrollOffset();
	x = std::clamp(x, ToolBar.x + PartMargin, ToolBar.x + ToolBar.w);

	if(pLineInput->IsActive())
	{
		 
		 
		const bool IsAtEnd = pLineInput->GetCursorOffset() >= (m_MapSettingsCommandContext.CommentOffset() != -1 ? m_MapSettingsCommandContext.CommentOffset() : pLineInput->GetLength());

		if(Context.CurrentArgName() && (!Context.HasError() || !IsAtEnd))  
			RenderFloatingPart(&ToolBar, x, Context.CurrentArgName());
		else if(Context.HasError() && IsAtEnd)  
			RenderFloatingPart(&ToolBar, ToolBar.x + PartMargin, Context.Error());
	}

	 
	const auto &vPossibleCommands = Context.PossibleMatches();
	int Selected = DoEditBoxDropdown<SPossibleValueMatch>(&Context.m_DropdownContext, pLineInput, &ToolBar, x - PartMargin, DropdownMaxHeight, Context.CurrentArg() >= 0, vPossibleCommands, MapSettingsDropdownRenderCallback);

	 
	 
	if(Context.m_DropdownContext.m_DidBecomeVisible)
	{
		Context.Update();
		Context.UpdateCursor(true);
	}

	if(!vPossibleCommands.empty())
	{
		 
		if(Selected != pContext->m_CurrentCompletionIndex)
		{
			 
			if(Selected != -1)
			{
				const char *pStr = vPossibleCommands[Selected].m_pValue;
				int Len = pContext->m_CurrentCompletionIndex == -1 ? str_length(Context.CurrentArgValue()) : (pContext->m_CurrentCompletionIndex < (int)vPossibleCommands.size() ? str_length(vPossibleCommands[pContext->m_CurrentCompletionIndex].m_pValue) : 0);
				size_t Start = Context.CurrentArgOffset();
				size_t End = Start + Len;
				pLineInput->SetRange(pStr, Start, End);
			}

			pContext->m_CurrentCompletionIndex = Selected;
		}
	}
	else
	{
		Context.m_DropdownContext.m_ListBox.SetActive(false);
	}
}

template<typename T>
int CEditor::DoEditBoxDropdown(SEditBoxDropdownContext *pDropdown, CLineInput *pLineInput, const CUIRect *pEditBoxRect, int x, float MaxHeight, bool AutoWidth, const std::vector<T> &vData, const FDropdownRenderCallback<T> &pfnMatchCallback)
{
	 
	 

	pDropdown->m_Selected = std::clamp(pDropdown->m_Selected, -1, (int)vData.size() - 1);

	if(Input()->KeyPress(KEY_SPACE) && Input()->ModifierIsPressed())
	{  
		pDropdown->m_ShortcutUsed = true;
		 
		pLineInput->SetRange("", pLineInput->GetCursorOffset() - 1, pLineInput->GetCursorOffset());
	}

	if((!pDropdown->m_ShouldHide && !pLineInput->IsEmpty() && (pLineInput->IsActive() || pDropdown->m_MousePressedInside)) || pDropdown->m_ShortcutUsed)
	{
		if(!pDropdown->m_Visible)
		{
			pDropdown->m_DidBecomeVisible = true;
			pDropdown->m_Visible = true;
		}
		else if(pDropdown->m_DidBecomeVisible)
			pDropdown->m_DidBecomeVisible = false;

		if(!pLineInput->IsEmpty() || !pLineInput->IsActive())
			pDropdown->m_ShortcutUsed = false;

		int CurrentSelected = pDropdown->m_Selected;

		 
		if(Ui()->ConsumeHotkey(CUi::HOTKEY_TAB) && !vData.empty())
		{
			int Direction = Input()->ShiftIsPressed() ? -1 : 1;

			pDropdown->m_Selected += Direction;
			if(pDropdown->m_Selected < 0)
				pDropdown->m_Selected = (int)vData.size() - 1;
			pDropdown->m_Selected %= vData.size();
		}

		int Selected = RenderEditBoxDropdown<T>(pDropdown, *pEditBoxRect, pLineInput, x, MaxHeight, AutoWidth, vData, pfnMatchCallback);
		if(Selected != -1)
			pDropdown->m_Selected = Selected;

		if(CurrentSelected != pDropdown->m_Selected)
			pDropdown->m_ListBox.ScrollToSelected();

		return pDropdown->m_Selected;
	}
	else
	{
		pDropdown->m_ShortcutUsed = false;
		pDropdown->m_Visible = false;
		pDropdown->m_ListBox.SetActive(false);
		pDropdown->m_Selected = -1;
	}

	return -1;
}

template<typename T>
int CEditor::RenderEditBoxDropdown(SEditBoxDropdownContext *pDropdown, CUIRect View, CLineInput *pLineInput, int x, float MaxHeight, bool AutoWidth, const std::vector<T> &vData, const FDropdownRenderCallback<T> &pfnMatchCallback)
{
	 
	auto *pListBox = &pDropdown->m_ListBox;

	pListBox->SetActive(m_Dialog == DIALOG_NONE && !Ui()->IsPopupOpen() && pLineInput->IsActive());
	pListBox->SetScrollbarWidth(15.0f);

	const int NumEntries = vData.size();

	 
	CUIRect CommandsDropdown = View;
	CommandsDropdown.y += View.h + 0.1f;
	CommandsDropdown.x = x;
	if(AutoWidth)
		CommandsDropdown.w = pDropdown->m_Width + pListBox->ScrollbarWidth();

	pListBox->SetActive(NumEntries > 0);
	if(NumEntries > 0)
	{
		 
		CommandsDropdown.h = minimum(NumEntries * 15.0f + 1.0f, MaxHeight);
		CommandsDropdown.Draw(ColorRGBA(0.1f, 0.1f, 0.1f, 0.9f), IGraphics::CORNER_ALL, 3.0f);

		if(Ui()->MouseButton(0) && Ui()->MouseInside(&CommandsDropdown))
			pDropdown->m_MousePressedInside = true;

		 
		int Selected = pDropdown->m_Selected;
		pListBox->DoStart(15.0f, NumEntries, 1, 3, Selected, &CommandsDropdown);
		CUIRect Label;

		int NewIndex = Selected;
		float LargestWidth = 0;
		for(int i = 0; i < NumEntries; i++)
		{
			const CListboxItem Item = pListBox->DoNextItem(&vData[i], Selected == i);

			Item.m_Rect.VMargin(4.0f, &Label);

			SLabelProperties Props;
			Props.m_MaxWidth = Label.w;
			Props.m_EllipsisAtEnd = true;

			 
			char aBuf[128];
			pfnMatchCallback(vData.at(i), aBuf, Props.m_vColorSplits);

			LargestWidth = maximum(LargestWidth, TextRender()->TextWidth(12.0f, aBuf) + 10.0f);
			if(!Item.m_Visible)
				continue;

			Ui()->DoLabel(&Label, aBuf, 12.0f, TEXTALIGN_ML, Props);

			if(Ui()->ActiveItem() == &vData[i])
			{
				 
				 
				NewIndex = i;
				Ui()->SetActiveItem(pLineInput);
			}
		}

		pDropdown->m_Width = LargestWidth;

		int EndIndex = pListBox->DoEnd();
		if(NewIndex == Selected)
			NewIndex = EndIndex;

		if(pDropdown->m_MousePressedInside && !Ui()->MouseButton(0))
		{
			Ui()->SetActiveItem(pLineInput);
			pDropdown->m_MousePressedInside = false;
		}

		if(NewIndex != Selected)
		{
			Ui()->SetActiveItem(pLineInput);
			return NewIndex;
		}
	}
	return -1;
}

void CEditor::RenderMapSettingsErrorDialog()
{
	auto &LoadedMapSettings = m_MapSettingsBackend.m_LoadedMapSettings;
	auto &vSettingsInvalid = LoadedMapSettings.m_vSettingsInvalid;
	auto &vSettingsValid = LoadedMapSettings.m_vSettingsValid;
	auto &SettingsDuplicate = LoadedMapSettings.m_SettingsDuplicate;

	Ui()->MapScreen();
	CUIRect Overlay = *Ui()->Screen();

	Overlay.Draw(ColorRGBA(0, 0, 0, 0.33f), IGraphics::CORNER_NONE, 0.0f);
	CUIRect Background;
	Overlay.VMargin(150.0f, &Background);
	Background.HMargin(50.0f, &Background);
	Background.Draw(ColorRGBA(0, 0, 0, 0.80f), IGraphics::CORNER_ALL, 5.0f);

	CUIRect View;
	Background.Margin(10.0f, &View);

	CUIRect Title, ButtonBar, Label;
	View.HSplitTop(18.0f, &Title, &View);
	View.HSplitTop(5.0f, nullptr, &View);  
	View.HSplitBottom(18.0f, &View, &ButtonBar);
	View.HSplitBottom(10.0f, &View, nullptr);  

	 
	Title.Draw(ColorRGBA(1, 1, 1, 0.25f), IGraphics::CORNER_ALL, 4.0f);
	Title.VMargin(10.0f, &Title);
	Ui()->DoLabel(&Title, "Map settings error", 12.0f, TEXTALIGN_ML);

	 
	{
		static CLineInputBuffered<256> s_Input;
		static CMapSettingsBackend::CContext s_Context = m_MapSettingsBackend.NewContext(&s_Input);

		 
		SLabelProperties Props;
		CUIRect Text;
		View.HSplitTop(30.0f, &Text, &View);
		Props.m_MaxWidth = Text.w;
		Ui()->DoLabel(&Text, "Below is a report of the invalid map settings found when loading the map. Please fix them before proceeding further.", 10.0f, TEXTALIGN_MC, Props);

		 
		CUIRect List = View;
		View.Draw(ColorRGBA(1, 1, 1, 0.25f), IGraphics::CORNER_ALL, 3.0f);

		const float RowHeight = 18.0f;
		static CScrollRegion s_ScrollRegion;
		vec2 ScrollOffset(0.0f, 0.0f);
		CScrollRegionParams ScrollParams;
		ScrollParams.m_ScrollUnit = 120.0f;
		s_ScrollRegion.Begin(&List, &ScrollOffset, &ScrollParams);
		const float EndY = List.y + List.h;
		List.y += ScrollOffset.y;

		List.HSplitTop(20.0f, nullptr, &List);

		static int s_FixingCommandIndex = -1;

		auto &&SetInput = [&](const char *pString) {
			s_Input.Set(pString);
			s_Context.Update();
			s_Context.UpdateCursor(true);
			Ui()->SetActiveItem(&s_Input);
		};

		CUIRect FixInput;
		bool DisplayFixInput = false;
		float DropdownHeight = 110.0f;

		for(int i = 0; i < (int)Map()->m_vSettings.size(); i++)
		{
			CUIRect Slot;

			auto pInvalidSetting = std::find_if(vSettingsInvalid.begin(), vSettingsInvalid.end(), [i](const SInvalidSetting &Setting) { return Setting.m_Index == i; });
			if(pInvalidSetting != vSettingsInvalid.end())
			{  
				if(!(pInvalidSetting->m_Type & SInvalidSetting::TYPE_DUPLICATE))
				{
					bool IsFixing = s_FixingCommandIndex == i;
					List.HSplitTop(RowHeight, &Slot, &List);

					 
					if(pInvalidSetting->m_Context.m_Deleted)
						Slot.Draw(ColorRGBA(0.85f, 0.0f, 0.0f, 0.15f), IGraphics::CORNER_ALL, 3.0f);

					Slot.VMargin(5.0f, &Slot);
					Slot.HMargin(1.0f, &Slot);

					if(!IsFixing && !pInvalidSetting->m_Context.m_Fixed)
					{  
						CUIRect FixBtn, DelBtn;
						Slot.VSplitRight(30.0f, &Slot, &DelBtn);
						Slot.VSplitRight(5.0f, &Slot, nullptr);
						DelBtn.HMargin(1.0f, &DelBtn);

						Slot.VSplitRight(30.0f, &Slot, &FixBtn);
						Slot.VSplitRight(10.0f, &Slot, nullptr);
						FixBtn.HMargin(1.0f, &FixBtn);

						 
						if(DoButton_FontIcon(&pInvalidSetting->m_Context.m_Deleted, FONT_ICON_TRASH, pInvalidSetting->m_Context.m_Deleted, &DelBtn, BUTTONFLAG_LEFT, "Delete this command.", IGraphics::CORNER_ALL, 10.0f))
							pInvalidSetting->m_Context.m_Deleted = !pInvalidSetting->m_Context.m_Deleted;

						 
						if(DoButton_Editor(&pInvalidSetting->m_Context.m_Fixed, "Fix", !pInvalidSetting->m_Context.m_Deleted ? (s_FixingCommandIndex == -1 ? 0 : (IsFixing ? 1 : -1)) : -1, &FixBtn, BUTTONFLAG_LEFT, "Fix this command."))
						{
							s_FixingCommandIndex = i;
							SetInput(pInvalidSetting->m_aSetting);
						}
					}
					else if(IsFixing)
					{  
						 
						CUIRect OkBtn, CancelBtn;
						Slot.VSplitRight(50.0f, &Slot, &CancelBtn);
						Slot.VSplitRight(5.0f, &Slot, nullptr);
						CancelBtn.HMargin(1.0f, &CancelBtn);

						Slot.VSplitRight(30.0f, &Slot, &OkBtn);
						Slot.VSplitRight(10.0f, &Slot, nullptr);
						OkBtn.HMargin(1.0f, &OkBtn);

						 
						static int s_Cancel = 0, s_Ok = 0;
						if(DoButton_Editor(&s_Cancel, "Cancel", 0, &CancelBtn, BUTTONFLAG_LEFT, "Cancel fixing this command.") || Ui()->ConsumeHotkey(CUi::HOTKEY_ESCAPE))
						{
							s_FixingCommandIndex = -1;
							s_Input.Clear();
						}

						 
						 
						 
						ECollisionCheckResult Res = ECollisionCheckResult::ERROR;
						s_Context.CheckCollision(vSettingsValid, Res);
						bool Valid = s_Context.Valid() && Res == ECollisionCheckResult::ADD;

						if(DoButton_Editor(&s_Ok, "Done", Valid ? 0 : -1, &OkBtn, BUTTONFLAG_LEFT, "Confirm editing of this command.") || (s_Input.IsActive() && Valid && Ui()->ConsumeHotkey(CUi::HOTKEY_ENTER)))
						{
							 
							pInvalidSetting->m_Context.m_Fixed = true;
							str_copy(pInvalidSetting->m_aSetting, s_Input.GetString());
							 
							vSettingsValid.emplace_back(s_Input.GetString());

							 
							s_FixingCommandIndex = -1;
							s_Input.Clear();
						}
					}

					Label = Slot;
					Props.m_EllipsisAtEnd = true;
					Props.m_MaxWidth = Label.w;

					if(IsFixing)
					{
						 
						Label.HMargin(1.0, &FixInput);
						DisplayFixInput = true;
						DropdownHeight = minimum(DropdownHeight, EndY - FixInput.y - 16.0f);
					}
					else
					{
						 
						 
						 
						 
						if(!pInvalidSetting->m_Context.m_Deleted)
						{
							if(pInvalidSetting->m_Context.m_Fixed)
								TextRender()->TextColor(0.0f, 1.0f, 0.0f, 1.0f);
							else
								TextRender()->TextColor(1.0f, 0.0f, 0.0f, 1.0f);
							Ui()->DoLabel(&Label, pInvalidSetting->m_aSetting, 10.0f, TEXTALIGN_ML, Props);
						}
						else
						{
							TextRender()->TextColor(0.3f, 0.3f, 0.3f, 1.0f);
							Ui()->DoLabel(&Label, pInvalidSetting->m_aSetting, 10.0f, TEXTALIGN_ML, Props);

							CUIRect Line = Label;
							Line.y = Label.y + Label.h / 2;
							Line.h = 1;
							Line.Draw(ColorRGBA(1, 0, 0, 1), IGraphics::CORNER_NONE, 0.0f);
						}
					}
					TextRender()->TextColor(TextRender()->DefaultTextColor());
				}
			}
			else
			{  
				 
				const std::vector<int> &vDuplicates = SettingsDuplicate.at(i);
				int Chosen = -1;  
				for(int d = 0; d < (int)vDuplicates.size(); d++)
				{
					int DupIndex = vDuplicates[d];
					if(vSettingsInvalid[DupIndex].m_Context.m_Chosen)
					{
						Chosen = d;
						break;
					}
				}

				List.HSplitTop(RowHeight * (vDuplicates.size() + 1) + 2.0f, &Slot, &List);
				Slot.HMargin(1.0f, &Slot);

				 
				if(!vDuplicates.empty())
					Slot.Draw(ColorRGBA(1, 1, 1, 0.15f), IGraphics::CORNER_ALL, 3.0f);

				Slot.VMargin(5.0f, &Slot);
				Slot.HSplitTop(RowHeight, &Label, &Slot);
				Label.HMargin(1.0f, &Label);

				 
				if(!vDuplicates.empty())
				{
					CUIRect ChooseBtn;
					Label.VSplitRight(50.0f, &Label, &ChooseBtn);
					Label.VSplitRight(5.0f, &Label, nullptr);
					ChooseBtn.HMargin(1.0f, &ChooseBtn);
					if(DoButton_Editor(&vDuplicates, "Choose", Chosen == -1, &ChooseBtn, BUTTONFLAG_LEFT, "Choose this command."))
					{
						if(Chosen != -1)
							vSettingsInvalid[vDuplicates[Chosen]].m_Context.m_Chosen = false;
						Chosen = -1;  
					}
				}

				 
				Props.m_MaxWidth = Label.w;
				Ui()->DoLabel(&Label, Map()->m_vSettings[i].m_aCommand, 10.0f, TEXTALIGN_ML, Props);

				 
				 
				 
				Slot.VSplitLeft(10.0f, nullptr, &Slot);
				for(int DuplicateIndex = 0; DuplicateIndex < (int)vDuplicates.size(); DuplicateIndex++)
				{
					auto &Duplicate = vSettingsInvalid.at(vDuplicates[DuplicateIndex]);
					bool IsFixing = s_FixingCommandIndex == Duplicate.m_Index;
					bool IsInvalid = Duplicate.m_Type & SInvalidSetting::TYPE_INVALID;

					ColorRGBA Color(0.329f, 0.714f, 0.859f, 1.0f);
					CUIRect SubSlot;
					Slot.HSplitTop(RowHeight, &SubSlot, &Slot);
					SubSlot.HMargin(1.0f, &SubSlot);

					if(!IsFixing)
					{
						 

						CUIRect ChooseBtn;
						SubSlot.VSplitRight(50.0f, &SubSlot, &ChooseBtn);
						SubSlot.VSplitRight(5.0f, &SubSlot, nullptr);
						ChooseBtn.HMargin(1.0f, &ChooseBtn);
						if(DoButton_Editor(&Duplicate.m_Context.m_Chosen, "Choose", IsInvalid && !Duplicate.m_Context.m_Fixed ? -1 : Duplicate.m_Context.m_Chosen, &ChooseBtn, BUTTONFLAG_LEFT, "Override with this command."))
						{
							Duplicate.m_Context.m_Chosen = !Duplicate.m_Context.m_Chosen;
							if(Chosen != -1 && Chosen != DuplicateIndex)
								vSettingsInvalid[vDuplicates[Chosen]].m_Context.m_Chosen = false;
							Chosen = DuplicateIndex;
						}

						if(IsInvalid)
						{
							if(!Duplicate.m_Context.m_Fixed)
							{
								Color = ColorRGBA(1, 0, 1, 1);
								CUIRect FixBtn;
								SubSlot.VSplitRight(30.0f, &SubSlot, &FixBtn);
								SubSlot.VSplitRight(10.0f, &SubSlot, nullptr);
								FixBtn.HMargin(1.0f, &FixBtn);
								if(DoButton_Editor(&Duplicate.m_Context.m_Fixed, "Fix", s_FixingCommandIndex == -1 ? 0 : (IsFixing ? 1 : -1), &FixBtn, BUTTONFLAG_LEFT, "Fix this command (needed before it can be chosen)."))
								{
									s_FixingCommandIndex = Duplicate.m_Index;
									SetInput(Duplicate.m_aSetting);
								}
							}
							else
							{
								Color = ColorRGBA(0.329f, 0.714f, 0.859f, 1.0f);
							}
						}
					}
					else
					{
						 
						CUIRect OkBtn, CancelBtn;
						SubSlot.VSplitRight(50.0f, &SubSlot, &CancelBtn);
						SubSlot.VSplitRight(5.0f, &SubSlot, nullptr);
						CancelBtn.HMargin(1.0f, &CancelBtn);

						SubSlot.VSplitRight(30.0f, &SubSlot, &OkBtn);
						SubSlot.VSplitRight(10.0f, &SubSlot, nullptr);
						OkBtn.HMargin(1.0f, &OkBtn);

						static int s_Cancel = 0, s_Ok = 0;
						if(DoButton_Editor(&s_Cancel, "Cancel", 0, &CancelBtn, BUTTONFLAG_LEFT, "Cancel fixing this command.") || Ui()->ConsumeHotkey(CUi::HOTKEY_ESCAPE))
						{
							s_FixingCommandIndex = -1;
							s_Input.Clear();
						}

						 
						 
						 
						 
						 
						 
						 
						ECollisionCheckResult Res = ECollisionCheckResult::ERROR;
						s_Context.CheckCollision({Map()->m_vSettings[i]}, Res);
						bool Valid = s_Context.Valid() && Res == ECollisionCheckResult::REPLACE;

						if(DoButton_Editor(&s_Ok, "Done", Valid ? 0 : -1, &OkBtn, BUTTONFLAG_LEFT, "Confirm editing of this command.") || (s_Input.IsActive() && Valid && Ui()->ConsumeHotkey(CUi::HOTKEY_ENTER)))
						{
							if(Valid)  
							{
								 
								Duplicate.m_Context.m_Fixed = true;
								str_copy(Duplicate.m_aSetting, s_Input.GetString());

								s_FixingCommandIndex = -1;
								s_Input.Clear();
							}
						}
					}

					Label = SubSlot;
					Props.m_MaxWidth = Label.w;

					if(IsFixing)
					{
						 
						Label.HMargin(1.0, &FixInput);
						DisplayFixInput = true;
						DropdownHeight = minimum(DropdownHeight, EndY - FixInput.y - 16.0f);
					}
					else
					{
						 
						TextRender()->TextColor(Color);
						Ui()->DoLabel(&Label, Duplicate.m_aSetting, 10.0f, TEXTALIGN_ML, Props);
						TextRender()->TextColor(TextRender()->DefaultTextColor());
					}
				}
			}

			 
			s_ScrollRegion.AddRect(Slot);
		}

		 
		 
		CUIRect PaddingBottom;
		List.HSplitTop(30.0f, &PaddingBottom, &List);
		s_ScrollRegion.AddRect(PaddingBottom);

		 
		 
		if(DisplayFixInput)
			DoMapSettingsEditBox(&s_Context, &FixInput, 10.0f, maximum(DropdownHeight, 30.0f));

		s_ScrollRegion.End();
	}

	 
	static int s_ConfirmButton = 0, s_CancelButton = 0, s_FixAllButton = 0;
	CUIRect ConfirmButton, CancelButton, FixAllUnknownButton;
	ButtonBar.VSplitLeft(110.0f, &CancelButton, &ButtonBar);
	ButtonBar.VSplitRight(110.0f, &ButtonBar, &ConfirmButton);
	ButtonBar.VSplitRight(5.0f, &ButtonBar, nullptr);
	ButtonBar.VSplitRight(150.0f, &ButtonBar, &FixAllUnknownButton);

	bool CanConfirm = true;
	bool CanFixAllUnknown = false;
	for(auto &InvalidSetting : vSettingsInvalid)
	{
		if(!InvalidSetting.m_Context.m_Fixed && !InvalidSetting.m_Context.m_Deleted && !(InvalidSetting.m_Type & SInvalidSetting::TYPE_DUPLICATE))
		{
			CanConfirm = false;
			if(InvalidSetting.m_Unknown)
				CanFixAllUnknown = true;
			break;
		}
	}

	auto &&Execute = [&]() {
		 

		 
		for(auto &FixedSetting : vSettingsInvalid)
		{
			if(FixedSetting.m_Context.m_Fixed)
			{
				str_copy(Map()->m_vSettings[FixedSetting.m_Index].m_aCommand, FixedSetting.m_aSetting);
			}
		}

		 
		 
		 
		std::vector<CEditorMapSetting> vSettingsToErase;
		for(auto &Setting : vSettingsInvalid)
		{
			if(Setting.m_Type & SInvalidSetting::TYPE_DUPLICATE)
			{
				if(!Setting.m_Context.m_Chosen)
					vSettingsToErase.emplace_back(Setting.m_aSetting);
				else
					vSettingsToErase.emplace_back(Map()->m_vSettings[Setting.m_CollidingIndex].m_aCommand);
			}
		}

		 
		for(auto &DeletedSetting : vSettingsInvalid)
		{
			if(DeletedSetting.m_Context.m_Deleted)
			{
				Map()->m_vSettings.erase(
					std::remove_if(Map()->m_vSettings.begin(), Map()->m_vSettings.end(), [&](const CEditorMapSetting &MapSetting) {
						return str_comp_nocase(MapSetting.m_aCommand, DeletedSetting.m_aSetting) == 0;
					}),
					Map()->m_vSettings.end());
			}
		}

		 
		for(auto &Setting : vSettingsToErase)
		{
			Map()->m_vSettings.erase(
				std::remove_if(Map()->m_vSettings.begin(), Map()->m_vSettings.end(), [&](const CEditorMapSetting &MapSetting) {
					return str_comp_nocase(MapSetting.m_aCommand, Setting.m_aCommand) == 0;
				}),
				Map()->m_vSettings.end());
		}

		Map()->OnModify();
	};

	auto &&FixAllUnknown = [&] {
		 
		for(auto &InvalidSetting : vSettingsInvalid)
			if(!InvalidSetting.m_Context.m_Fixed && !InvalidSetting.m_Context.m_Deleted && !(InvalidSetting.m_Type & SInvalidSetting::TYPE_DUPLICATE) && InvalidSetting.m_Unknown)
				InvalidSetting.m_Context.m_Fixed = true;
	};

	 
	if(DoButton_Editor(&s_FixAllButton, "Allow all unknown settings", CanFixAllUnknown ? 0 : -1, &FixAllUnknownButton, BUTTONFLAG_LEFT, nullptr))
	{
		FixAllUnknown();
	}

	 
	if(DoButton_Editor(&s_ConfirmButton, "Confirm", CanConfirm ? 0 : -1, &ConfirmButton, BUTTONFLAG_LEFT, nullptr) || (CanConfirm && Ui()->ConsumeHotkey(CUi::HOTKEY_ENTER)))
	{
		Execute();
		OnDialogClose();
	}

	 
	if(DoButton_Editor(&s_CancelButton, "Cancel", 0, &CancelButton, BUTTONFLAG_LEFT, nullptr) || (Ui()->ConsumeHotkey(CUi::HOTKEY_ESCAPE)))
	{
		Reset();
		OnDialogClose();
	}
}

void CEditor::MapSettingsDropdownRenderCallback(const SPossibleValueMatch &Match, char (&aOutput)[128], std::vector<STextColorSplit> &vColorSplits)
{
	 
	 
	 
	if(Match.m_ArgIndex == -1)
	{
		IMapSetting *pInfo = (IMapSetting *)Match.m_pData;
		vColorSplits = {
			{str_length(pInfo->m_pName) + 1, -1, ColorRGBA(0.6f, 0.6f, 0.6f, 1)},  
		};

		if(pInfo->m_Type == IMapSetting::SETTING_INT)
		{
			str_format(aOutput, sizeof(aOutput), "%s i[value]", pInfo->m_pName);
		}
		else if(pInfo->m_Type == IMapSetting::SETTING_COMMAND)
		{
			SMapSettingCommand *pCommand = (SMapSettingCommand *)pInfo;
			str_format(aOutput, sizeof(aOutput), "%s %s", pCommand->m_pName, pCommand->m_pArgs);
		}
	}
	else
	{
		str_copy(aOutput, Match.m_pValue);
	}
}

 

void CMapSettingsBackend::OnInit(CEditor *pEditor)
{
	CEditorComponent::OnInit(pEditor);

	 
	InitValueLoaders();

	 
	LoadAllMapSettings();

	CValuesBuilder Builder(&m_PossibleValuesPerCommand);

	 
	for(auto &pSetting : m_vpMapSettings)
	{
		 
		 
		 
		 
		if(pSetting->m_Type == IMapSetting::SETTING_INT)
			LoadSettingInt(std::static_pointer_cast<SMapSettingInt>(pSetting));
		else if(pSetting->m_Type == IMapSetting::SETTING_COMMAND)
			LoadSettingCommand(std::static_pointer_cast<SMapSettingCommand>(pSetting));

		LoadPossibleValues(Builder(pSetting->m_pName), pSetting);
	}

	 
	LoadConstraints();
}

void CMapSettingsBackend::LoadAllMapSettings()
{
	 
	Editor()->ConfigManager()->PossibleConfigVariables("", CFGFLAG_GAME, PossibleConfigVariableCallback, this);

	 
	LoadCommand("tune", "s[tuning] f[value]", "Tune variable to value or show current value");
	LoadCommand("tune_zone", "i[zone] s[tuning] f[value]", "Tune in zone a variable to value");
	LoadCommand("tune_zone_enter", "i[zone] r[message]", "Which message to display on zone enter; use 0 for normal area");
	LoadCommand("tune_zone_leave", "i[zone] r[message]", "Which message to display on zone leave; use 0 for normal area");
	LoadCommand("mapbug", "s[mapbug]", "Enable map compatibility mode using the specified bug (example: grenade-doubleexplosion@ddnet.tw)");
	LoadCommand("switch_open", "i[switch]", "Whether a switch is deactivated by default (otherwise activated)");
}

void CMapSettingsBackend::LoadCommand(const char *pName, const char *pArgs, const char *pHelp)
{
	m_vpMapSettings.emplace_back(std::make_shared<SMapSettingCommand>(pName, pHelp, pArgs));
}

void CMapSettingsBackend::LoadSettingInt(const std::shared_ptr<SMapSettingInt> &pSetting)
{
	 
	m_ParsedCommandArgs[pSetting].emplace_back();
	auto &Arg = m_ParsedCommandArgs[pSetting].back();
	str_copy(Arg.m_aName, "value");
	Arg.m_Type = 'i';
}

void CMapSettingsBackend::LoadSettingCommand(const std::shared_ptr<SMapSettingCommand> &pSetting)
{
	 
	 
	 

	m_ParsedCommandArgs[pSetting].clear();
	const char *pIterator = pSetting->m_pArgs;

	char Type;

	while(*pIterator)
	{
		if(*pIterator == '?')  
			pIterator++;

		Type = *pIterator;
		pIterator++;
		while(*pIterator && *pIterator != '[')
			pIterator++;
		pIterator++;  

		const char *pNameStart = pIterator;

		while(*pIterator && *pIterator != ']')
			pIterator++;

		size_t Len = pIterator - pNameStart;
		pIterator++;  

		dbg_assert(Len + 1 < sizeof(SParsedMapSettingArg::m_aName), "Length of server setting name exceeds limit.");

		 
		m_ParsedCommandArgs[pSetting].emplace_back();
		auto &Arg = m_ParsedCommandArgs[pSetting].back();
		str_copy(Arg.m_aName, pNameStart, Len + 1);
		Arg.m_Type = Type;

		pIterator = str_skip_whitespaces_const(pIterator);
	}
}

void CMapSettingsBackend::LoadPossibleValues(const CSettingValuesBuilder &Builder, const std::shared_ptr<IMapSetting> &pSetting)
{
	 
	auto Iter = m_LoaderFunctions.find(pSetting->m_pName);
	if(Iter == m_LoaderFunctions.end())
		return;

	(*Iter->second)(Builder);
}

void CMapSettingsBackend::RegisterLoader(const char *pSettingName, const FLoaderFunction &pfnLoader)
{
	 
	m_LoaderFunctions[pSettingName] = pfnLoader;
}

void CMapSettingsBackend::LoadConstraints()
{
	 
	CCommandArgumentConstraintBuilder Command(&m_ArgConstraintsPerCommand);

	 
	 
	Command("tune", 2).Unique(0);
	Command("tune_zone", 3).Multiple(0).Unique(1);
	Command("tune_zone_enter", 2).Unique(0);
	Command("tune_zone_leave", 2).Unique(0);
	Command("switch_open", 1).Unique(0);
	Command("mapbug", 1).Unique(0);
}

void CMapSettingsBackend::PossibleConfigVariableCallback(const SConfigVariable *pVariable, void *pUserData)
{
	CMapSettingsBackend *pBackend = (CMapSettingsBackend *)pUserData;

	if(pVariable->m_Type == SConfigVariable::VAR_INT)
	{
		SIntConfigVariable *pIntVariable = (SIntConfigVariable *)pVariable;
		pBackend->m_vpMapSettings.emplace_back(std::make_shared<SMapSettingInt>(
			pIntVariable->m_pScriptName,
			pIntVariable->m_pHelp,
			pIntVariable->m_Default,
			pIntVariable->m_Min,
			pIntVariable->m_Max));
	}
}

void CMapSettingsBackend::CContext::Reset()
{
	m_LastCursorOffset = 0;
	m_CursorArgIndex = -1;
	m_pCurrentSetting = nullptr;
	m_vCurrentArgs.clear();
	m_aCommand[0] = '\0';
	m_DropdownContext.m_Selected = -1;
	m_CurrentCompletionIndex = -1;
	m_DropdownContext.m_ShortcutUsed = false;
	m_DropdownContext.m_MousePressedInside = false;
	m_DropdownContext.m_Visible = false;
	m_DropdownContext.m_ShouldHide = false;
	m_CommentOffset = -1;

	ClearError();
}

void CMapSettingsBackend::CContext::Update()
{
	UpdateFromString(InputString());
}

void CMapSettingsBackend::CContext::UpdateFromString(const char *pStr)
{
	 
	 
	 

	m_pCurrentSetting = nullptr;
	m_vCurrentArgs.clear();
	m_CommentOffset = -1;

	const char *pIterator = pStr;

	 
	const char *pEnd = pStr;
	int InString = 0;

	while(*pEnd)
	{
		if(*pEnd == '"')
			InString ^= 1;
		else if(*pEnd == '\\')  
		{
			if(pEnd[1] == '"')
				pEnd++;
		}
		else if(!InString)
		{
			if(*pEnd == '#')  
			{
				m_CommentOffset = pEnd - pStr;
				break;
			}
		}

		pEnd++;
	}

	if(m_CommentOffset == 0)
		return;

	 
	char aInputString[256];
	str_copy(aInputString, pStr, m_CommentOffset != -1 ? m_CommentOffset + 1 : sizeof(aInputString));
	pIterator = aInputString;

	 
	m_aCommand[0] = '\0';
	while(pIterator && *pIterator != ' ' && *pIterator != '\0')
		pIterator++;

	str_copy(m_aCommand, aInputString, (pIterator - aInputString) + 1);

	 
	for(auto &pSetting : m_pBackend->m_vpMapSettings)
	{
		if(str_comp_nocase(m_aCommand, pSetting->m_pName) == 0)
		{
			m_pCurrentSetting = pSetting;
			break;
		}
	}

	 
	ParseArgs(aInputString, pIterator);
}

void CMapSettingsBackend::CContext::ParseArgs(const char *pLineInputStr, const char *pStr)
{
	 

	ClearError();

	const char *pIterator = pStr;

	if(!pStr || *pStr == '\0')
		return;  

	 
	 
	 
	auto &&NextArg = [&](const char *pArg, int *pLength) {
		if(*pIterator == '"')
		{
			pIterator++;
			bool Valid = true;
			bool IsEscape = false;

			while(true)
			{
				if(pIterator[0] == '"' && !IsEscape)
					break;
				else if(pIterator[0] == 0)
				{
					Valid = false;
					break;
				}

				if(pIterator[0] == '\\' && !IsEscape)
					IsEscape = true;
				else if(IsEscape)
					IsEscape = false;

				pIterator++;
			}
			const char *pEnd = ++pIterator;
			pIterator = str_skip_to_whitespace_const(pIterator);

			 
			 
			Valid = Valid && pIterator == pEnd;
			*pLength = pEnd - pArg;

			return Valid;
		}
		else
		{
			pIterator = str_skip_to_whitespace_const(pIterator);
			*pLength = pIterator - pArg;
			return true;
		}
	};

	 
	auto &&ValidateStr = [](const char *pString) -> bool {
		const char *pIt = pString;
		bool IsEscape = false;
		while(*pIt)
		{
			if(pIt[0] == '"' && !IsEscape)
				return false;

			if(pIt[0] == '\\' && !IsEscape)
				IsEscape = true;
			else if(IsEscape)
				IsEscape = false;

			pIt++;
		}
		return true;
	};

	const int CommandArgCount = m_pCurrentSetting != nullptr ? m_pBackend->m_ParsedCommandArgs.at(m_pCurrentSetting).size() : 0;
	int ArgIndex = 0;
	SCommandParseError::EErrorType Error = SCommandParseError::ERROR_NONE;

	 
	float PosX = 0;
	const float WW = m_pLineInput != nullptr ? m_pBackend->TextRender()->TextWidth(m_FontSize, " ") : 0.0f;
	PosX += m_pLineInput != nullptr ? m_pBackend->TextRender()->TextWidth(m_FontSize, m_aCommand) : 0.0f;

	 
	while(*pIterator)
	{
		Error = SCommandParseError::ERROR_NONE;
		pIterator++;  
		PosX += WW;  

		 
		char Char = *pIterator;
		const char *pArgStart = pIterator;
		int Length;
		bool Valid = NextArg(pArgStart, &Length);  
		size_t Offset = pArgStart - pLineInputStr;  

		 
		m_vCurrentArgs.emplace_back();
		auto &NewArg = m_vCurrentArgs.back();
		 
		str_copy(NewArg.m_aValue, pArgStart, minimum((int)sizeof(SCurrentSettingArg::m_aValue), Length + 1));

		 
		 

		char Type = 'u';  
		if(ArgIndex < CommandArgCount)
		{
			SParsedMapSettingArg &Arg = m_pBackend->m_ParsedCommandArgs[m_pCurrentSetting].at(ArgIndex);
			if(Arg.m_Type == 'r')
			{
				 
				 
				if(Char != '"')
				{
					while(*pIterator)
						pIterator++;
					Length = pIterator - pArgStart;
					str_copy(NewArg.m_aValue, pArgStart, Length + 1);
				}

				if(!Valid)
					Error = SCommandParseError::ERROR_INVALID_VALUE;
			}
			else if(Arg.m_Type == 'i')
			{
				 
				if(!str_toint(NewArg.m_aValue, nullptr))
					Error = SCommandParseError::ERROR_INVALID_VALUE;
			}
			else if(Arg.m_Type == 'f')
			{
				 
				if(!str_tofloat(NewArg.m_aValue, nullptr))
					Error = SCommandParseError::ERROR_INVALID_VALUE;
			}
			else if(Arg.m_Type == 's')
			{
				 
				if(!Valid || (Char != '"' && !ValidateStr(NewArg.m_aValue)))
					Error = SCommandParseError::ERROR_INVALID_VALUE;
			}

			 
			 
			 
			EValidationResult Result = ValidateArg(ArgIndex, NewArg.m_aValue);
			if(Length && !Error && Result != EValidationResult::VALID)
			{
				if(Result == EValidationResult::ERROR)
					Error = SCommandParseError::ERROR_INVALID_VALUE;  
				else if(Result == EValidationResult::UNKNOWN)
					Error = SCommandParseError::ERROR_UNKNOWN_VALUE;  
				else if(Result == EValidationResult::INCOMPLETE)
					Error = SCommandParseError::ERROR_INCOMPLETE;  
				else if(Result == EValidationResult::OUT_OF_RANGE)
					Error = SCommandParseError::ERROR_OUT_OF_RANGE;  
				else
					Error = SCommandParseError::ERROR_UNKNOWN;  
			}

			Type = Arg.m_Type;
		}
		else
		{
			 
			if(m_CommentOffset == -1)
				Error = SCommandParseError::ERROR_TOO_MANY_ARGS;
			else
			{  
				const char *pSubIt = pArgStart;
				pSubIt = str_skip_whitespaces_const(pSubIt);
				if(*pSubIt != '\0')
				{  
					Error = SCommandParseError::ERROR_TOO_MANY_ARGS;
				}
				else  
				{
					m_vCurrentArgs.pop_back();
					break;
				}
			}
		}

		 
		NewArg.m_X = PosX;
		NewArg.m_Start = Offset;
		NewArg.m_End = Offset + Length;
		NewArg.m_Error = Error != SCommandParseError::ERROR_NONE || Length == 0 || m_Error.m_Type != SCommandParseError::ERROR_NONE;
		NewArg.m_ExpectedType = Type;

		 
		if(Error == SCommandParseError::ERROR_INVALID_VALUE || Error == SCommandParseError::ERROR_UNKNOWN_VALUE || Error == SCommandParseError::ERROR_OUT_OF_RANGE || Error == SCommandParseError::ERROR_INCOMPLETE)
		{
			 
			if(!m_Error.m_aMessage[0])
			{
				int ErrorArgIndex = (int)m_vCurrentArgs.size() - 1;
				SCurrentSettingArg &ErrorArg = m_vCurrentArgs.back();
				SParsedMapSettingArg &SettingArg = m_pBackend->m_ParsedCommandArgs[m_pCurrentSetting].at(ArgIndex);
				char aFormattedValue[256];
				FormatDisplayValue(ErrorArg.m_aValue, aFormattedValue);

				if(Error == SCommandParseError::ERROR_INVALID_VALUE || Error == SCommandParseError::ERROR_UNKNOWN_VALUE || Error == SCommandParseError::ERROR_INCOMPLETE)
				{
					static const std::map<int, const char *> s_Names = {
						{SCommandParseError::ERROR_INVALID_VALUE, "Invalid"},
						{SCommandParseError::ERROR_UNKNOWN_VALUE, "Unknown"},
						{SCommandParseError::ERROR_INCOMPLETE, "Incomplete"},
					};
					str_format(m_Error.m_aMessage, sizeof(m_Error.m_aMessage), "%s argument value: %s at position %d for argument '%s'", s_Names.at(Error), aFormattedValue, (int)ErrorArg.m_Start, SettingArg.m_aName);
				}
				else
				{
					std::shared_ptr<SMapSettingInt> pSettingInt = std::static_pointer_cast<SMapSettingInt>(m_pCurrentSetting);
					str_format(m_Error.m_aMessage, sizeof(m_Error.m_aMessage), "Invalid argument value: %s at position %d for argument '%s': out of range [%d, %d]", aFormattedValue, (int)ErrorArg.m_Start, SettingArg.m_aName, pSettingInt->m_Min, pSettingInt->m_Max);
				}
				m_Error.m_ArgIndex = ErrorArgIndex;
				m_Error.m_Type = Error;
			}
		}
		else if(Error == SCommandParseError::ERROR_TOO_MANY_ARGS)
		{
			 
			if(!m_Error.m_aMessage[0])
			{
				if(m_pCurrentSetting != nullptr)
				{
					str_copy(m_Error.m_aMessage, "Too many arguments");
					m_Error.m_ArgIndex = ArgIndex;
					break;
				}
				else
				{
					char aFormattedValue[256];
					FormatDisplayValue(m_aCommand, aFormattedValue);
					str_format(m_Error.m_aMessage, sizeof(m_Error.m_aMessage), "Unknown server setting: %s", aFormattedValue);
					m_Error.m_ArgIndex = -1;
					break;
				}
				m_Error.m_Type = Error;
			}
		}

		PosX += m_pLineInput != nullptr ? m_pBackend->TextRender()->TextWidth(m_FontSize, pArgStart, Length) : 0.0f;  
		ArgIndex++;
	}
}

void CMapSettingsBackend::CContext::ClearError()
{
	m_Error.m_aMessage[0] = '\0';
	m_Error.m_Type = SCommandParseError::ERROR_NONE;
}

bool CMapSettingsBackend::CContext::UpdateCursor(bool Force)
{
	 
	 
	 
	 
	 

	if(!m_pLineInput)
		return false;

	size_t Offset = m_pLineInput->GetCursorOffset();
	if(Offset == m_LastCursorOffset && !Force)
		return false;

	m_LastCursorOffset = Offset;
	int NewArg = m_CursorArgIndex;

	 
	if(m_CommentOffset != -1 && Offset >= (size_t)m_CommentOffset)
	{
		NewArg = (int)m_vCurrentArgs.size();
	}
	else
	{
		bool FoundArg = false;
		for(int i = (int)m_vCurrentArgs.size() - 1; i >= 0; i--)
		{
			if(Offset >= m_vCurrentArgs[i].m_Start)
			{
				NewArg = i;
				FoundArg = true;
				break;
			}
		}

		if(!FoundArg)
			NewArg = -1;
	}

	bool ShouldUpdate = NewArg != m_CursorArgIndex;
	m_CursorArgIndex = NewArg;

	 
	if(m_pLineInput != nullptr)
	{
		if(Offset == m_pLineInput->GetLength() && m_Error.m_aMessage[0] && m_Error.m_ArgIndex == m_CursorArgIndex && m_Error.m_Type == SCommandParseError::ERROR_INCOMPLETE)
			ClearError();
	}

	if(m_DropdownContext.m_Selected == -1 || ShouldUpdate || Force)
	{
		 
		UpdatePossibleMatches();
	}

	return true;
}

EValidationResult CMapSettingsBackend::CContext::ValidateArg(int Index, const char *pArg)
{
	if(!m_pCurrentSetting)
		return EValidationResult::ERROR;

	 
	if(m_pCurrentSetting->m_Type == IMapSetting::SETTING_INT)
	{
		std::shared_ptr<SMapSettingInt> pSetting = std::static_pointer_cast<SMapSettingInt>(m_pCurrentSetting);
		if(Index > 0)
			return EValidationResult::ERROR;

		int Value;
		if(!str_toint(pArg, &Value))  
			return EValidationResult::ERROR;

		return Value >= pSetting->m_Min && Value <= pSetting->m_Max ? EValidationResult::VALID : EValidationResult::OUT_OF_RANGE;
	}
	else if(m_pCurrentSetting->m_Type == IMapSetting::SETTING_COMMAND)
	{
		auto &vArgs = m_pBackend->m_ParsedCommandArgs.at(m_pCurrentSetting);
		if(Index < (int)vArgs.size())
		{
			auto It = m_pBackend->m_PossibleValuesPerCommand.find(m_pCurrentSetting->m_pName);
			if(It != m_pBackend->m_PossibleValuesPerCommand.end())
			{
				auto ValuesIt = It->second.find(Index);
				if(ValuesIt != It->second.end())
				{
					 
					 
					const bool EqualsAny = std::any_of(ValuesIt->second.begin(), ValuesIt->second.end(), [pArg](auto *pValue) { return str_comp_nocase(pArg, pValue) == 0; });

					 
					if(EqualsAny)
						return EValidationResult::VALID;

					 
					const bool StartsAny = std::any_of(ValuesIt->second.begin(), ValuesIt->second.end(), [pArg](auto *pValue) { return str_startswith_nocase(pValue, pArg) != nullptr; });
					if(StartsAny)
						return EValidationResult::INCOMPLETE;

					return EValidationResult::UNKNOWN;
				}
			}
		}

		 
		 
		 
		 
		return EValidationResult::VALID;
	}

	return EValidationResult::ERROR;
}

void CMapSettingsBackend::CContext::UpdatePossibleMatches()
{
	 
	 
	 
	 
	 
	 

	m_vPossibleMatches.clear();
	m_DropdownContext.m_Selected = -1;

	if(m_CommentOffset == 0 || (m_aCommand[0] == '\0' && !m_DropdownContext.m_ShortcutUsed))
		return;

	 
	if(m_CursorArgIndex == -1)
	{
		 
		char aSubString[128];
		str_copy(aSubString, m_aCommand, minimum(m_LastCursorOffset + 1, sizeof(aSubString)));

		 
		for(auto &pSetting : m_pBackend->m_vpMapSettings)
		{
			if(str_startswith_nocase(pSetting->m_pName, aSubString))
			{
				m_vPossibleMatches.emplace_back(SPossibleValueMatch{
					pSetting->m_pName,
					m_CursorArgIndex,
					pSetting.get(),
				});
			}
		}

		 
		if(m_vPossibleMatches.empty())
		{
			 
			char aFormattedValue[256];
			FormatDisplayValue(m_aCommand, aFormattedValue);
			str_format(m_Error.m_aMessage, sizeof(m_Error.m_aMessage), "Unknown server setting: %s", aFormattedValue);
			m_Error.m_ArgIndex = -1;
		}
	}
	else
	{
		 
		if(!m_pCurrentSetting)  
			return;

		if(m_pCurrentSetting->m_Type == IMapSetting::SETTING_INT)
		{
			 
			 
		}
		else
		{
			 
			auto &vArgs = m_pBackend->m_ParsedCommandArgs.at(m_pCurrentSetting);
			 
			if(m_CursorArgIndex < (int)vArgs.size() && m_CursorArgIndex < (int)m_vCurrentArgs.size())
			{
				 
				auto It = m_pBackend->m_PossibleValuesPerCommand.find(m_pCurrentSetting->m_pName);
				if(It != m_pBackend->m_PossibleValuesPerCommand.end())
				{
					 
					auto ValuesIt = It->second.find(m_CursorArgIndex);
					if(ValuesIt != It->second.end())
					{
						 
						 

						auto &CurrentArg = m_vCurrentArgs.at(m_CursorArgIndex);
						int SubstringLength = minimum(m_LastCursorOffset, CurrentArg.m_End) - CurrentArg.m_Start;

						 
						char aSubString[256];
						str_copy(aSubString, CurrentArg.m_aValue, SubstringLength + 1);

						for(auto &pValue : ValuesIt->second)
						{
							if(str_startswith_nocase(pValue, aSubString))
							{
								m_vPossibleMatches.emplace_back(SPossibleValueMatch{
									pValue,
									m_CursorArgIndex,
									nullptr,
								});
							}
						}
					}
				}
			}
		}
	}
}

bool CMapSettingsBackend::CContext::OnInput(const IInput::CEvent &Event)
{
	if(!m_pLineInput)
		return false;

	if(!m_pLineInput->IsActive())
		return false;

	if(Event.m_Flags & (IInput::FLAG_PRESS | IInput::FLAG_TEXT) && !m_pBackend->Input()->ModifierIsPressed() && !m_pBackend->Input()->AltIsPressed())
	{
		 
		 
		 
		if(Event.m_Key != KEY_TAB && Event.m_Key != KEY_LSHIFT && Event.m_Key != KEY_RSHIFT && Event.m_Key != KEY_UP && Event.m_Key != KEY_DOWN && !(Event.m_Key >= KEY_MOUSE_1 && Event.m_Key <= KEY_MOUSE_WHEEL_RIGHT))
		{
			if(m_CurrentCompletionIndex != -1)
			{
				m_CurrentCompletionIndex = -1;
				m_DropdownContext.m_Selected = -1;
				Update();
				UpdateCursor(true);
			}
		}
	}

	return false;
}

const char *CMapSettingsBackend::CContext::InputString() const
{
	if(!m_pLineInput)
		return nullptr;
	return m_pBackend->Input()->HasComposition() ? m_CompositionStringBuffer.c_str() : m_pLineInput->GetString();
}

void CMapSettingsBackend::CContext::ColorArguments(std::vector<STextColorSplit> &vColorSplits) const
{
	 
	auto &&GetArgumentColor = [](char Type) -> ColorRGBA {
		if(Type == 'u')
			return ms_ArgumentUnknownColor;
		else if(Type == 's' || Type == 'r')
			return ms_ArgumentStringColor;
		else if(Type == 'i' || Type == 'f')
			return ms_ArgumentNumberColor;
		return ms_ErrorColor;  
	};

	 
	for(int i = 0; i < ArgCount(); i++)
	{
		const auto &Argument = Arg(i);
		 
		auto Color = Argument.m_Error ? ms_ErrorColor : GetArgumentColor(Argument.m_ExpectedType);
		vColorSplits.emplace_back(Argument.m_Start, Argument.m_End - Argument.m_Start, Color);
	}

	if(m_pLineInput && !m_pLineInput->IsEmpty())
	{
		if(!CommandIsValid() && m_CommentOffset != 0)
		{
			 
			int ErrorLength = m_CommentOffset == -1 ? -1 : m_CommentOffset;
			vColorSplits = {{0, ErrorLength, ms_ErrorColor}};
		}
		else if(HasError())
		{
			 
			int ErrorLength = m_CommentOffset == -1 ? -1 : m_CommentOffset - ErrorOffset();
			vColorSplits.emplace_back(ErrorOffset(), ErrorLength, ms_ErrorColor);
		}
		if(m_CommentOffset != -1)
		{  
			vColorSplits.emplace_back(m_CommentOffset, -1, ms_CommentColor);
		}
	}

	std::sort(vColorSplits.begin(), vColorSplits.end(), [](const STextColorSplit &a, const STextColorSplit &b) {
		return a.m_CharIndex < b.m_CharIndex;
	});
}

int CMapSettingsBackend::CContext::CheckCollision(ECollisionCheckResult &Result) const
{
	return CheckCollision(m_pBackend->Editor()->Map()->m_vSettings, Result);
}

int CMapSettingsBackend::CContext::CheckCollision(const std::vector<CEditorMapSetting> &vSettings, ECollisionCheckResult &Result) const
{
	return CheckCollision(InputString(), vSettings, Result);
}

int CMapSettingsBackend::CContext::CheckCollision(const char *pInputString, const std::vector<CEditorMapSetting> &vSettings, ECollisionCheckResult &Result) const
{
	 
	 
	 
	 
	 
	 
	 

	const int InputLength = str_length(pInputString);

	if(m_CommentOffset == 0 || InputLength == 0)
	{  
		Result = ECollisionCheckResult::ADD;
		return -1;
	}

	struct SArgument
	{
		char m_aValue[128];
		SArgument(const char *pStr)
		{
			str_copy(m_aValue, pStr);
		}
	};

	struct SLineArgs
	{
		int m_Index;
		std::vector<SArgument> m_vArgs;
	};

	 
	auto &&SplitSetting = [](const char *pStr) {
		std::vector<SArgument> vaArgs;
		const char *pIt = pStr;
		char aBuffer[128];
		while((pIt = str_next_token(pIt, " ", aBuffer, sizeof(aBuffer))))
			vaArgs.emplace_back(aBuffer);
		return vaArgs;
	};

	 
	Result = ECollisionCheckResult::ERROR;

	 
	if(!CommandIsValid())
	{
		 
		 
		if(!m_AllowUnknownCommands)
			return -1;

		 
		 
		 
		for(int i = 0; i < (int)vSettings.size(); i++)
		{
			if(str_comp_nocase(vSettings[i].m_aCommand, pInputString) == 0)
			{
				Result = ECollisionCheckResult::REPLACE;
				return i;
			}
		}

		 
		 
		const char *pSpace = str_find(pInputString, " ");
		if(!pSpace || !*(pSpace + 1))
			Result = ECollisionCheckResult::ERROR;
		else
			Result = ECollisionCheckResult::ADD;

		return -1;  
	}

	 
	 
	 

	const std::shared_ptr<IMapSetting> &pSetting = Setting();
	if(pSetting->m_Type == IMapSetting::SETTING_INT)
	{
		 
		 

		 
		 
		 
		char aBuffer[256];
		auto It = std::find_if(vSettings.begin(), vSettings.end(), [&](const CEditorMapSetting &Setting) {
			const char *pLineSettingValue = Setting.m_aCommand;  
			pLineSettingValue = str_next_token(pLineSettingValue, " ", aBuffer, sizeof(aBuffer));  
			return str_comp_nocase(aBuffer, pSetting->m_pName) == 0;  
		});

		if(It == vSettings.end())
		{
			 
			Result = ECollisionCheckResult::ADD;
			return -1;
		}
		else
		{
			 
			Result = ECollisionCheckResult::REPLACE;
			return It - vSettings.begin();  
		}
	}
	else if(pSetting->m_Type == IMapSetting::SETTING_COMMAND)
	{
		 
		 
		 
		 
		 
		 
		 
		 

		std::shared_ptr<SMapSettingCommand> pSettingCommand = std::static_pointer_cast<SMapSettingCommand>(pSetting);
		 
		std::vector<SLineArgs> vLineArgs;
		for(int i = 0; i < (int)vSettings.size(); i++)
		{
			const auto &Setting = vSettings.at(i);

			 
			std::vector<SArgument> vArgs = SplitSetting(Setting.m_aCommand);
			 
			if(!vArgs.empty() && str_comp_nocase(vArgs[0].m_aValue, pSettingCommand->m_pName) == 0)
			{
				 
				vArgs.erase(vArgs.begin());
				vLineArgs.push_back(SLineArgs{
					i,
					vArgs,
				});
			}
		}

		 
		bool Error = false;
		int CollidingLineIndex = -1;
		for(int ArgIndex = 0; ArgIndex < ArgCount(); ArgIndex++)
		{
			bool Collide = false;
			const char *pValue = Arg(ArgIndex).m_aValue;
			for(auto &Line : vLineArgs)
			{
				 
				if(str_comp_nocase(pValue, Line.m_vArgs[ArgIndex].m_aValue) == 0)
				{
					Collide = true;
					CollidingLineIndex = Line.m_Index;
					Error = m_pBackend->ArgConstraint(pSetting->m_pName, ArgIndex) == CMapSettingsBackend::EArgConstraint::UNIQUE;
				}
				if(Error)
					break;
			}

			 
			 
			if(!Collide || Error)
				break;

			 
			vLineArgs.erase(
				std::remove_if(vLineArgs.begin(), vLineArgs.end(), [&](const SLineArgs &Line) {
					return str_comp_nocase(pValue, Line.m_vArgs[ArgIndex].m_aValue) != 0;
				}),
				vLineArgs.end());
		}

		 
		Result = Error ? ECollisionCheckResult::REPLACE : ECollisionCheckResult::ADD;
		return CollidingLineIndex;
	}

	return -1;
}

bool CMapSettingsBackend::CContext::Valid() const
{
	 

	 
	if(m_AllowUnknownCommands)
		return true;

	if(m_CommentOffset == 0 || m_aCommand[0] == '\0')
		return true;  

	 
	if(m_pCurrentSetting)
	{
		 
		const bool ArgumentsValid = std::all_of(m_vCurrentArgs.begin(), m_vCurrentArgs.end(), [](const SCurrentSettingArg &Arg) {
			return !Arg.m_Error;
		});

		if(!ArgumentsValid)
			return false;

		 
		return m_vCurrentArgs.size() == m_pBackend->m_ParsedCommandArgs.at(m_pCurrentSetting).size();
	}
	else
	{
		return false;
	}
}

void CMapSettingsBackend::CContext::GetCommandHelpText(char *pStr, int Length) const
{
	if(!m_pCurrentSetting)
		return;

	str_copy(pStr, m_pCurrentSetting->m_pHelp, Length);
}

template<int N>
void CMapSettingsBackend::CContext::FormatDisplayValue(const char *pValue, char (&aOut)[N])
{
	const int MaxLength = 32;
	if(str_length(pValue) > MaxLength)
	{
		str_copy(aOut, pValue, MaxLength);
		str_append(aOut, "...");
	}
	else
	{
		str_copy(aOut, pValue);
	}
}

void CMapSettingsBackend::OnMapLoad()
{
	 
	m_LoadedMapSettings.Reset();

	auto &vLoadedMapSettings = Editor()->Map()->m_vSettings;

	 

	 
	CContext LocalContext = NewContext(nullptr);

	 
	 
	 
	 

	std::vector<std::tuple<int, bool, CEditorMapSetting>> vSettingsInvalid;

	for(int i = 0; i < (int)vLoadedMapSettings.size(); i++)
	{
		CEditorMapSetting &Setting = vLoadedMapSettings.at(i);
		 
		LocalContext.UpdateFromString(Setting.m_aCommand);

		bool Valid = LocalContext.Valid();
		ECollisionCheckResult Result = ECollisionCheckResult::ERROR;
		LocalContext.CheckCollision(Setting.m_aCommand, m_LoadedMapSettings.m_vSettingsValid, Result);

		if(Valid && Result == ECollisionCheckResult::ADD)
			m_LoadedMapSettings.m_vSettingsValid.emplace_back(Setting);
		else
			vSettingsInvalid.emplace_back(i, Valid, Setting);

		LocalContext.Reset();

		 
		m_LoadedMapSettings.m_SettingsDuplicate.insert({i, {}});
	}

	for(const auto &[Index, Valid, Setting] : vSettingsInvalid)
	{
		LocalContext.UpdateFromString(Setting.m_aCommand);

		ECollisionCheckResult Result = ECollisionCheckResult::ERROR;
		int CollidingLineIndex = LocalContext.CheckCollision(Setting.m_aCommand, m_LoadedMapSettings.m_vSettingsValid, Result);
		int RealCollidingLineIndex = CollidingLineIndex;

		if(CollidingLineIndex != -1)
			RealCollidingLineIndex = std::find_if(vLoadedMapSettings.begin(), vLoadedMapSettings.end(), [&](const CEditorMapSetting &MapSetting) {
				return str_comp_nocase(MapSetting.m_aCommand, m_LoadedMapSettings.m_vSettingsValid.at(CollidingLineIndex).m_aCommand) == 0;
			}) - vLoadedMapSettings.begin();

		int Type = 0;
		if(!Valid)
			Type |= SInvalidSetting::TYPE_INVALID;
		if(Result == ECollisionCheckResult::REPLACE)
			Type |= SInvalidSetting::TYPE_DUPLICATE;

		m_LoadedMapSettings.m_vSettingsInvalid.emplace_back(Index, Setting.m_aCommand, Type, RealCollidingLineIndex, !Valid || !LocalContext.CommandIsValid());
		if(Type & SInvalidSetting::TYPE_DUPLICATE)
			m_LoadedMapSettings.m_SettingsDuplicate[RealCollidingLineIndex].emplace_back(m_LoadedMapSettings.m_vSettingsInvalid.size() - 1);

		LocalContext.Reset();
	}

	if(!m_LoadedMapSettings.m_vSettingsInvalid.empty())
		Editor()->m_Dialog = DIALOG_MAPSETTINGS_ERROR;
}

 

void CMapSettingsBackend::InitValueLoaders()
{
	 
	RegisterLoader("tune", SValueLoader::LoadTuneValues);
	RegisterLoader("tune_zone", SValueLoader::LoadTuneZoneValues);
	RegisterLoader("mapbug", SValueLoader::LoadMapBugs);
}

void SValueLoader::LoadTuneValues(const CSettingValuesBuilder &TuneBuilder)
{
	 
	LoadArgumentTuneValues(TuneBuilder.Argument(0));
}

void SValueLoader::LoadTuneZoneValues(const CSettingValuesBuilder &TuneZoneBuilder)
{
	 
	LoadArgumentTuneValues(TuneZoneBuilder.Argument(1));
}

void SValueLoader::LoadMapBugs(const CSettingValuesBuilder &BugBuilder)
{
	 
	auto ArgBuilder = BugBuilder.Argument(0);
	 
	ArgBuilder.Add("grenade-doubleexplosion@ddnet.tw");
}

void SValueLoader::LoadArgumentTuneValues(CArgumentValuesListBuilder &&ArgBuilder)
{
	 
	for(int i = 0; i < CTuningParams::Num(); i++)
	{
		ArgBuilder.Add(CTuningParams::Name(i));
	}
}
