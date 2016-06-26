//--------------------------------------------------------------------------------------
// By Stars XU Tianchen
//--------------------------------------------------------------------------------------

#include "ConstantsCH.hlsli"

//--------------------------------------------------------------------------------------
// Structured Buffers
//--------------------------------------------------------------------------------------
RWStructuredBuffer<float2>	g_rwVelocity	: register(u0);

//--------------------------------------------------------------------------------------
// Boundary process
//--------------------------------------------------------------------------------------
[numthreads(THREAD_BLOCK_SIZE, 1, 1)]
void main(uint3 DTid : SV_DispatchThreadID)
{
	// Current location
	const int2 vMax = m_vSimSize.xy - 1;
	int2 vPos = { DTid.x % m_vSimSize.x, DTid.x / m_vSimSize.x };

	int2 vOffset = {
		vPos.x >= vMax.x ? -1 : (vPos.x <= 0 ? 1 : 0),
		vPos.y >= vMax.y ? -1 : (vPos.y <= 0 ? 1 : 0)
	};
	vPos += vOffset;

	if (vOffset.x || vOffset.y)
		g_rwVelocity[DTid.x] = -g_rwVelocity[IDX2V(vPos)];
}
