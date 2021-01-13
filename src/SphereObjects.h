#pragma once
#include "SceneObjects.h"

class Sphere : public SceneObject
{
public:
	Sphere() : radius(0), theta(0), phi(0) {} //default constructor for Sphere
	Sphere(glm::vec3 center, float radius, ofColor color, ofColor specularColor = ofColor::lightGray, float theta = 0, float phi = 0)
		: SceneObject(color, specularColor), center(center), radius(radius), theta(theta), phi(phi) {}

	virtual void draw() { ofSetColor(getDiffuseColor()); ofDrawSphere(center, radius); }

	virtual bool intersects(const Ray& ray, glm::vec3& intersectPoint, glm::vec3& intersectNormal)
	{
		bool rayIntersects = glm::intersectRaySphere(ray.origin, ray.direction, center, radius, intersectPoint, intersectNormal);
		if (glm::distance(ray.origin, intersectPoint) > ray.maxDistance)
			rayIntersects = false;

		return rayIntersects;
	}

	glm::vec2 parameterizePoint(const glm::vec3& point);

private:
	glm::vec3 center;
	float radius;

	float theta;
	float phi;
};

class TexturedSphere : public Sphere
{
public:
	TexturedSphere() : texture(nullptr) {} //default constructor for TexturedSphere
	TexturedSphere(glm::vec3 center, float radius, shared_ptr<ofImage> texture, ofColor specularColor = ofColor::black, float theta = 0, float phi = 0)
		: Sphere(center, radius, ofColor::darkGray, specularColor, theta, phi), texture(texture) {}

	virtual ofColor getDiffuseColor(const glm::vec3& point);
private:
	shared_ptr<ofImage> texture;
};

class TransparentSphere : public Sphere
{
public:
	TransparentSphere() {}
	TransparentSphere(glm::vec3 center, float radius, ofColor transparentColor) : Sphere(center, radius, transparentColor, ofColor::black) { }

	virtual bool isTransparent() { return true; }
};
