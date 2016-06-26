//--------------------------------------------------------------------------------------
// By Stars XU Tianchen
//--------------------------------------------------------------------------------------

#include "Divergence2DCS.hlsli"

//--------------------------------------------------------------------------------------
// Structured Buffers
//--------------------------------------------------------------------------------------
RWStructuredBuffer<float>	g_rwDivergence	: register(u0);
StructuredBuffer<float2>	g_roVector		: register(t0);

//--------------------------------------------------------------------------------------
// Compute divergence
//--------------------------------------------------------------------------------------
[numthreads(THREAD_BLOCK_SIZE, 1, 1)]
void main(uint3 DTid : SV_DispatchThreadID)
{
	// Current location
	const uint2 vPos = { DTid.x % m_vSimSize.x, DTid.x / m_vSimSize.x };

	g_rwDivergence[DTid.x] = Divergence2D(g_roVector, vPos);
}
