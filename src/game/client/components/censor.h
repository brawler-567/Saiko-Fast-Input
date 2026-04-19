#ifndef GAME_CLIENT_COMPONENTS_CENSOR_H
#define GAME_CLIENT_COMPONENTS_CENSOR_H
 

#include <game/client/component.h>

class CCensor : public CComponent
{
public:
	int Sizeof() const override { return sizeof(*this); }
};

#endif  
