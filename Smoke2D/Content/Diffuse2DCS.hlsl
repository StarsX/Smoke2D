//--------------------------------------------------------------------------------------
// By Stars XU Tianchen
//--------------------------------------------------------------------------------------

//--------------------------------------------------------------------------------------
// Type definations
//--------------------------------------------------------------------------------------
typedef float2	floatV;
typedef half2	halfV;

#include "Jacobi2DCS.hlsli"

//--------------------------------------------------------------------------------------
// Definitions
//--------------------------------------------------------------------------------------
#define	VISCOSITY	0.15
//static const float g_fCellsPerUnit = 1.0 / CELL_SIZE;

//--------------------------------------------------------------------------------------
// Diffusion
//--------------------------------------------------------------------------------------
[numthreads(THREAD_BLOCK_SIZE, 1, 1)]
void main(uint3 DTid : SV_DispatchThreadID)
{
	// Current location
	const uint2 vPos = { DTid.x % m_vSimSize.x, DTid.x / m_vSimSize.x };

	// Jacobi iteration
	const half fAlpha = 1.0 / (VISCOSITY * m_fDeltaTime);
	const half fBeta = 4.0 + fAlpha;
	const half2 vf = { fAlpha, fBeta };
	const uint3 vTex = { vPos, DTid.x };

	g_rwUnknown[DTid.x] = Jacobi(vf, vTex);
}
