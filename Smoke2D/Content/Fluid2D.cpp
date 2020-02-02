//--------------------------------------------------------------------------------------
// By Stars XU Tianchen
//--------------------------------------------------------------------------------------

#include "pch.h"
#include "Fluid2D.h"

using namespace DirectX;
using namespace DX;
using namespace std;
using namespace ShaderIDs;
using namespace ShaderSlots;
using namespace XSDX;

const auto g_pNullSRV	= static_cast<ID3D11ShaderResourceView*>(nullptr);	// Helper to Clear SRVs
const auto g_pNullUAV	= static_cast<ID3D11UnorderedAccessView*>(nullptr);	// Helper to Clear UAVs
const auto g_iNullUint	= 0u;												// Helper to Clear Buffers

Fluid2D::Fluid2D(const CPDXDevice &pd3dDevice, const spShader &pShader, const spState &pState) :
	m_pd3dDevice(pd3dDevice),
	m_pShader(pShader),
	m_pState(pState)
{
	m_pDiffuse = make_unique<Poisson2D>(pd3dDevice, pShader);
	m_pPressure = make_unique<Poisson2D>(pd3dDevice, pShader);
	m_pDiffuse->SetShaders(g_iCSDiffuse);
	m_pPressure->SetShaders(g_iCSDiv, g_iCSPressure);
	m_pd3dDevice->GetImmediateContext(&m_pd3dContext);
}

void Fluid2D::Init(float fWidth, float fHeight)
{
	Init(static_cast<uint32_t>(fWidth), static_cast<uint32_t>(fHeight));
}

void Fluid2D::Init(uint32_t iWidth, uint32_t iHeight)
{
	m_vViewport.x = max(iWidth, 1u);
	m_vViewport.y = max(iHeight, 1u);
	m_iSimSize = m_vViewport.x * m_vViewport.y;
	createConstBuffers();

	// Create Structured Buffer
	m_pSBDye = make_unique<StructuredBuffer>(m_pd3dDevice);
	m_pSBAdvDye = make_unique<StructuredBuffer>(m_pd3dDevice);
	m_pSBDye->Create(m_iSimSize, sizeof(XMFLOAT4), D3D11_BIND_SHADER_RESOURCE | D3D11_BIND_UNORDERED_ACCESS);
	m_pSBAdvDye->Create(m_iSimSize, sizeof(XMFLOAT4), D3D11_BIND_SHADER_RESOURCE | D3D11_BIND_UNORDERED_ACCESS);

	m_pDiffuse->Init(m_iSimSize, sizeof(XMFLOAT2));
	m_pPressure->Init(m_iSimSize, sizeof(float));
	m_pSBVelocity = m_pDiffuse->GetResult();
	m_pSBAdvVelocity = m_pDiffuse->GetKnown();
}

void Fluid2D::Simulate(
	float fDeltaTime,
	const CPDXShaderResourceView &pImpulseSRV,
	uint8_t iItPress,
	uint8_t iItVisc
	)
{
	const auto cb = CBUserConstants{ XMFLOAT4A(0, 0, 0, 0), fDeltaTime };
	m_pd3dContext->UpdateSubresource(m_pCBUser.Get(), 0, nullptr, &cb, 0, 0);
	m_pd3dContext->CSSetConstantBuffers(g_iCBGlobal2D, 1, m_pCBUser.GetAddressOf());

	advect();
	diffuse(iItVisc);
	impulse(pImpulseSRV);
	project(iItPress);

	const auto pSRVs = array<ID3D11ShaderResourceView*, 2>{ { nullptr, nullptr } };
	m_pd3dContext->CSSetUnorderedAccessViews(g_iUASlot, 1, &g_pNullUAV, &g_iNullUint);
	m_pd3dContext->CSSetShaderResources(g_iSRField, 2, pSRVs.data());
}

void Fluid2D::Simulate(float fDeltaTime, XMFLOAT4 vForce, uint8_t iItPress, uint8_t iItVisc)
{
	const auto cb = CBUserConstants{ vForce, fDeltaTime };
	m_pd3dContext->UpdateSubresource(m_pCBUser.Get(), 0, nullptr, &cb, 0, 0);
	m_pd3dContext->CSSetConstantBuffers(g_iCBGlobal2D, 1, m_pCBUser.GetAddressOf());

	advect();
	diffuse(iItVisc);
	impulse(vForce);
	project(iItPress);

	const auto pSRVs = array<ID3D11ShaderResourceView*, 2>{ { nullptr, nullptr } };
	m_pd3dContext->CSSetUnorderedAccessViews(g_iUASlot, 1, &g_pNullUAV, &g_iNullUint);
	m_pd3dContext->CSSetShaderResources(g_iSRField, 2, pSRVs.data());
}

