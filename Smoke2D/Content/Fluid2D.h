//--------------------------------------------------------------------------------------
// By Stars XU Tianchen
//--------------------------------------------------------------------------------------

#pragma once

#include "SharedMacros.h"
#include "Poisson2D.h"

#define VISC_ITERATION	0
#define	PRESS_ITERATION	20

class Fluid2D
{
public:
	__declspec(align(16)) struct CBUserConstants
	{
		DirectX::XMFLOAT4	vForce;
		FLOAT				fDeltaTime;
	};

	Fluid2D(const XSDX::CPDXDevice &pd3dDevice, const XSDX::spShader &pShader, const XSDX::spState &pState);

	void Init(float fWidth, float fHeight);
	void Init(uint32_t iWidth, uint32_t iHeight);
	void Simulate(
		float fDeltaTime,
		const XSDX::CPDXShaderResourceView &pImpulseSRV,
		uint8_t iItPress = PRESS_ITERATION,
		uint8_t iItVisc = VISC_ITERATION
		);
	void Simulate(
		float fDeltaTime,
		DirectX::XMFLOAT4 vForce = DirectX::XMFLOAT4(0, 0, 0, 0),
		uint8_t iItPress = PRESS_ITERATION,
		uint8_t iItVisc = VISC_ITERATION
		);
	void Render(const XSDX::CPDXShaderResourceView &pDepthSRV);
	void Render();
protected:
	void createConstBuffers();
	void advect();
	void advectVelocity();
	void advectDye();
	void diffuse(uint8_t iIteration);
	void diffuseVelocity(uint8_t iIteration);
	void diffuseDye();
	void impulse(const XSDX::CPDXShaderResourceView &pImpulseSRV);
	void impulse(DirectX::XMFLOAT4 &vForce);
	void impulseVelocity();
	void impulseDye();
	void project(uint8_t iIteration);
	void project();
	void boundary();

	XSDX::spStructuredBuffer	m_pSBVelocity;
	XSDX::spStructuredBuffer	m_pSBAdvVelocity;
	XSDX::spStructuredBuffer	m_pSBDye;
	XSDX::spStructuredBuffer	m_pSBAdvDye;
	
	uint32_t					m_iSimSize;

	DirectX::XMUINT2			m_vViewport;

	upPoisson2D					m_pDiffuse;
	upPoisson2D					m_pPressure;

	XSDX::spShader				m_pShader;
	XSDX::spState				m_pState;

	XSDX::CPDXBuffer			m_pCBImmutable;
	XSDX::CPDXBuffer			m_pCBUser;

	XSDX::CPDXDevice			m_pd3dDevice;
	XSDX::CPDXContext			m_pd3dContext;
};

using upFluid2D = std::unique_ptr<Fluid2D>;
using spFluid2D = std::shared_ptr<Fluid2D>;
using vuFluid2D = std::vector<upFluid2D>;
using vpFluid2D = std::vector<spFluid2D>;
