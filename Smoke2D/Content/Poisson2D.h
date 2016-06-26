//--------------------------------------------------------------------------------------
// By Stars XU Tianchen
//--------------------------------------------------------------------------------------

#pragma once

#include "XSDXShader.h"
#include "XSDXState.h"
#include "XSDXBuffer.h"
#include "ShaderCommon2D.h"

class Poisson2D
{
public:
	Poisson2D(const XSDX::CPDXDevice &pd3dDevice, const XSDX::spShader &pShader);

	void Init(uint32_t iSimSize, uint32_t iStride);
	void SetShaders(uint8_t csJacobi);
	void SetShaders(uint8_t csDiv, uint8_t csJacobi);
	void ComputeDivergence(const XSDX::CPDXShaderResourceView &srvSource);
	void SolvePoisson(uint8_t iIteration);
	void SwapBuffers();

	const XSDX::spStructuredBuffer	&GetKnown() const;
	const XSDX::spStructuredBuffer	&GetResult() const;
protected:
	void jacobi();

	XSDX::spStructuredBuffer	m_pSBKnown;
	XSDX::spStructuredBuffer	m_pSBUnknown;

	uint8_t						m_iCSDiv;
	uint8_t						m_iCSJacobi;

	uint32_t					m_iSimSize;

	XSDX::spShader				m_pShader;
	XSDX::CPDXDevice			m_pd3dDevice;
	XSDX::CPDXContext			m_pd3dContext;
};

using upPoisson2D = std::unique_ptr<Poisson2D>;
using spPoisson2D = std::shared_ptr<Poisson2D>;
