//--------------------------------------------------------------------------------------
// By Stars XU Tianchen
//--------------------------------------------------------------------------------------

#include "ConstantsCH.hlsli"

//--------------------------------------------------------------------------------------
// Compute gradient
//--------------------------------------------------------------------------------------
half2 Gradient2D(StructuredBuffer<float> roSb, uint2 vTex)
{
	const uint2 vTexPos = min(vTex + 1, m_vSimSize.xy);
	
	const half fLeft = roSb[IDX2(vTex.x - 1, vTex.y)];
	const half fRight = roSb[IDX2(vTexPos.x, vTex.y)];
	const half fTop = roSb[IDX2(vTex.x, vTex.y - 1)];
	const half fBottom = roSb[IDX2(vTex.x, vTexPos.y)];
	
	// Compute the velocity's divergence using central differences
	return 0.5 * half2(fRight - fLeft, fBottom - fTop);
}
