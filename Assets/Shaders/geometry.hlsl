#ifndef GEOMETRY_HLSL
#define GEOMETRY_HLSL

#include "core.hlsl"

struct Ray
{
	float3 mOrigin;
	float3 mDirection;
};

struct AABB
{
	float3 mMin;
	float3 mMax;
};

//reflects v about n (v and n both pointing in the same direction)
static float3 Reflect(in float3 v, in float3 n)
{
	//normalize(2.0f * dot(Wo, globalHitNormal) * globalHitNormal - Wo);/
	return normalize(2.0f * dot(v, n) * n - v);
}

static bool Refract(in float3 wi, in float3 n, float eta, out float3 wt, out float cosThetaT)
{
	float cosThetaI  = dot(n, wi);
	// cos2(x) + sin2(x) = 1
	float sin2ThetaI = max(0, 1 - cosThetaI * cosThetaI);
	//Snell's law (eta = etaI / etaT)
	float sin2ThetaT = eta * eta * sin2ThetaI;

	//total internal reflection
	if (sin2ThetaT >= 1)
		return false;
	
	cosThetaT = sqrt(1 - sin2ThetaT);
	wt = normalize(eta * -wi + (eta * cosThetaI - cosThetaT) * n);
	return true;
}

static void ComputeTangentSpace(
	const float3 dp0_2, const float3 dp1_2,
	const float2 duv0_2, const float2 duv1_2,
	out float3 dpdu, out float3 dpdv)
{
	/*
	A triangle can be described as:
	p' = p + u*dpdu + v*dpdv
	dpdu: The vector which represents the change of position w.r.t change in tex coord U
	dpdv: The vector which represents the change of position w.r.t change in tex coord V

	We know pi[0,1,2] (ui,vi)[0,1,2]. Therefore:
	pi = p + ui*dpdu + vi*dpdv
	*/

	//If you use this form for p0, p1, p2, and compute p0-p2 and p1-p2, you get 
	//the matrix equation necessary in order to find dpdu and dpdv

	float determinant = (duv0_2.x * duv1_2.y) - (duv0_2.y - duv1_2.x);
	bool degenerateUV = abs(determinant) < (1e-8);
	if (!degenerateUV)
	{
		float invdet = 1.0f / determinant;
		dpdu = (duv1_2.y * dp0_2 - duv0_2.y * dp1_2) * invdet;
		dpdv = (-duv1_2.x * dp0_2 + duv0_2.x * dp1_2) * invdet;
	}
	if (degenerateUV || length(cross(dpdu, dpdv)) == 0)
	{
		//printf("determinant=%f\n", determinant);
		dpdu = dpdu = float3(0, 0, 0);
	}
}

static bool IntersectRayAABB(in Ray r, in AABB a)//, out float out_tmin)//<---- Not really needed
{
	float p[] = { r.mOrigin.x, r.mOrigin.y, r.mOrigin.z };
	float d[] = { r.mDirection.x, r.mDirection.y, r.mDirection.z };
	float aMin[] = { a.mMin.x, a.mMin.y, a.mMin.z };
	float aMax[] = { a.mMax.x, a.mMax.y, a.mMax.z };

	float tmin = -FLT_MAX;
	float tmax = FLT_MAX;
	float4 minNorm, maxNorm;

	// check vs. all three 'slabs' of the aabb
	for (int i = 0; i < 3; ++i)
	{
		if (abs(d[i]) < Epsilon)
		{   // ray is parallel to slab, no hit if origin not within slab
			if (p[i] < aMin[i] || p[i] > aMax[i])
			{
				return false;
			}
		}
		else
		{
			// compute intersection t values of ray with near and far plane of slab
			float ood = 1.0f / d[i];
			float t1 = (aMin[i] - p[i]) * ood;
			float t2 = (aMax[i] - p[i]) * ood;
			tmin = max(tmin, min(t1, t2));
			tmax = min(tmax, max(t1, t2));

			// exit with no collision as soon as slab intersection becomes empty
			if (tmin > tmax)
			{
				return false;
			}
		}
	}

	if (tmax < 0.f)
	{
		// entire bounding box is behind us
		return false;
	}
	else if (tmin < 0.f)
	{
		// we are inside the bounding box
		//*out_tmin = 0.f;
		return true;
	}
	else
	{
		// ray intersects all 3 slabs. return point and normal of intersection
		//out_tmin = tmin;
		return true;
	}
}

static Ray TransformRay(in Ray ray, in float4x4 xform)
{
	Ray xformed;
	xformed.mOrigin    = mul(xform, float4(ray.mOrigin, 1)).xyz;
	xformed.mDirection = mul(xform, float4(ray.mDirection, 0)).xyz;
	xformed.mDirection = normalize(xformed.mDirection);

	return xformed;
}

// Fast RayTriangle Intersection
static bool IntersectionRayTriangle(in Ray r, in float3 A, in float3 B, in float3 C, out float out_tmin, out float uOut, out float vOut)
{
	float3 E1 = B - A;
	float3 E2 = C - A;

	float3 P = cross(r.mDirection, E2);
	float det = dot(P, E1);

	// det is the same as the dot(r.direction, cross(E1, E2))
	// when det is < 0, r.direction is opposite direction then winding order
	// So if backfaceCull'ing and det < 0, then return false
	// If not backfaceCull'ing, then just check if det is close to zero
	if ((det < Epsilon) && (det > -Epsilon)) return false;

	float3 K = r.mOrigin - A;
	float3 Q = cross(K, E1);
	float u = dot(P, K) / det;

	if (u < 0.f || u > 1.f)
		return false;

	float v = dot(Q, r.mDirection) / det;
	if (v < 0.f || (u + v) > 1.f)
		return false;

	// if need to compute v and u.
	//u /= det;
	//v /= det;
	float tmin = dot(Q, E2) / det;
	if (tmin > 0)
	{
		out_tmin = tmin;

		//if (uOut && vOut)
		{
			uOut = u;
			vOut = v;
		}
		return true;
	}
	else
	{
		return false;
	}
}

static float SurfaceArea(in float3 p0, in float3 p1, in float3 p2)
{
	return 0.5 * length(cross(p1 - p0, p2 - p0));
}

static bool Polynomial(float a, float b, float c, out float x0, out float x1)
{
    float discr = b * b - 4 * a * c;
    if (discr < 0) 
        return false;
    else if (discr == 0) 
        x0 = x1 = -0.5 * b / a;
    else 
    {
        float q = (b > 0) ?
            -0.5 * (b + sqrt(discr)) :
            -0.5 * (b - sqrt(discr));
        x0 = q / a;
        x1 = c / q;
    }
    if (x0 > x1)
    {
        float temp = x0;
        x0 = x1;
        x1 = temp;
    }

    return true;
}

float RayVsSphere(in Ray ray)//, in Sphere sphere)
{ 
    float t0, t1;
    float3 L = ray.mOrigin;// - sphere.mCenter; <--- center is 0
    float a = dot(ray.mDirection, ray.mDirection); 
    float b = 2 * dot(ray.mDirection, L);
    float c = dot(L, L);// - (sphere.mRadius * sphere.mRadius); <--- radius is 1
    if (!Polynomial(a, b, c, t0, t1)) return FLT_MAX; 

    if (t0 < 0) 
    { 
        t0 = t1;
        if (t0 < 0) 
            return FLT_MAX;
    } 

    return t0; 
}

#endif //GEOMETRY_HLSL