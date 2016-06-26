//--------------------------------------------------------------------------------------
// By Stars XU Tianchen
//--------------------------------------------------------------------------------------

#include "SharedMacros.h"

#define IDX2V(P)	((P.x) + (P.y) * vViewport.x)
#define IDX2(X, Y)	((X) + (Y) * vViewport.x)

//--------------------------------------------------------------------------------------
// Constant buffers
//--------------------------------------------------------------------------------------
cbuffer CBEnvironment : register(b0)
{
	float2	m_vViewport;
};

//--------------------------------------------------------------------------------------
// Structured Buffers
//--------------------------------------------------------------------------------------
StructuredBuffer<float4>	g_roDye			: register(t0);

//--------------------------------------------------------------------------------------
// bilinear interpolation for 4-neighborhood sampling 
//--------------------------------------------------------------------------------------
half4 Bilinear(StructuredBuffer<float4> roSb, half2 vTex)
{
	static const half2 vViewport = m_vViewport * SIM_SCALE;
	const half2 vfFloor = floor(vTex);
	const half2 vParam = vTex - vfFloor;
	const uint2 vFloor = min(vfFloor, vViewport);
	const uint2 vCeil = min(vFloor + 1, vViewport);

	const half4 vSample00 = roSb[IDX2(vFloor.x, vFloor.y)];
	const half4 vSample01 = roSb[IDX2(vCeil.x, vFloor.y)];
	const half4 vSample10 = roSb[IDX2(vFloor.x, vCeil.y)];
	const half4 vSample11 = roSb[IDX2(vCeil.x, vCeil.y)];

	const half4 vBlend0 = lerp(vSample00, vSample01, vParam.x);
	const half4 vBlend1 = lerp(vSample10, vSample11, vParam.x);
	return lerp(vBlend0, vBlend1, vParam.y);
}

half4 main(float4 Pos : SV_POSITION) : SV_TARGET
{
	static const half3 vDye = { 0, 0, 1.0 };
	static const half3 vCanvas = { 1.0, 1.0, 1.0 };
	half2 vTex = Pos.xy * SIM_SCALE;
	half fDens = saturate(Bilinear(g_roDye, vTex)).x;
	return half4(lerp(vCanvas, vDye, fDens), 1.0);
}