void Fluid2D::createConstBuffers()
{
	// Setup constant buffers
	auto desc = CD3D11_BUFFER_DESC(sizeof(CBUserConstants), D3D11_BIND_CONSTANT_BUFFER);
	ThrowIfFailed(m_pd3dDevice->CreateBuffer(&desc, nullptr, &m_pCBUser));

	desc.ByteWidth = sizeof(XMUINT4);
	desc.Usage = D3D11_USAGE_IMMUTABLE;
	const auto im = XMUINT4(m_vViewport.x, m_vViewport.y, 0, 0);
	const auto dsd = D3D11_SUBRESOURCE_DATA{ &im, 0, 0 };
	ThrowIfFailed(m_pd3dDevice->CreateBuffer(&desc, &dsd, &m_pCBImmutable));

	m_pd3dContext->CSSetConstantBuffers(0, 1, m_pCBImmutable.GetAddressOf());
}

void Fluid2D::Render(const CPDXShaderResourceView &pDepthSRV)
{
	m_pd3dContext->PSSetShaderResources(g_iSRFluidDepth, 1, pDepthSRV.GetAddressOf());

	Render();

	const auto pSRVs = array<ID3D11ShaderResourceView*, 2>{ { nullptr, nullptr } };
	m_pd3dContext->PSSetShaderResources(g_iSRField, 2, pSRVs.data());
}

void Fluid2D::Render()
{
	// Show velocity field
	m_pd3dContext->IASetPrimitiveTopology(D3D11_PRIMITIVE_TOPOLOGY_TRIANGLESTRIP);
	//m_pd3dContext->PSSetShaderResources(g_iSRField, 1, m_Pressure->GetResult().m_pSRView.GetAddressOf());
	//m_pd3dContext->PSSetShaderResources(g_iSRField, 1, m_sbVelocity.m_pSRView.GetAddressOf());
	m_pd3dContext->PSSetShaderResources(g_iSRField, 1, m_pSBDye->GetSRV().GetAddressOf());

	m_pd3dContext->VSSetShader(m_pShader->GetVertexShader(g_iVSFluid2D).Get(), nullptr, 0);
	m_pd3dContext->GSSetShader(nullptr, nullptr, 0);
	m_pd3dContext->PSSetShader(m_pShader->GetPixelShader(g_iVSFluid2D).Get(), nullptr, 0);
	//m_pd3dContext->PSSetSamplers(g_iSmpPointWrap, 1, m_pSamplerPointWrap.GetAddressOf());

	m_pd3dContext->Draw(3, 0);

	m_pd3dContext->VSSetShader(nullptr, nullptr, 0);
	m_pd3dContext->PSSetShader(nullptr, nullptr, 0);
	m_pd3dContext->RSSetState(0);
}

void Fluid2D::advect()
{
	advectVelocity();
	advectDye();
}

void Fluid2D::advectVelocity()
{
	const auto UAVInitialCounts = 0u;

	// Setup
	const auto pSRVs = array<ID3D11ShaderResourceView*, 2>{ {
		m_pSBVelocity->GetSRV().Get(),
		m_pSBVelocity->GetSRV().Get()
	} };
	m_pd3dContext->CSSetUnorderedAccessViews(g_iUASlot, 1, m_pSBAdvVelocity->GetUAV().GetAddressOf(), &UAVInitialCounts);
	m_pd3dContext->CSSetShaderResources(g_iSRField, 2, pSRVs.data());

	// Compute velocity advection
	m_pd3dContext->CSSetShader(m_pShader->GetComputeShader(g_iCSAdvect).Get(), nullptr, 0);
	m_pd3dContext->Dispatch(m_iSimSize / THREAD_BLOCK_SIZE, 1, 1);
}

void Fluid2D::advectDye()
{
	const auto UAVInitialCounts = 0u;

	// Setup
	const auto pSRVs = array<ID3D11ShaderResourceView*, 2>{ {
		m_pSBDye->GetSRV().Get(),
		m_pSBVelocity->GetSRV().Get()
	} };
	m_pd3dContext->CSSetUnorderedAccessViews(g_iUASlot, 1, m_pSBAdvDye->GetUAV().GetAddressOf(), &UAVInitialCounts);
	m_pd3dContext->CSSetShaderResources(g_iSRField, 2, pSRVs.data());

	// Compute dye advection
	m_pd3dContext->CSSetShader(m_pShader->GetComputeShader(g_iCSAdvectDye).Get(), nullptr, 0);
	m_pd3dContext->Dispatch(m_iSimSize / THREAD_BLOCK_SIZE, 1, 1);
}

