#include "BoxObjects.h"

//--------------------------------------------------------------

Box::Box(glm::vec3 corner0, glm::vec3 corner1, ofColor color) : SceneObject(color)
{
	float width = corner1[0] - corner0[0];
	float height = corner0[1] - corner1[1];
	float depth = corner1[2] - corner0[2];

	sides[0] = Plane(corner0, width, height, Plane::Axis::XY, color);
	sides[1] = Plane(corner0 + glm::vec3(0, 0, depth), width, height, Plane::Axis::XY, color);

	sides[2] = Plane(corner0, depth, height, Plane::Axis::YZ, color);
	sides[3] = Plane(corner0 + glm::vec3(width, 0, 0), depth, height, Plane::Axis::YZ, color);

	sides[4] = Plane(corner0, width, depth, Plane::Axis::XZ, color);
	sides[5] = Plane(corner0 - glm::vec3(0, height, 0), width, depth, Plane::Axis::XZ, color);
}

void Box::draw()
{
	for (Plane p : sides)
		p.draw();
}

bool Box::intersects(const Ray& ray, glm::vec3& intersectPoint, glm::vec3& intersectNormal)
{
	float dist2 = std::numeric_limits<float>::infinity();

	bool intersectsBox = false;

	for (Plane p : sides)
	{
		glm::vec3 tempPoint, tempNormal;

		bool intersectsPlane = p.intersects(ray, tempPoint, tempNormal);

		if (intersectsPlane)
		{
			intersectsBox = true;
			float curDist2 = glm::distance2(tempPoint, ray.origin);

			if (curDist2 < dist2)
			{
				intersectPoint = tempPoint;
				intersectNormal = tempNormal;
				dist2 = curDist2;
			}
		}
	}

	return intersectsBox;
}


//--------------------------------------------------------------

TexturedBox::TexturedBox(glm::vec3 corner0, glm::vec3 corner1, float maxU, float maxV, shared_ptr<ofImage> texture)
	: Box(corner0, corner1, ofColor::darkGray), texture(texture), maxU(maxU), maxV(maxV)
{
	float width = corner1[0] - corner0[0];
	float height = corner0[1] - corner1[1];
	float depth = corner1[2] - corner0[2];

	float maxUXZ = maxU;
	float maxVXZ = maxV;

	float maxUXY = maxU;
	float maxVXY = height / depth * maxV;

	float maxUYZ = maxV;
	float maxVYZ = height / depth * maxV;

	sides[0] = TexturedPlane(corner0, width, height, Plane::Axis::XY, maxUXY, maxVXY, texture);
	sides[1] = TexturedPlane(corner0 + glm::vec3(0, 0, depth), width, height, Plane::Axis::XY, maxUXY, maxVXY, texture);

	sides[2] = TexturedPlane(corner0, depth, height, Plane::Axis::YZ, maxUYZ, maxVYZ, texture);
	sides[3] = TexturedPlane(corner0 + glm::vec3(width, 0, 0), depth, height, Plane::Axis::YZ, maxUYZ, maxVYZ, texture);

	sides[4] = TexturedPlane(corner0, width, depth, Plane::Axis::XZ, maxUXZ, maxVXZ, texture);
	sides[5] = TexturedPlane(corner0 - glm::vec3(0, height, 0), width, depth, Plane::Axis::XZ, maxUXZ, maxVXZ, texture);
}

ofColor TexturedBox::getDiffuseColor(const glm::vec3& point)
{
	for (int i = 0; i < 6; i++)
	{
		TexturedPlane face = sides[i];

		if (face.onInfinitePlane(point))
		{
			return face.getDiffuseColor(point);
		}
	}
	return SceneObject::getDiffuseColor();
}
