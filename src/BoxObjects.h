#pragma once
#include "SceneObjects.h"
#include "PlaneObjects.h"

class Box : public SceneObject
{
public:
	Box() {} //default constructor for Box

	/**
	* @param corner0 the far upper left-hand corner
	* @param corner1 the near lower right-hand corner
	*/
	Box(glm::vec3 corner0, glm::vec3 corner1, ofColor color);

	virtual void draw();

	virtual bool intersects(const Ray& ray, glm::vec3& intersectPoint, glm::vec3& intersectNormal);

private:
	Plane sides[6];
};

class TexturedBox : public Box
{
public:
	TexturedBox() : maxU(0), maxV(0) {} //default constructor for Box

	//maxU and maxV are for the top face; the other faces are scaled off of it
	TexturedBox(glm::vec3 corner0, glm::vec3 corner1, float maxU, float maxV, shared_ptr<ofImage> texture);

	virtual ofColor getDiffuseColor(const glm::vec3& point);
private:
	TexturedPlane sides[6];

	shared_ptr<ofImage> texture;
	float maxU, maxV;
};

