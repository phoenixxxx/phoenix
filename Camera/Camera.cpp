#include "Camera.h"
#include <Utils/Math.h>

namespace Phoenix {

    // =================================================================================================
    // Camera
    // =================================================================================================
    Camera::Camera():m_view_plane_width(0), m_view_plane_height(0)
    {
        float4x4::MakeIdentity(m_view_matrix);
        float4x4::MakeIdentity(m_projection_matrix);
        float4x4::MakeIdentity(m_view_projection_matrix);
    }

    void Camera::LookAt(const float3& eye, const float3& center, const float3& up)
    {
        m_eye_position = eye;
        m_view_matrix = float4x4::MakeLookAt(m_eye_position, center, up);

        m_view_projection_matrix = m_projection_matrix * m_view_matrix;

        mRightVector = float3(m_view_matrix.m[0], m_view_matrix.m[4], m_view_matrix.m[8]);
        mLookVector  = float3(m_view_matrix.m[1], m_view_matrix.m[5], m_view_matrix.m[9]);
        mUpVector    = float3(m_view_matrix.m[2], m_view_matrix.m[6], m_view_matrix.m[10]);

        //get spherical from view vector
        mSpherical = Math::Spherical(mLookVector);
    }

    void Camera::LookAtSpherical( const float3& up)
    {
        float3 center = m_eye_position + Math::Cartesian(mSpherical);

        m_view_matrix = float4x4::MakeLookAt(m_eye_position, center, up);

        m_view_projection_matrix = m_projection_matrix * m_view_matrix;

        mRightVector = float3(m_view_matrix.m[0], m_view_matrix.m[4], m_view_matrix.m[8]);
        mLookVector = float3(m_view_matrix.m[1], m_view_matrix.m[5], m_view_matrix.m[9]);
        mUpVector = float3(m_view_matrix.m[2], m_view_matrix.m[6], m_view_matrix.m[10]);
    }

    void Camera::Pan(float dx, float dz)
    {
        float3 newEye = m_eye_position + mRightVector * dx + mUpVector * dz;
        LookAt(newEye, newEye + mLookVector, float3(0, 0, 1));
    }

    void Camera::Zoom(float dy)
    {
        float3 newEye = m_eye_position + mLookVector * dy;
        LookAt(newEye, newEye + mLookVector, float3(0, 0, 1));
    }

    float Camera::DistanceTo(const float3& world_point) const
    {
        return float3::length(world_point - m_eye_position);
    }
    float3 Camera::WorldToViewPoint(const float3& world_point) const
    {
        return m_view_matrix.TransformPoint(world_point);
        //float3 view_point = float3(m_view_matrix * float4(world_point, 1.0f));
        //return view_point;
    }

    float3 Camera::WorldToViewVector(const float3& world_vector) const
    {
        return m_view_matrix.Transform(world_vector);
        //float3 view_point = float3(m_view_matrix * float4(world_vector, 0.0f));
        //return view_point;
    }

	void Camera::GetInverseViewMatrix(float4x4& transform) const
	{
		const float3& x = mRightVector;
		const float3& y = mLookVector;
		const float3& z = mUpVector;

		transform.m[0] = x.x; transform.m[4] = y.x; transform.m[8] = z.x;  transform.m[12] = m_eye_position.x;
		transform.m[1] = x.y; transform.m[5] = y.y; transform.m[9] = z.y;  transform.m[13] = m_eye_position.y;
		transform.m[2] = x.z; transform.m[6] = y.z; transform.m[10] = z.z; transform.m[14] = m_eye_position.z;
		transform.m[3] = 0;   transform.m[7] = 0;   transform.m[11] = 0;   transform.m[15] = 1;
	}

    void Camera::SetPerspective(float fov_degrees, float aspect, float near_clip, float far_clip)
    {
        m_fov_degrees = fov_degrees;
        m_aspect = aspect;
        m_near_clip = near_clip;
        m_far_clip = far_clip;

        m_projection_matrix = float4x4::MakePerspective(
            m_fov_degrees * (Pi/180),
            m_aspect,
            m_near_clip,
            m_far_clip);

        m_view_plane_width  = 2 * (1.0f / m_projection_matrix.m[0]);
        m_view_plane_height = 2 * (1.0f / m_projection_matrix.m[9]);//remember Y and Z flipped

        m_view_projection_matrix = m_projection_matrix * m_view_matrix;
    }

    void Camera::Rotate(float dx, float dz)
    {
        mSpherical.x += dx;
        mSpherical.y += dz;
        //clamp Phi angle (spherical.x)
        if (mSpherical.x > TwoPi)
            mSpherical.x = 0;

        //clamp Theta angle (spherical.y)
        if (mSpherical.y < 0)
            mSpherical.y = 0.00005f;
        else if (mSpherical.y > Pi)
            mSpherical.y = Pi - 0.00005f;

        LookAtSpherical(float3(0, 0, 1));
    }

}
