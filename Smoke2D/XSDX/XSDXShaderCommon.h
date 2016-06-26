//--------------------------------------------------------------------------------------
// By Stars XU Tianchen
//--------------------------------------------------------------------------------------

#pragma once

namespace XSDX
{
#ifndef	BASIC_SHADER_STRUCTURES
#define	BASIC_SHADER_STRUCTURES

	// Constant buffer used to send MVP matrices to the vertex shader.
#ifndef	CB_MATRICES
#define CB_MATRICES
	struct CBMatrices
	{
		DirectX::XMMATRIX	m_mWorldViewProj;
		DirectX::XMMATRIX	m_mWorld;
		DirectX::XMMATRIX	m_mNormal;
		DirectX::XMMATRIX	m_mShadow;
	};
	using LPCBMatrices = std::add_pointer_t<CBMatrices>;
#endif

#ifndef	CB_IMMUTABLE
#define	CB_IMMUTABLE
	struct CBImmutable
	{
		DirectX::XMFLOAT4	m_vDirectional;
		DirectX::XMFLOAT4	m_vAmbient;
	};
#endif

#ifndef	CB_GLOBAL
#define	CB_GLOBAL
	struct CBGlobal
	{
		DirectX::XMVECTOR	m_vLightPt;
		DirectX::XMVECTOR	m_vEyePt;
		DirectX::XMVECTOR	m_vLookAtPt;
		DirectX::XMFLOAT4	m_vViewport;
	};
#endif

#endif

#ifndef	BASIC_SHADER_IDS
#define	BASIC_SHADER_IDS
	// Vertex shaders
	static auto				g_uVSScreenQuad	(0ui8);
	static auto				g_uVSBasePass	(0ui8);
	static auto				g_uVSPostproc	(0ui8);
	static auto				g_uVSDepth		(1ui8);
	static auto				g_uVSSkinning	(2ui8);
	static auto				g_uVSBound		(5ui8);

	// Geometry shaders
	static auto				g_uGSSkinning	(2ui8);

	// Pixel shaders
	static auto				g_uPSShade		(0ui8);
	static auto				g_uPSBasePass	(0ui8);
	static auto				g_uPSSSReflect	(0ui8);
	static auto				g_uPSPostproc	(0ui8);
	static auto				g_uPSDepth		(1ui8);
	static auto				g_uPSToneMap	(1ui8);
	
	static auto				g_uPSFXAA		(2ui8);
	static auto				g_uPSOcclusion	(2ui8);
	static auto				g_uPSResample	(3ui8);
	static auto				g_uPSReflect	(3ui8);
	static auto				g_uPSBound		(5ui8);

	// Compute shaders
	static auto				g_uCSLumAdapt	(0ui8);
#endif

#ifndef	BASIC_SHADER_SLOTS
#define	BASIC_SHADER_SLOTS
	// Constant slots
	static const auto		g_uCBBound		(3ui8);
#endif
}
