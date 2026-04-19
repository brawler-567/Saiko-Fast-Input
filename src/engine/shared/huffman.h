 
 
#ifndef ENGINE_SHARED_HUFFMAN_H
#define ENGINE_SHARED_HUFFMAN_H

class CHuffman
{
	enum
	{
		HUFFMAN_EOF_SYMBOL = 256,

		HUFFMAN_MAX_SYMBOLS = HUFFMAN_EOF_SYMBOL + 1,
		HUFFMAN_MAX_NODES = HUFFMAN_MAX_SYMBOLS * 2 - 1,

		HUFFMAN_LUTBITS = 10,
		HUFFMAN_LUTSIZE = (1 << HUFFMAN_LUTBITS),
		HUFFMAN_LUTMASK = (HUFFMAN_LUTSIZE - 1)
	};

	struct CNode
	{
		 
		unsigned m_Bits;
		unsigned m_NumBits;

		 
		unsigned short m_aLeaves[2];

		 
		unsigned char m_Symbol;
	};

	static const unsigned ms_aFreqTable[HUFFMAN_MAX_SYMBOLS];

	CNode m_aNodes[HUFFMAN_MAX_NODES];
	CNode *m_apDecodeLut[HUFFMAN_LUTSIZE];
	CNode *m_pStartNode;
	int m_NumNodes;

	void Setbits_r(CNode *pNode, int Bits, unsigned Depth);
	void ConstructTree(const unsigned *pFrequencies);

public:
	 
	void Init(const unsigned *pFrequencies = ms_aFreqTable);

	 
	int Compress(const void *pInput, int InputSize, void *pOutput, int OutputSize) const;

	 
	int Decompress(const void *pInput, int InputSize, void *pOutput, int OutputSize) const;
};
#endif  
