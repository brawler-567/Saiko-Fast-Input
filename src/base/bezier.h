#ifndef BASE_BEZIER_H
#define BASE_BEZIER_H

 
 
 
 
 
class CCubicBezier
{
	float a;
	float b;
	float c;
	float d;
	constexpr CCubicBezier(float a_, float b_, float c_, float d_) :
		a(a_), b(b_), c(c_), d(d_)
	{
	}

public:
	constexpr CCubicBezier() = default;
	float Evaluate(float t) const;
	float Derivative(float t) const;
	static CCubicBezier With(float Start, float StartDerivative, float EndDerivative, float End);
};

#endif  
