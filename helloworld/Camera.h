#pragma once
#include <vec3.h>
#include <mat4.h>

class xCamera
{
public:
	xCamera():MouseSensitivity(0.005), WalkSpeed(5.5), view( 0.425733656 ,0.279169768 ,-0.860706151 ), pos(38.3913116 , -97.0132446 ,111.233620 ), fov(60), near_dist(0.2), far_dist(20000.0)
	{

	}
	mat4f GetLookAtMat(vec3f &up = vec3f(0.0, 0.0, 1.0))
	{
		return mat4f::lookAt(pos, pos+view, up);
	}
	mat4f GetPerspectiveMat(float ratio)
	{
		return mat4f::perspectiveProjection(fov, ratio, near_dist, far_dist);
	}
	void RotateView(float angle, vec3f axis)
	{
		vec3f tempview = view.rotateVec(angle, axis.x, axis.y, axis.z);
		view = tempview.normalize();
	}
	void GoFront ()
	{
		pos += (view * WalkSpeed);
	}
	void GoBack()
	{
		pos -= (view * WalkSpeed);
	}
	void GoLeft()
	{
		pos.x -= (view.y * WalkSpeed);
		pos.y += (view.x * WalkSpeed);
	}
	void GoRight()
	{
		pos.x += (view.y * WalkSpeed);
		pos.y -= (view.x * WalkSpeed);
	}
	vec3f getPos()
	{
		return pos;
	}
	bool sphereInFrustum(const vec3f &center, double radius)
	{
		for (int p = 0; p < 6; p++)
			if (_frustumPlane[p][0] * center.x + _frustumPlane[p][1] * center.y +
				_frustumPlane[p][2] * center.z + _frustumPlane[p][3] <= -radius)
				return false;
		return true;
	}
	
protected:
public:
	float MouseSensitivity;
	float WalkSpeed;
	vec3f view;
	vec3f pos;
	float fov;
	float near_dist;
	float far_dist;
	mat4f _mvp;
	vec4f _frustumPlane[6]; //left right up bottom near far
	void updateFrustum()	
	{
		mat4f invMvp = _mvp.inverse();
		vec4f near00 = invMvp * vec4f(-1, -1, -1, 1.0); near00 /= near00.w;
		vec4f near01 = invMvp * vec4f(-1, 1, -1, 1.0); near01 /= near01.w;
		vec4f near10 = invMvp * vec4f(1, -1, -1, 1.0); near10 /= near10.w;
		vec4f near11 = invMvp * vec4f(1, 1, -1, 1.0); near11 /= near11.w;
		vec4f far00 = invMvp * vec4f(-1, -1, 1, 1.0); far00 /= far00.w;
		vec4f far11 = invMvp * vec4f(1, 1, 1, 1.0); far11 /= far11.w;
		vec3f near0100 = (near01.xyz() - near00.xyz());
		vec3f near1101 = (near11.xyz() - near01.xyz());
		vec3f near1011 = (near10.xyz() - near11.xyz());
		vec3f near0010 = (near00.xyz() - near10.xyz());
		vec3f far00near00 = (far00.xyz() - near00.xyz());
		vec3f far11near11 = (far11.xyz() - near11.xyz());

		_frustumPlane[0] = vec4f((far00near00.crossProduct(near0100)).normalize(), 0.0);
		_frustumPlane[0].w = -_frustumPlane[0].dotproduct(near00);
		_frustumPlane[1] = vec4f((far11near11.crossProduct(near1011)).normalize(), 0.0);
		_frustumPlane[1].w = -_frustumPlane[1].dotproduct(near11);
		_frustumPlane[2] = vec4f((far11near11.crossProduct(near1101)).normalize(), 0.0);
		_frustumPlane[2].w = -_frustumPlane[2].dotproduct(near11);
		_frustumPlane[3] = vec4f((far00near00.crossProduct(near0010)).normalize(), 0.0);
		_frustumPlane[3].w = -_frustumPlane[3].dotproduct(near00);
		_frustumPlane[5] = vec4f(view, 0.0);
		_frustumPlane[5].w = -view.dotproduct(near00.xyz());
		_frustumPlane[6] = vec4f(-view, 0.0);
		_frustumPlane[6].w = view.dotproduct(far00.xyz());
	}
};