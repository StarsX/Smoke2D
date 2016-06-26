//--------------------------------------------------------------------------------------
// By Stars XU Tianchen
//--------------------------------------------------------------------------------------

#include "ConstantsCH.hlsli"

//--------------------------------------------------------------------------------------
// Impulse
//--------------------------------------------------------------------------------------
half2 Displacement2D(unsigned int DTid)
{
	// Current location
	const half2 vPos = { DTid.x % m_vSimSize.x, DTid.x / m_vSimSize.x };

	return vPos - m_vFpos;
}

half Gaussian2D(half2 vDisp)
{
	static const half fFRad = 16.0;
	static const half fFRad2 = fFRad * fFRad * SIM_SCALE;

	return exp(-dot(vDisp, vDisp) / fFRad2);
}

//--------------------------------------------------------------------------------------
// Impulse
//--------------------------------------------------------------------------------------
half Impulse2D(unsigned int DTid)
{
	const half2 vDisp = Displacement2D(DTid);
	return Gaussian2D(vDisp);
}
