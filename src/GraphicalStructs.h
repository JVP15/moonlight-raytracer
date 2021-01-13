#pragma once
#include "ofMain.h"
#include <glm/gtx/intersect.hpp>
#include <memory>

/*
 * @author Jordan Conragan
 */

//structs
struct Tri 
{
	Tri(int v1, int v2, int v3) : v1(v1), v2(v2), v3(v3) {}
	bool contains(int v) { return v == v1 || v == v2 || v == v3; }

	int v1;
	int v2;
	int v3;
};

struct Mesh 
{
	vector<glm::vec3> verts;
	vector<Tri> triangles;

	void draw();
	friend istream& operator >>(std::istream& ins, Mesh& m);
	friend ostream& operator <<(std::ostream& os, Mesh& m);
};

//Mainly a wrapper for the origin and direction vectors, but also contains info needed to make sure that shadows are being calculated correctly
struct Ray
{
	Ray(glm::vec3 origin, glm::vec3 direction, float maxDistance = std::numeric_limits<float>::infinity()) : origin(origin), direction(direction), maxDistance(maxDistance)
	{
		/* used by box intersection (no longer implemented)
		inv_direction = glm::vec3(1 / origin[0], 1 / origin[1], 1 / origin[2]);
		sign[0] = inv_direction[0] < 0;
		sign[1] = inv_direction[1] < 0;
		sign[2] = inv_direction[2] < 0;
		*/
	}

	glm::vec3 origin;
	glm::vec3 direction;
	float maxDistance;
};

//classes
class Light 
{
public:
	Light(glm::vec3 origin, float luminosity) : origin(origin), luminosity(luminosity) {}
	virtual void draw() { ofSetColor(ofColor::white); ofDrawSphere(origin, LIGHT_RADIUS); }
	virtual glm::vec3 lightAt(glm::vec3 point);
	Ray getRayToLight(glm::vec3 point) 
	{ 
		float distance = glm::distance(point, origin);
		return Ray(point, (origin - point) / distance, distance); 
	}

	glm::vec3 getOrigin() { return origin; }

private:
	const float LIGHT_RADIUS = 1;

	glm::vec3 origin;
	float luminosity;
};


class Spotlight : public Light
{
public:
	Spotlight(glm::vec3 origin, float luminosity, glm::vec3 direction, float coneAngle);

	virtual void draw();
	virtual glm::vec3 lightAt(glm::vec3 point);

private:
	const float LIGHT_LENGTH = 1;
	const float LINE_WDITH = 2;

	glm::vec3 direction;
	float coneAngle;
};




