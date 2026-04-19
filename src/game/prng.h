#ifndef GAME_PRNG_H
#define GAME_PRNG_H

#include <cstdint>

class CPrng
{
public:
	 
	CPrng();

	 
	const char *Description() const;

	 
	 
	 
	void Seed(uint64_t aSeed[2]);

	 
	 
	unsigned int RandomBits();

private:
	char m_aDescription[64];

	bool m_Seeded;
	uint64_t m_State;
	uint64_t m_Increment;
};

#endif  
