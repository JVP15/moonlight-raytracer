#pragma once

#include "GraphicalStructs.h"


class SceneObject
{
public:
	SceneObject() : diffuseColor(ofColor::white), spectralColor(ofColor::lightGray) {}

	SceneObject(ofColor diffuseColor, ofColor spectralColor = ofColor::lightGray) 
		: diffuseColor(diffuseColor), spectralColor(spectralColor) {}

	virtual void draw() = 0;
	virtual bool intersects(const Ray& ray, glm::vec3& intersectPoint, glm::vec3& intersectNormal) = 0;

	virtual ofColor getDiffuseColor() { return diffuseColor; }
	virtual ofColor getDiffuseColor(const glm::vec3& point) { return diffuseColor; }

	virtual ofColor getSpectralColor() { return spectralColor; }
	virtual ofColor getSpectralColor(const glm::vec3& point) { return spectralColor; }

	virtual bool isReflective() { return false; }
	virtual float getReflectance() { return 0.0; }

	virtual bool isTransparent() { return false; }

private:
	ofColor diffuseColor;
	ofColor spectralColor;
};

