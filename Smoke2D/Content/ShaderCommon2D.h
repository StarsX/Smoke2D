//--------------------------------------------------------------------------------------
// By Stars XU Tianchen
//--------------------------------------------------------------------------------------

#pragma once

namespace ShaderIDs
{
	// Vertex shaders
	static auto			g_iVSFluid2D	(0ui8);

	// Pixel shaders
	static auto			g_iPSFluid2D	(0ui8);

	// Compute shaders
	static auto			g_iCSAdvect		(0ui8);
	static auto			g_iCSDiffuse	(1ui8);
	static auto			g_iCSImpulse	(2ui8);
	static auto			g_iCSDiv		(3ui8);
	static auto			g_iCSPressure	(4ui8);
	static auto			g_iCSProject	(5ui8);
	static auto			g_iCSAdvectDye	(6ui8);
	static auto			g_iCSImpulseDye	(7ui8);
	static auto			g_iCSBoundary	(8ui8);
}

namespace ShaderSlots
{
	// Constant slots
	static const auto	g_iCBGlobal2D	(1ui8);

	// Texture slots
	static const auto	g_iSRField		(0ui8);
	static const auto	g_iSRDye		(1ui8);
	static const auto	g_iSRFluidDepth	(1ui8);

	// UAV slot
	static const auto	g_iUASlot		(0ui8);
}
