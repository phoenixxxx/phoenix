#pragma once
#include <memory>
#include <Utils/Types.h>
#include <Utils/Math.h>
#include <Utils/VectorMath.h>
#include <Image/Image.h>
#include <SharedGPU_CPU/SharedMaterial.h>

namespace Phoenix
{
	class MaterialNode
	{
	public:
		class Plug
		{
		public:
			Plug(MaterialNode* node, uint32_t index):mNode(node), mSlot(nullptr), mIndex(index) {}
			bool operator==(const Plug& rhs)const { return (this == &rhs); }
			bool operator==(const std::nullptr_t& rhs)const { return (mSlot == nullptr); }
			bool operator!=(const std::nullptr_t& rhs)const { return (mSlot != nullptr); }

			MaterialNode* mNode;
			Plug* mSlot;

			uint32_t mIndex;//the index of this plug
		};

		enum class Type {
			eTexture2D,

			eScalar,
			eVector,

			//shaders
			eDiffuseBSDF,
		};

		virtual ~MaterialNode() {}
		virtual Type GetType() = 0;

	public:
		static void ConnectPlugs(Plug& p0, Plug& p1)
		{
			p0.mSlot = &p1;
			p1.mSlot = &p0;
		}
	};

	class MaterialNodeDiffuseBSDF : public MaterialNode
	{
		friend class MaterialSystem;
		MaterialNodeDiffuseBSDF():mAlbedoPlug(this, 0), mNormalPlug(this, 1), mRoughnessPlug(this, 2), mBSDFPlug(this, 3) {}
	public:
		virtual Type GetType() { return Type::eDiffuseBSDF; }

	public:
		Plug& GetAlbedoPlug() { return mAlbedoPlug; }
		Plug& GetNormalPlug() { return mNormalPlug; }
		Plug& GetRoughnessPlug() { return mRoughnessPlug; }
		Plug& GetBSDFPlug() { return mBSDFPlug; }
	private:
		Plug mAlbedoPlug;
		Plug mNormalPlug;
		Plug mRoughnessPlug;
		Plug mBSDFPlug;
	};

	class MaterialNodeScalar : public MaterialNode
	{
		friend class MaterialSystem;
		MaterialNodeScalar() :mValue(0), mValuePlug(this, 0) {}
	public:
		virtual Type GetType() { return Type::eScalar; }
	public:
		const float& GetValue() { return mValue; }
		void SetValue(const float& value) { mValue = value; }

		Plug& GetValuePlug() { return mValuePlug; }

	private:
		float mValue;
		Plug mValuePlug;
	};

	//Value nodes
	class MaterialNodeVector : public MaterialNode
	{
		friend class MaterialSystem;
		MaterialNodeVector():mValue(0, 0, 0, 1), mValuePlug(this, 0) {}
	public:
		virtual Type GetType() { return Type::eVector; }
	public:
		const float4& GetValue() { return mValue; }
		void SetValue(const float4& value) { mValue = value; }

		Plug& GetValuePlug() { return mValuePlug; }
	private:
		float4 mValue;
		Plug mValuePlug;
	};

	//Texturing
	class MaterialNodeTexture2D : public MaterialNode
	{
		friend class MaterialSystem;
		MaterialNodeTexture2D():mAddressing(Image::Boundaries::eClamp), mFiltering(Image::Filter::eLinear), mImage(nullptr),
			mTexCoordsPlug(this, 0), mColorPlug(this, 1), mAlphaPlug(this, 2){}
	public:
		virtual Type GetType() { return Type::eTexture2D; }
	public:
		Image::Boundaries GetAddressing() { return mAddressing; }
		void              SetAddressing(Image::Boundaries addressing) { mAddressing = addressing; }
		Image::Filter     GetFiltering() { return mFiltering; }
		void              SetFiltering(Image::Filter filter) { mFiltering = filter; }

		Image*            GetImage() { return mImage; }
		void              SetImage(Image* image) { mImage = image; }
	private:
		Image::Boundaries mAddressing;
		Image::Filter mFiltering;
		Image* mImage;

	public:
		Plug& GetTexCoordsPlug() { return mTexCoordsPlug; }
		Plug& GetColorPlug() { return mColorPlug; }
		Plug& GetAlphaPlug() { return mAlphaPlug; }

	private:
		Plug mTexCoordsPlug;
		Plug mColorPlug;
		Plug mAlphaPlug;
	};
}