void Fluid2D::diffuse(uint8_t iIteration)
{
	diffuseVelocity(iIteration);
	diffuseDye();
}

void Fluid2D::diffuseVelocity(uint8_t iIteration)
{
	if (iIteration > 0) {
		m_pd3dContext->CSSetShaderResources(g_iSRField, 1, &g_pNullSRV);
		m_pDiffuse->SolvePoisson(iIteration);
	}
	else {
		m_pDiffuse->SwapBuffers();
		m_pSBAdvVelocity = m_pDiffuse->GetKnown();
	}
	m_pSBVelocity = m_pDiffuse->GetResult();
}

void Fluid2D::diffuseDye()
{
	m_pSBDye.swap(m_pSBAdvDye);
}

void Fluid2D::impulse(const CPDXShaderResourceView &pImpulseSRV)
{
	m_pd3dContext->CSSetShaderResources(g_iSRField, 1, pImpulseSRV.GetAddressOf());

	impulseDye();
	impulseVelocity();

	m_pd3dContext->CSSetShaderResources(g_iSRField, 1, &g_pNullSRV);
}

void Fluid2D::impulse(XMFLOAT4 &vForce)
{
	if (vForce.x * vForce.x + vForce.y * vForce.y > 0.00001f) {
		impulseDye();
		impulseVelocity();
	}
}

void Fluid2D::impulseVelocity()
{
	const auto UAVInitialCounts = 0u;
	
	// Setup
	m_pd3dContext->CSSetUnorderedAccessViews(g_iUASlot, 1, m_pSBVelocity->GetUAV().GetAddressOf(), &UAVInitialCounts);
	m_pd3dContext->CSSetShaderResources(g_iSRDye, 1, m_pSBDye->GetSRV().GetAddressOf());

	// Add force
	m_pd3dContext->CSSetShader(m_pShader->GetComputeShader(g_iCSImpulse).Get(), nullptr, 0);
	m_pd3dContext->Dispatch(m_iSimSize / THREAD_BLOCK_SIZE, 1, 1);
}

void Fluid2D::impulseDye()
{
	const auto UAVInitialCounts = 0u;
	
	// Setup
	m_pd3dContext->CSSetUnorderedAccessViews(g_iUASlot, 1, m_pSBDye->GetUAV().GetAddressOf(), &UAVInitialCounts);

	// Add dye
	m_pd3dContext->CSSetShader(m_pShader->GetComputeShader(g_iCSImpulseDye).Get(), nullptr, 0);
	m_pd3dContext->Dispatch(m_iSimSize / THREAD_BLOCK_SIZE, 1, 1);
}

void Fluid2D::project(uint8_t iIteration)
{
	m_pPressure->ComputeDivergence(m_pSBVelocity->GetSRV());
	m_pPressure->SolvePoisson(iIteration);

	boundary();
	project();
	boundary();
}

void Fluid2D::project()
{
	const auto UAVInitialCounts = 0u;
	
	// Setup
	m_pd3dContext->CSSetUnorderedAccessViews(g_iUASlot, 1, m_pSBVelocity->GetUAV().GetAddressOf(), &UAVInitialCounts);
	m_pd3dContext->CSSetShaderResources(g_iSRField, 1, m_pPressure->GetResult()->GetSRV().GetAddressOf());

	// Compute projection
	m_pd3dContext->CSSetShader(m_pShader->GetComputeShader(g_iCSProject).Get(), nullptr, 0);
	m_pd3dContext->Dispatch(m_iSimSize / THREAD_BLOCK_SIZE, 1, 1);
}

void Fluid2D::boundary()
{
	const auto UAVInitialCounts = 0u;
	
	// Setup
	m_pd3dContext->CSSetUnorderedAccessViews(g_iUASlot, 1, m_pSBVelocity->GetUAV().GetAddressOf(), &UAVInitialCounts);

	// Compute boundary conditions
	m_pd3dContext->CSSetShader(m_pShader->GetComputeShader(g_iCSBoundary).Get(), nullptr, 0);
	m_pd3dContext->Dispatch(m_iSimSize / THREAD_BLOCK_SIZE, 1, 1);
}
