#include "bezier.h"

CCubicBezier CCubicBezier::With(float Start, float StartDerivative, float EndDerivative, float End)
{
	return CCubicBezier(Start, Start + StartDerivative / 3, End - EndDerivative / 3, End);
}

 
float CCubicBezier::Evaluate(float t) const
{
	return (1 - t) * (1 - t) * (1 - t) * a + 3 * (1 - t) * (1 - t) * t * b + 3 * (1 - t) * t * t * c + t * t * t * d;
}

 
float CCubicBezier::Derivative(float t) const
{
	return 3 * (1 - t) * (1 - t) * (b - a) + 6 * (1 - t) * t * (c - b) + 3 * t * t * (d - c);
}
