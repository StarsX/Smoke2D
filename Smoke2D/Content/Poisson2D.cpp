//--------------------------------------------------------------------------------------
// By Stars XU Tianchen
//--------------------------------------------------------------------------------------

#include "pch.h"
#include "SharedMacros.h"
#include "Poisson2D.h"

using namespace DirectX;
using namespace DX;
using namespace std;
using namespace ShaderSlots;
using namespace XSDX;

const auto g_pNullSRV = static_cast<ID3D11ShaderResourceView*>(nullptr);	// Helper to Clear SRVs
const auto g_pNullUAV = static_cast<ID3D11UnorderedAccessView*>(nullptr);	// Helper to Clear UAVs

Poisson2D::Poisson2D(const CPDXDevice &pd3dDevice, const spShader &pShader) :
	m_pd3dDevice(pd3dDevice),
	m_pShader(pShader),
	m_iCSDiv(0),
	m_iCSJacobi(1)
{
	m_pd3dDevice->GetImmediateContext(&m_pd3dContext);
}

void Poisson2D::Init(uint32_t iSimSize, uint32_t iStride)
{
	m_iSimSize = iSimSize;

	// Initialize data
	auto iByteWidth = iSimSize * iStride;
	auto vData = vbyte(iByteWidth);
	ZeroMemory(vData.data(), iByteWidth);

	// Create Structured Buffers
	m_pSBKnown = make_shared<StructuredBuffer>(m_pd3dDevice);
	m_pSBUnknown = make_shared<StructuredBuffer>(m_pd3dDevice);
	m_pSBKnown->Create(iSimSize, iStride, D3D11_BIND_SHADER_RESOURCE | D3D11_BIND_UNORDERED_ACCESS);
	m_pSBUnknown->Create(iSimSize, iStride, D3D11_BIND_SHADER_RESOURCE | D3D11_BIND_UNORDERED_ACCESS, vData.data());
}

void Poisson2D::SetShaders(uint8_t csJacobi)
{
	m_iCSJacobi = csJacobi;
}

void Poisson2D::SetShaders(uint8_t csDiv, uint8_t csJacobi)
{
	m_iCSDiv = csDiv;
	m_iCSJacobi = csJacobi;
}

void Poisson2D::ComputeDivergence(const CPDXShaderResourceView &srvSource)
{
	const auto UAVInitialCounts = 0u;

	// Setup
	m_pd3dContext->CSSetUnorderedAccessViews(g_iUASlot, 1, m_pSBKnown->GetUAV().GetAddressOf(), &UAVInitialCounts);
	m_pd3dContext->CSSetShaderResources(g_iSRField, 1, srvSource.GetAddressOf());

	// Compute Divergence
	m_pd3dContext->CSSetShader(m_pShader->GetComputeShader(m_iCSDiv).Get(), nullptr, 0);
	m_pd3dContext->Dispatch(m_iSimSize / THREAD_BLOCK_SIZE, 1, 1);

	// Unset
	m_pd3dContext->CSSetUnorderedAccessViews(g_iUASlot, 1, &g_pNullUAV, &UAVInitialCounts);
}

void Poisson2D::jacobi()
{
	const auto UAVInitialCounts = 0u;

	// Setup
	m_pd3dContext->CSSetUnorderedAccessViews(g_iUASlot, 1, m_pSBUnknown->GetUAV().GetAddressOf(), &UAVInitialCounts);

	// Jacobi iteration
	m_pd3dContext->CSSetShader(m_pShader->GetComputeShader(m_iCSJacobi).Get(), nullptr, 0);
	m_pd3dContext->Dispatch(m_iSimSize / THREAD_BLOCK_SIZE, 1, 1);
}

void Poisson2D::SolvePoisson(uint8_t iIteration)
{
	const auto UAVInitialCounts = 0u;

	// Setup
	m_pd3dContext->CSSetShaderResources(g_iSRField, 1, m_pSBKnown->GetSRV().GetAddressOf());

	// Clear buffer
	/*if (m_iCSClear != NULL_SHADER)
	{
		m_pd3dContext->CSSetUnorderedAccessViews(g_iUASlot, 1, m_sbSwaps[m_iResult].m_pUAView.GetAddressOf(), &UAVInitialCounts);
		m_pd3dContext->CSSetShader(m_pShader->GetComputeShader(m_iCSClear).Get(), nullptr, 0);
		m_pd3dContext->Dispatch(g_iSimGrid2D / THREAD_BLOCK_SIZE, 1, 1);
	}*/

	// Jacobi iterations
	for (auto i = 0ui8; i < iIteration; ++i) jacobi();

	// Unset
	m_pd3dContext->CSSetUnorderedAccessViews(g_iUASlot, 1, &g_pNullUAV, &UAVInitialCounts);
	m_pd3dContext->CSSetShaderResources(g_iSRField, 1, &g_pNullSRV);
}

void Poisson2D::SwapBuffers()
{
	m_pSBKnown.swap(m_pSBUnknown);
}

const spStructuredBuffer &Poisson2D::GetKnown() const
{
	return m_pSBKnown;
}

const spStructuredBuffer &Poisson2D::GetResult() const
{
	return m_pSBUnknown;
}
