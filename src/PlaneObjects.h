#pragma once
#include "SceneObjects.h"

class Plane : public SceneObject
{
public:
	enum class Axis { XY, XZ, YZ };

	Plane() : width(0), height(0), normal(0, 0, 1), axis(Axis::XY), epsilon(.0001), reflective(false), reflectance(0.0) {} //default constructor for Plane
	Plane(glm::vec3 upperLeftCorner, float width, float height, Axis planeAxis, ofColor diffuseColor = ofColor::lightGray, ofColor spectralColor = ofColor::lightGray, bool reflective = false, float reflectance = 0.0);

	virtual void draw() { ofSetColor(getDiffuseColor()); m.draw(); }

	virtual bool intersects(const Ray& ray, glm::vec3& intersectPoint, glm::vec3& intersectNormal);

	virtual bool isReflective() { return reflective; }
	virtual float getReflectance() { return reflectance; }

	void setReflective(bool reflective) { this->reflective = reflective; }
	void setReflectance(float reflectance) { if (reflectance >= 0) this->reflectance = reflectance; }

	bool onInfinitePlane(const glm::vec3& point);
	bool insideFinitePlane(const glm::vec3& point);
	
	int getNormalSign(Ray ray);

	Axis getAxis() { return axis; }
	
	glm::vec3 getUpperLeftCorner() { if (m.verts.size() == 0) return glm::vec3(); else return m.verts.at(0); }
	
	//assumes the point is actually on the plane. 
	glm::vec2 parameterizePoint(const glm::vec3& point);


protected:
	float width, height;
	float epsilon;

private:
	glm::vec3 normal;
	Axis axis;
	Mesh m;

	bool reflective;
	float reflectance;
};

class ReflectivePlane : public Plane
{
public:
	ReflectivePlane() : Plane(), reflectance(0) {}
	ReflectivePlane(glm::vec3 upperLeftCorner, float width, float height, Axis planeAxis, float reflectance,
		ofColor diffuseColor = ofColor::lightGray, ofColor spectralColor = ofColor::lightGray)
		: Plane(upperLeftCorner, width, height, planeAxis, diffuseColor, spectralColor), reflectance(reflectance) {}

	virtual bool isReflective() { return true; }
	virtual float getReflectance() { return reflectance; }
private:
	float reflectance;
};

class TexturedPlane : public Plane
{
public:
	TexturedPlane() : maxU(0), maxV(0), texture(nullptr) {} //default constructor for TexturedPlane
	TexturedPlane(glm::vec3 upperLeftCorner, float width, float heigth, Axis planeAxis, float maxU, float maxV,
		shared_ptr<ofImage> texture);

	virtual ofColor getDiffuseColor(const glm::vec3& point);

protected:
	float maxU, maxV;

private:
	shared_ptr<ofImage> texture;

};

class NormalPlane : public TexturedPlane
{
public: 
	NormalPlane() : TexturedPlane(), normalMap(nullptr) {} //default constructor for NormalPlane
	NormalPlane(glm::vec3 upperLeftCorner, float width, float heigth, Axis planeAxis, float maxU, float maxV,
		shared_ptr<ofImage> texture, shared_ptr<ofImage> normalMap);

	virtual bool intersects(const Ray& ray, glm::vec3& intersectPoint, glm::vec3& intersectNormal);

	/// <summary>
	/// Gets the normal at a given point on the plane as seen from the provided ray
	/// </summary>
	glm::vec3 getNormalAt(glm::vec3 point, Ray ray);
private:
	shared_ptr<ofImage> normalMap;
};

/// <summary>
/// Represents a plane that has support for displacement mapping
/// Only works in the XY plane
/// Currently, the height and width of the displacement map must be a multiple of two. Additionally, the maxU and maxV values must be integers, unlike in other planes
/// </summary>
class DisplacementPlane : public NormalPlane
{
public:
	DisplacementPlane() : NormalPlane(), displacementDepth(0), displacementMap(nullptr), calculateNormal(true) {} //default constructor for DisplacementPlane

	DisplacementPlane(glm::vec3 upperLeftCorner, float width, float heigth, Axis planeAxis, int maxU, int maxV,
		shared_ptr<ofImage> texture, shared_ptr<ofImage> normalMap, shared_ptr<ofImage> displacementMap, float displacementDepth);

	virtual bool intersects(const Ray& ray, glm::vec3& intersectPoint, glm::vec3& intersectNormal);
	virtual void draw() { heightMesh.draw(); }

private:
	shared_ptr<ofImage> displacementMap;
	float displacementDepth;
	bool calculateNormal;

	Mesh heightMesh;
	void addDisplacementToMesh();
	void iterateOverDisplacementMap(float pixelWidth, float pixelHeight, float baseX, float baseZ, int& numVerts, glm::vec3& UL, glm::vec3& LL, glm::vec3& UR, glm::vec3& LR);

	glm::vec3 aabbMin;
	glm::vec3 aabbMax;
	void calculateBoundingBox();

	bool intersectsBoundingBox(const Ray& r);
	bool intersectsTriangle(const Ray& ray, int numTriangle, glm::vec3& intersectPoint, glm::vec3& intersectNormal);
	static bool intersectsTriangle(Ray ray, glm::vec3 p0, glm::vec3 p1, glm::vec3 p2, glm::vec3& baryCoords, glm::vec3& normal);

};