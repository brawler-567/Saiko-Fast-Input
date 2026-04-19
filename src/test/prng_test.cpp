#include "test.h"

#include <game/prng.h>

#include <gtest/gtest.h>

 
 
 
 
 
 
 
 
 
 
 
 
 
 
 
 
 
 
 
 
 
 
 
 
 
 
 
 
 
 
 
 
 
 
 
 
 

static const unsigned int PCG32_GLOBAL_DEMO[] = {
	0xa15c02b7,
	0x7b47f409,
	0xba1d3330,
	0x83d2f293,
	0xbfa4784b,
	0xcbed606e,
};

TEST(Prng, EqualsPcg32GlobalDemo)
{
	uint64_t aSeed[2] = {42, 54};

	CPrng Prng;
	Prng.Seed(aSeed);
	for(auto Expected : PCG32_GLOBAL_DEMO)
	{
		EXPECT_EQ(Prng.RandomBits(), Expected);
	}
}

TEST(Prng, Description)
{
	CPrng Prng;
	EXPECT_STREQ(Prng.Description(), "pcg-xsh-rr:unseeded");

	uint64_t aSeed0[2] = {0xfedbca9876543210, 0x0123456789abcdef};
	uint64_t aSeed1[2] = {0x0123456789abcdef, 0xfedcba9876543210};
	uint64_t aSeed2[2] = {0x0000000000000000, 0x0000000000000000};

	Prng.Seed(aSeed0);
	EXPECT_STREQ(Prng.Description(), "pcg-xsh-rr:fedbca9876543210:0123456789abcdef");
	Prng.Seed(aSeed1);
	EXPECT_STREQ(Prng.Description(), "pcg-xsh-rr:0123456789abcdef:fedcba9876543210");
	Prng.Seed(aSeed2);
	EXPECT_STREQ(Prng.Description(), "pcg-xsh-rr:0000000000000000:0000000000000000");
}
