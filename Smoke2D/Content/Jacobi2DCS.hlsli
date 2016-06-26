//--------------------------------------------------------------------------------------
// By Stars XU Tianchen
//--------------------------------------------------------------------------------------

#include "ConstantsCH.hlsli"

//--------------------------------------------------------------------------------------
// Constants
//--------------------------------------------------------------------------------------
//static const float g_fCellsPerUnit = 1.0 / CELL_SIZE;

//--------------------------------------------------------------------------------------
// Structured Buffers
//--------------------------------------------------------------------------------------
RWStructuredBuffer<floatV>	g_rwUnknown	: register(u0);
StructuredBuffer<floatV>	g_roKnown	: register(t0);

//--------------------------------------------------------------------------------------
// Jacobi iteration
//--------------------------------------------------------------------------------------
floatV Jacobi(half2 vf, uint3 vTex)
{
	const uint2 vTexPos = min(vTex.xy + 1, m_vSimSize.xy);

	// Jacobi iteration
	const halfV fLeft = g_rwUnknown[IDX2(vTex.x - 1, vTex.y)];
	const halfV fRight = g_rwUnknown[IDX2(vTexPos.x, vTex.y)];
	const halfV fTop = g_rwUnknown[IDX2(vTex.x, vTex.y - 1)];
	const halfV fBottom = g_rwUnknown[IDX2(vTex.x, vTexPos.y)];
	const halfV fKnown = g_roKnown[vTex.z];

	return (fLeft + fRight + fTop + fBottom + vf.x * fKnown) / vf.y;
}
