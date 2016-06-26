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
RWStructuredBuffer<floatV>	g_rwVector		: register(u0);
StructuredBuffer<floatV>	g_roVector		: register(t0);
StructuredBuffer<float2>	g_roVelocity	: register(t1);

//--------------------------------------------------------------------------------------
// bilinear interpolation for 4-neighborhood sampling 
//--------------------------------------------------------------------------------------
halfV Bilinear(StructuredBuffer<floatV> roSb, half2 vTex)
{
	const half2 vfFloor = floor(vTex);
	const half2 vParam = vTex - vfFloor;
	const uint2 vMaxIdx = m_vSimSize.xy - 1;
	const uint2 vFloor = min(vfFloor, vMaxIdx);
	const uint2 vCeil = min(vFloor + 1, vMaxIdx);

	const halfV vSample00 = roSb[IDX2(vFloor.x, vFloor.y)];
	const halfV vSample01 = roSb[IDX2(vCeil.x, vFloor.y)];
	const halfV vSample10 = roSb[IDX2(vFloor.x, vCeil.y)];
	const halfV vSample11 = roSb[IDX2(vCeil.x, vCeil.y)];

	const halfV vBlend0 = lerp(vSample00, vSample01, vParam.x);
	const halfV vBlend1 = lerp(vSample10, vSample11, vParam.x);
	return lerp(vBlend0, vBlend1, vParam.y);
}

//--------------------------------------------------------------------------------------
// Advection
//--------------------------------------------------------------------------------------
[numthreads(THREAD_BLOCK_SIZE, 1, 1)]
void main(uint3 DTid : SV_DispatchThreadID)
{
	// Current location
	const half2 vU = g_roVelocity[DTid.x];

	half2 vPos = { DTid.x % m_vSimSize.x, DTid.x / m_vSimSize.x };
	vPos -= vU * m_fDeltaTime;

	halfV vAdv = Bilinear(g_roVector, vPos);
	//floatV vAdv = g_roVector[IND2V(uint2(vPos))];
	//g_rwVector[IND2V(uint2(vPos))] = g_roVector[DTid.x];
	g_rwVector[DTid.x] = g_vDissipation * vAdv;
}
