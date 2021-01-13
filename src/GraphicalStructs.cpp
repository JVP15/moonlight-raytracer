#include "GraphicalStructs.h"
#include <fstream>
#include <iostream>
#include <cstdlib>

/**
 * @author Jordan Conragan
 */

//--------------------------------------------------------------

void Mesh::draw()
{
	for (Tri t : triangles)
	{
		ofSetLineWidth(1);
		ofNoFill();
		ofDrawTriangle(verts[t.v1], verts[t.v2], verts[t.v3]);
	}
}

istream& operator>>(istream& ins, Mesh& m)
{
	char ch, test_ch;

	ins >> ch >> test_ch;

	while (ch != 'v') //skip any comments/whitespace until the vertex list
	{
		if (ch == '#') //skip the line if it is a comment
		{
			ins.ignore(numeric_limits<streamsize>::max(), '\n');
		}

		ins >> ch; //keep consuming characters until the vertex list

	}

	while (ch == 'v' && ins.get() == ' ') //handles adding vertices
	{
		float x, y, z;

		ins >> x >> y >> z;
		ins >> ch;

		m.verts.push_back(glm::vec3(x, y, z));
	}

	while (ch != 'f') //skips any comments/whitespace until the face list
	{
		if (ch == '#') //skip the line if it is a comment
		{
			ins.ignore(numeric_limits<streamsize>::max(), '\n');
		}

		ins >> ch; //ignores everything else until the face list
	}

	while (ch == 'f' && !ins.eof()) //handles adding faces
	{
		string token; //this is used to read in a tokenized part of the face, which may look like v1/vt1/vn1, but we only want v1
		int v1, v2, v3;

		ins >> token;
		v1 = stoi(token) - 1; //stoi extracts the first integer from a string, but stops when it reaches the first non-digit character so we only get v1

		ins >> token;
		v2 = stoi(token) - 1; //vertex numbers start from 1, but vector idices start from 0, hence the -1

		ins >> token;
		v3 = stoi(token) - 1;

		ins >> ch; //consumes the next f character

		m.triangles.push_back(Tri(v1, v2, v3));
	}

	return ins;
}

ostream& operator<<(ostream& os, Mesh& m)
{
	os << "This mesh has " << m.verts.size() << " vertices.\n";
	os << "This mesh has " << m.triangles.size() << " faces.\n";
	os << "This mesh is " << (sizeof(glm::vec3) * m.verts.size() + sizeof(Tri) * m.triangles.size()) / 1024.0 << " kilobytes." << endl; //can't use sizeof for Mesh because it doesn't work well with vectors

	return os;
}


//--------------------------------------------------------------

glm::vec3 Light::lightAt(glm::vec3 point)
{
	glm::vec3 lightVec = point - origin;
	float distance = glm::length(lightVec);

	lightVec = -1 * luminosity / (distance * distance) * (lightVec / distance); //divided lightVec by it's distance to normalize it, multiplied it by luminosity / distance^2 to set the magnitude, and -1 to reverse vector direction

	return lightVec;
}

//--------------------------------------------------------------

Spotlight::Spotlight(glm::vec3 origin, float luminosity, glm::vec3 direction, float coneAngle) : Light(origin, luminosity), direction(direction), coneAngle(coneAngle)
{

}

void Spotlight::draw()
{
	//ofPrimitiveCone had weird deformations when it was rotated, so this is my custom "cone" design; it's not perfect, but it's pretty good.
	float lightRadius = sin(coneAngle / 2);
	glm::vec3 origin = getOrigin();
	glm::vec3 sphereOrigin = origin + (LIGHT_LENGTH + lightRadius) * direction;

	ofSetColor(ofColor::lightGray);
	ofDrawSphere(sphereOrigin, lightRadius);

	ofSetLineWidth(LINE_WDITH);

	//I'm sure that there's a better way of doing this, but I've spent the last 2.5 hours trying to figure out the proper trig and I just gave up and slapped this together
	glm::vec3 dx = glm::normalize(glm::vec3(lightRadius * direction[0], 0, -lightRadius * direction[2])) * lightRadius;
	glm::vec3 dy(0, lightRadius, 0);

	ofDrawLine(origin, sphereOrigin + dx);
	ofDrawLine(origin, sphereOrigin - dx);
	ofDrawLine(origin, sphereOrigin + dy);
	ofDrawLine(origin, sphereOrigin - dy);
}

glm::vec3 Spotlight::lightAt(glm::vec3 point)
{
	glm::vec3 vecToPoint = glm::normalize(point - getOrigin());

	float angle = acos(glm::dot(direction, vecToPoint));

	if (angle <= coneAngle / 2) //if the point is within the cone defined by the direction and cone angle
		return Light::lightAt(point);

	else
		return glm::vec3(0, 0, 0);

}