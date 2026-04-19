#ifndef GAME_EDITOR_SMOOTH_VALUE_H
#define GAME_EDITOR_SMOOTH_VALUE_H

#include "component.h"

#include <base/bezier.h>

 
class CSmoothValue : public CEditorComponent
{
public:
	CSmoothValue(float InitialValue, float MinValue, float MaxValue);

	 
	void SetValue(float Target);

	 
	void ChangeValue(float Amount);

	 
	void SetValueInstant(float Target);

	bool UpdateValue();

	float GetValue() const;
	void SetValueRange(float MinValue, float MaxValue);
	float GetMinValue() const;
	float GetMaxValue() const;

private:
	float ZoomProgress(float CurrentTime) const;

	bool m_Smoothing;
	float m_Value;
	CCubicBezier m_ValueSmoothing;
	float m_ValueSmoothingTarget;
	float m_ValueSmoothingStart;
	float m_ValueSmoothingEnd;

	float m_MinValue;
	float m_MaxValue;
};

#endif
