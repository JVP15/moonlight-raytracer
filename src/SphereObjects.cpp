#define _USE_MATH_DEFINES

#include "SphereObjects.h"
#include <cmath>
//--------------------------------------------------------------

glm::vec2 Sphere::parameterizePoint(const glm::vec3& point)
{
	glm::vec3 positionRelativeToCenter = point - center;
	float x = positionRelativeToCenter[0];
	float y = positionRelativeToCenter[1];
	float z = positionRelativeToCenter[2];

	//uses Spherical coordinate projection from https://people.cs.clemson.edu/~dhouse/courses/405/notes/texture-maps.pdf

	float theta = atan2(-z, x) + this->theta;
	float phi = acos(y / radius) + this->phi;

	float u = (theta + M_PI) / (2 * M_PI);
	float v = phi / M_PI;

	return glm::vec2(u, v);
}

//--------------------------------------------------------------

ofColor TexturedSphere::getDiffuseColor(const glm::vec3& point)
{
	glm::vec2 parameterizedVec = parameterizePoint(point);

	int x = fmod(parameterizedVec[0] * texture->getWidth(), texture->getWidth());
	int y = fmod(parameterizedVec[1] * texture->getHeight(), texture->getHeight());

	return texture->getColor(x, y);
}