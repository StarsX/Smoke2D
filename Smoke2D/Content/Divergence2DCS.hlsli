//--------------------------------------------------------------------------------------
// By Stars XU Tianchen
//--------------------------------------------------------------------------------------

#include "ConstantsCH.hlsli"

//--------------------------------------------------------------------------------------
// Compute divergence
//--------------------------------------------------------------------------------------
half Divergence2D(StructuredBuffer<float2> roSb, uint2 vTex)
{
	const uint2 vTexPos = min(vTex + 1, m_vSimSize.xy);

	const half fLeft = roSb[IDX2(vTex.x - 1, vTex.y)].x;
	const half fRight = roSb[IDX2(vTexPos.x, vTex.y)].x;
	const half fTop = roSb[IDX2(vTex.x, vTex.y - 1)].y;
	const half fBottom = roSb[IDX2(vTex.x, vTexPos.y)].y;

	// Take central differences of neighboring values
	return 0.5 * (fRight - fLeft + fBottom - fTop);
}
