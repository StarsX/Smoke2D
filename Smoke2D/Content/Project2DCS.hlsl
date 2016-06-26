//--------------------------------------------------------------------------------------
// By Stars XU Tianchen
//--------------------------------------------------------------------------------------

#include "Gradient2DCS.hlsli"

//--------------------------------------------------------------------------------------
// Structured Buffers
//--------------------------------------------------------------------------------------
RWStructuredBuffer<float2>	g_rwVelocity	: register(u0);
StructuredBuffer<float>		g_roPressure	: register(t0);

//--------------------------------------------------------------------------------------
// Projection
//--------------------------------------------------------------------------------------
[numthreads(THREAD_BLOCK_SIZE, 1, 1)]
void main(uint3 DTid : SV_DispatchThreadID)
{
	// Current location
	const uint2 vPos = { DTid.x % m_vSimSize.x, DTid.x / m_vSimSize.x };

	// Project the velocity onto its divergence-free component
	g_rwVelocity[DTid.x] -= Gradient2D(g_roPressure, vPos);
}
