#pragma once
#include <Utils/VectorMath.h>

namespace Phoenix
{
	class Camera
	{
	public:
		Camera();
		virtual ~Camera() {}
		void SetPerspective(float fov_degrees, float aspect, float near_clip, float far_clip);

	public:
		void            Pan(float dx, float dz);
		void            Zoom(float dy);
		void            LookAt(const float3& eye, const float3& center, const float3& up);
		void            Rotate(float dx, float dz);

		void            GetInverseViewMatrix(float4x4&) const;
		const float4x4& GetViewMatrix() const { return m_view_matrix; }
		const float4x4& GetProjectionMatrix() const { return m_projection_matrix; }
		const float4x4& GetViewProjectionMatrix() const { return m_view_projection_matrix; }

		float           DistanceTo(const float3& world_point) const;
		float3          WorldToViewPoint(const float3& world_point) const;
		float3          WorldToViewVector(const float3& world_vector) const;

		float           GetViewPlaneWidth()const { return m_view_plane_width; }
		float           GetViewPlaneHeight()const { return m_view_plane_height; }

		const float3&   GetEyePosition() const { return m_eye_position; }
		const float3&   GetLookVector()const { return mLookVector; }
		const float3&   GetRightVector()const { return mRightVector; }
		const float3&   GetUpVector()const { return mUpVector; }

		float GetNearPlane()const { return mFocalLength; }
		const float2& GetSpherical()const { return mSpherical; }

		void SetFocalLength(uint32_t l);
		void SetFStop(float stop);
		void SetFocusDistance(float distance) { mFocusDistance = distance; }
		void SetShutterSpeed(float speed) { mShutterSpeed = speed; }
		void SetSensorDimension(uint32_t sensorWidth, const uint2& resolution);

		uint32_t GetFocalLength() { return mFocalLength; }
		float    GetFocusDistance() { return mFocusDistance; }
		float    GetFStop() { return mFStop; }
		float    GetShutterSpeed() { return mShutterSpeed; }
		float    GetLenseDiameter() { return mLenseDiameter;}

		const uint2& GetSensorDimension() { return mSensorDimension; }
		const uint2& GetSensorResolution() { return mSensorResolution; }

	protected:
		void            LookAtSpherical(const float3& up);

	protected:
		float           m_view_plane_width;
		float           m_view_plane_height;

		float           m_fov_degrees = 60.0f;
		float           m_aspect = 0;
		float3          m_eye_position = float3(0, 0, 0);

		mutable float4x4  m_view_matrix;
		mutable float4x4  m_projection_matrix;
		mutable float4x4  m_view_projection_matrix;

		float3 mLookVector, mRightVector, mUpVector;
		float2 mSpherical;

		//thin lense
		uint32_t mFocalLength = 1;
		float    mFocusDistance;//distance to the focal plane from sensor
		float    mFStop;
		float    mShutterSpeed;
		float    mLenseDiameter;//(lense_diameter / focal_length) = (1 / F-Stop)float m

		uint2 mSensorDimension;//Sensor height is calculated by accounting image aspect ratio (vertical film size = horizontal film size / aspect ratio).
		uint2 mSensorResolution;

	private:
		inline void RecomputeLenseDiameter()
		{
			mLenseDiameter = mFocalLength / mFStop;
		}
	};
}
