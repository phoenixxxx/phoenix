#include "MaterialEditor.h"
#include <Window/Window.h>
#include <Utils/Math.h>
#include <Utils/Console.h>
#include <Utils/Utils.h>
#include <Utils/VectorMath.h>

#include <Window/Window.h>
#include <D3D11/D3D11Manager.h>
#include <Utils/D3D11imgui.h>
#include <Image/Image.h>

#include <Material/Material.h>
#include <Utils/CSVDocument.h>
#include <Utils/CPUTimer.h>
#include <Utils/BlockIO.h>


namespace Phoenix
{
	MaterialEditor::MaterialEditor()
	{
		mCamera.LookAt(float3(10, 10, 10), float3(0, 0, 0), float3(0, 0, 1));
		mMouseClickX = -1;
		mMouseClickY = -1;
	}

	cstr_t MaterialEditor::Name()
	{
		return "MaterialEditor";
	}

	MaterialEditor::~MaterialEditor()
	{
	}

	//CSVDocument<2> doc("test.csv", { "Theta", "Phi" });
	bool MaterialEditor::Initialize()
	{
		HRESULT hr;
		cstr_t shaderPath = "Assets/Shaders/MaterialEvaluation.hlsl";
		std::vector<byte_t> shaderCode = LoadFile(shaderPath);
		if (shaderCode.size() != 0)
		{
			ID3DBlob* blob;
			//vertex shader
			blob = Phoenix::D3D11Manager::Instance()->CompileShader(shaderCode, shaderPath, "Evaluate", "cs_5_0");
			//hr = Phoenix::D3D11Manager::Instance()->GetDevice()->CreateVertexShader(blob->GetBufferPointer(), blob->GetBufferSize(), nullptr, &mVertexShader);
		}
#if 0
		auto albedoSamplerNode = MaterialSystem::Instance()->CreateNode(eNodeTypes::eImageSampler2D);
		auto normalNode = MaterialSystem::Instance()->CreateNode(eNodeTypes::eObjNormal);
		auto roughnessNode = MaterialSystem::Instance()->CreateNode(eNodeTypes::eFloat);
		auto bsdfNode = MaterialSystem::Instance()->CreateNode(eNodeTypes::eDiffseBRDF);
		auto texCoordNode = MaterialSystem::Instance()->CreateNode(eNodeTypes::eObjTexcoord0);

		MaterialSystem::Instance()->SetFloat(roughnessNode, 0.5f);

		MaterialSystem::Instance()->SetSamplerTexcoords(albedoSamplerNode, texCoordNode);
		MaterialSystem::Instance()->SetSamplerAddressing(albedoSamplerNode, eImageSampler2DAddressing::eClamp);
		MaterialSystem::Instance()->SetSamplerFiltering(albedoSamplerNode, eImageSampler2DFilter::eLinear);

		MaterialSystem::Instance()->SetDiffuseBRDFAlbedo(bsdfNode, albedoSamplerNode);
		MaterialSystem::Instance()->SetDiffuseBRDFNormal(bsdfNode, normalNode);
		MaterialSystem::Instance()->SetDiffuseBRDFRoughness(bsdfNode, roughnessNode);

		const auto& rgb = MaterialSystem::Instance()->Execute(bsdfNode);


		//math nodes
		auto aNode = MaterialSystem::Instance()->CreateNode(eNodeTypes::eFloat);
		auto bNode = MaterialSystem::Instance()->CreateNode(eNodeTypes::eFloat);
		auto sinNode = MaterialSystem::Instance()->CreateNode(eNodeTypes::eSin);
		auto mulNode = MaterialSystem::Instance()->CreateNode(eNodeTypes::eMul);

		MaterialSystem::Instance()->SetFloat(aNode, -0.9f);
		MaterialSystem::Instance()->SetFloat(bNode, 4.5f);
		MaterialSystem::Instance()->SetMathInputA(sinNode, aNode);
		MaterialSystem::Instance()->SetMathInputA(mulNode, sinNode);
		MaterialSystem::Instance()->SetMathInputB(mulNode, bNode);

		const auto& toto = MaterialSystem::Instance()->Execute(mulNode);

		float a = sinf(-0.9f) * 4.5f;
#endif

		return true;
	}

	void MaterialEditor::Run()
	{
		DrawUI();
	}

	void MaterialEditor::DrawUI()
	{
        ImGui::SetNextWindowSize(ImVec2(700, 600), ImGuiCond_FirstUseEver);
        if (!ImGui::Begin(Name()))
        {
            ImGui::End();
            return;
        }
        ImGui::End();
	}

	void MaterialEditor::MouseMove(const MouseInfo& info)
	{
	}

	void MaterialEditor::MouseClick(const MouseInfo& info)
	{
	}

	void MaterialEditor::MouseDrag(const MouseInfo& info)
	{
	}

	void MaterialEditor::MouseWheel(const MouseInfo& info)
	{
	}

	void MaterialEditor::Resize(uint32_t width, uint32_t height)
	{
	}
}
