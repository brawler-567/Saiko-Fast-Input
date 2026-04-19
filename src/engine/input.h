 
 
#ifndef ENGINE_INPUT_H
#define ENGINE_INPUT_H

#include "kernel.h"

#include <base/types.h>
#include <base/vmath.h>

#include <chrono>
#include <cstdint>
#include <functional>
#include <string>
#include <vector>

class IInput : public IInterface
{
	MACRO_INTERFACE("input")
public:
	class CEvent
	{
	public:
		int m_Flags;
		int m_Key;
		uint32_t m_InputCount;
		char m_aText[32];  
	};

	enum
	{
		FLAG_PRESS = 1 << 0,
		FLAG_RELEASE = 1 << 1,
		FLAG_TEXT = 1 << 2,
		 
		FLAG_REPEAT = 1 << 3,
	};
	enum ECursorType
	{
		CURSOR_NONE,
		CURSOR_MOUSE,
		CURSOR_JOYSTICK,
	};

	 
	virtual void ConsumeEvents(std::function<void(const CEvent &Event)> Consumer) const = 0;
	 
	virtual void Clear() = 0;

	 
	virtual float GetUpdateTime() const = 0;

	 
	virtual bool ModifierIsPressed() const = 0;
	virtual bool ShiftIsPressed() const = 0;
	virtual bool AltIsPressed() const = 0;
	 
	virtual bool KeyIsPressed(int Key) const = 0;
	 
	virtual bool KeyPress(int Key) const = 0;
	virtual const char *KeyName(int Key) const = 0;
	virtual int FindKeyByName(const char *pKeyName) const = 0;

	 
	class IJoystick
	{
	public:
		virtual ~IJoystick() = default;
		virtual int GetIndex() const = 0;
		virtual const char *GetName() const = 0;
		virtual int GetNumAxes() const = 0;
		virtual int GetNumButtons() const = 0;
		virtual int GetNumBalls() const = 0;
		virtual int GetNumHats() const = 0;
		virtual float GetAxisValue(int Axis) = 0;
		virtual void GetHatValue(int Hat, int (&aHatKeys)[2]) = 0;
		virtual bool Relative(float *pX, float *pY) = 0;
		virtual bool Absolute(float *pX, float *pY) = 0;
	};
	virtual size_t NumJoysticks() const = 0;
	virtual IJoystick *GetJoystick(size_t Index) = 0;
	virtual IJoystick *GetActiveJoystick() = 0;
	virtual void SetActiveJoystick(size_t Index) = 0;

	 
	virtual vec2 NativeMousePos() const = 0;
	virtual bool NativeMousePressed(int Index) const = 0;
	virtual void MouseModeRelative() = 0;
	virtual void MouseModeAbsolute() = 0;
	virtual bool MouseRelative(float *pX, float *pY) = 0;

	 
	 
	class CTouchFinger
	{
		friend class CInput;

		int64_t m_DeviceId;
		int64_t m_FingerId;

	public:
		bool operator==(const CTouchFinger &Other) const { return m_DeviceId == Other.m_DeviceId && m_FingerId == Other.m_FingerId; }
		bool operator!=(const CTouchFinger &Other) const { return !(*this == Other); }
	};
	 
	class CTouchFingerState
	{
	public:
		 
		CTouchFinger m_Finger;
		 
		vec2 m_Position;
		 
		vec2 m_Delta;
		 
		std::chrono::nanoseconds m_PressTime;
	};
	 
	virtual const std::vector<CTouchFingerState> &TouchFingerStates() const = 0;
	 
	virtual void ClearTouchDeltas() = 0;

	 
	virtual std::string GetClipboardText() = 0;
	virtual void SetClipboardText(const char *pText) = 0;

	 
	virtual void StartTextInput() = 0;
	virtual void StopTextInput() = 0;
	virtual void EnsureScreenKeyboardShown() = 0;
	virtual const char *GetComposition() const = 0;
	virtual bool HasComposition() const = 0;
	virtual int GetCompositionCursor() const = 0;
	virtual int GetCompositionLength() const = 0;
	virtual const char *GetCandidate(int Index) const = 0;
	virtual int GetCandidateCount() const = 0;
	virtual int GetCandidateSelectedIndex() const = 0;
	virtual void SetCompositionWindowPosition(float X, float Y, float H) = 0;

	virtual bool GetDropFile(char *aBuf, int Len) = 0;

	ECursorType CursorRelative(float *pX, float *pY)
	{
		if(MouseRelative(pX, pY))
			return CURSOR_MOUSE;
		IJoystick *pJoystick = GetActiveJoystick();
		if(pJoystick && pJoystick->Relative(pX, pY))
			return CURSOR_JOYSTICK;
		return CURSOR_NONE;
	}
};

class IEngineInput : public IInput
{
	MACRO_INTERFACE("engineinput")
public:
	virtual void Init() = 0;
	void Shutdown() override = 0;
	virtual int Update() = 0;
};

extern IEngineInput *CreateEngineInput();

#endif
