#include "PlaneObjects.h"

Plane::Plane(glm::vec3 corner, float width, float height, Axis planeAxis, ofColor diffuseColor, ofColor spectralColor, bool reflective, float reflectance)
	: SceneObject(diffuseColor, spectralColor), width(width), height(height), axis(planeAxis), epsilon(.0001), reflective(reflective), reflectance(reflectance)
{
	glm::vec3 widthVec, heightVec;

	switch (planeAxis)
	{
	case Axis::XY: widthVec = glm::vec3(width, 0, 0); heightVec = glm::vec3(0, -height, 0); normal = glm::vec3(0, 0, 1); break;
	case Axis::XZ: widthVec = glm::vec3(width, 0, 0); heightVec = glm::vec3(0, 0, height); normal = glm::vec3(0, 1, 0);  break;
	case Axis::YZ: widthVec = glm::vec3(0, -height, 0); heightVec = glm::vec3(0, 0, width); normal = glm::vec3(1, 0, 0);  break;
	}

	m.verts.push_back(corner); //upper 'left' corner
	m.verts.push_back(corner + widthVec); //upper 'right' corner
	m.verts.push_back(corner + heightVec); //bottom 'left' corner
	m.verts.push_back(corner + widthVec + heightVec); //bottom 'right' corner

	m.triangles.push_back(Tri(0, 2, 1));
	m.triangles.push_back(Tri(1, 2, 3));
}

bool Plane::insideFinitePlane(const glm::vec3& point)
{
	bool pointIsInside = false;
	float x = m.verts[0][0];
	float y = m.verts[0][1];
	float z = m.verts[0][2];

	switch (axis)
	{
	case Axis::XY: pointIsInside = point[0] >= x && point[0] <= x + width && point[1] <= y && point[1] >= y - height; break;
	case Axis::XZ: pointIsInside = point[0] >= x && point[0] <= x + width && point[2] >= z && point[2] <= z + height; break;
	case Axis::YZ: pointIsInside = point[1] <= y && point[1] >= y - height && point[2] >= z && point[2] <= z + width;  break;
	}

	return pointIsInside;
}

bool Plane::onInfinitePlane(const glm::vec3& point)
{
	bool pointOnPlane = false;
	float x = m.verts[0][0];
	float y = m.verts[0][1];
	float z = m.verts[0][2];

	switch (axis)
	{
	case Axis::XY: pointOnPlane = fabs(z - point[2]) <= epsilon; break;
	case Axis::XZ: pointOnPlane = fabs(y - point[1]) <= epsilon; break;
	case Axis::YZ: pointOnPlane = fabs(x - point[0]) <= epsilon; break;
	}

	return pointOnPlane;
}

glm::vec2 Plane::parameterizePoint(const glm::vec3& point)
{
	float u = 0;
	float v = 0;

	glm::vec3 upperLeftHandCorner = m.verts[0];

	switch (axis)
	{
	case Axis::XY: u = (point[0] - upperLeftHandCorner[0]) / width; v = (upperLeftHandCorner[1] - point[1]) / height; break;
	case Axis::XZ: u = (point[0] - upperLeftHandCorner[0]) / width; v = (point[2] - upperLeftHandCorner[2]) / height; break;
	case Axis::YZ: u = (point[2] - upperLeftHandCorner[2]) / width; v = (upperLeftHandCorner[1] - point[1]) / height; break;
	}

	return glm::vec2(u, v);
}

bool Plane::intersects(const Ray& ray, glm::vec3& intersectPoint, glm::vec3& intersectNormal)
{
	glm::vec3 truePlaneNormal = normal * getNormalSign(ray);

	float distance;
	bool rayIntersects = glm::intersectRayPlane(ray.origin, ray.direction, m.verts[0], truePlaneNormal, distance); //calculates intersection for an infinite plane

	if (rayIntersects && distance <= ray.maxDistance)
	{
		glm::vec3 point = ray.origin + distance * ray.direction; //get the point of intersection on the infinite plane

		if (insideFinitePlane(point)) //if the intersection point on the infinite plane is also on the finite plane, then do stuff
		{
			intersectPoint = point;
			intersectNormal = truePlaneNormal;
		}
		else //if the ray intersects with the infinite plane, but not the finite plane, then rayIntersect should be false
		{
			rayIntersects = false;
		}
	}
	else //if the ray doesn't intersect, or it intersects, but it intersect is too far away, then rayIntersects should be false
	{
		rayIntersects = false;
	}

	return rayIntersects;
}

int Plane::getNormalSign(Ray ray)
{
	int normalSign = 1;

	switch (axis) //by default, normal points in the positive direction. This checks to see if the ray came from the negative direction
	{
	case Axis::XY: if (ray.origin[2] < m.verts[0][2]) normalSign = -1; break;
	case Axis::XZ: if (ray.origin[1] < m.verts[0][1]) normalSign = -1; break;
	case Axis::YZ: if (ray.origin[0] < m.verts[0][0]) normalSign = -1; break;
	}

	return normalSign;
}

//--------------------------------------------------------------


TexturedPlane::TexturedPlane(glm::vec3 upperLeftCorner, float width, float heigth, Axis planeAxis, float maxU, float maxV, shared_ptr<ofImage> texture)
	: Plane(upperLeftCorner, width, heigth, planeAxis, ofColor::darkGray, ofColor::black),
	maxU(maxU), maxV(maxV), texture(texture)
{ }

ofColor TexturedPlane::getDiffuseColor(const glm::vec3& point)
{
	if (texture != nullptr)
	{
		glm::vec2 parameterizedVec = parameterizePoint(point);

		float u = parameterizedVec[0] * maxU;
		float v = parameterizedVec[1] * maxV;

		int x = fmod(u * texture->getWidth(), texture->getWidth());
		int y = fmod(v * texture->getHeight(), texture->getHeight());

		return texture->getColor(x, y);
	}

	Plane::getDiffuseColor();
}


//--------------------------------------------------------------

NormalPlane::NormalPlane(glm::vec3 upperLeftCorner, float width, float heigth, Axis planeAxis, float maxU, float maxV, 
	shared_ptr<ofImage> texture, shared_ptr<ofImage> normalMap)
	: TexturedPlane(upperLeftCorner, width, heigth, planeAxis, maxU, maxV, texture), normalMap(normalMap)
{ }

bool NormalPlane::intersects(const Ray& ray, glm::vec3& intersectPoint, glm::vec3& intersectNormal)
{
	bool rayIntersects = TexturedPlane::intersects(ray, intersectPoint, intersectNormal);

	if (rayIntersects && normalMap != nullptr)
	{
		intersectNormal = getNormalAt(intersectPoint, ray);
	}

	return rayIntersects;
}


glm::vec3 NormalPlane::getNormalAt(glm::vec3 point, Ray ray)
{
	glm::vec2 parameterizedVec = parameterizePoint(point);

	float u = parameterizedVec[0] * maxU;
	float v = parameterizedVec[1] * maxV;

	int x = fmod(u * normalMap->getWidth(), normalMap->getWidth());
	int y = fmod(v * normalMap->getHeight(), normalMap->getHeight());

	ofColor normalColor = normalMap->getColor(x, y);

	int xColor = 255, yColor = 255, zColor = 255;
	switch (getAxis())
	{
	case Axis::XY: xColor = normalColor.r; yColor = normalColor.g; zColor = normalColor.b; break;
	case Axis::XZ: xColor = normalColor.r; yColor = normalColor.b; zColor = normalColor.g; break;
	case Axis::YZ: xColor = normalColor.b; yColor = normalColor.g; zColor = normalColor.r;  break;
	}

	//normal math taken from https://learnopengl.com/Advanced-Lighting/Normal-Mapping
	float normalX = (xColor / 255.0) * 2 - 1;
	float normalY = (yColor / 255.0) * 2 - 1;
	float normalZ = (zColor / 255.0) * 2 - 1;

	return glm::normalize(glm::vec3(normalX, normalY, normalZ)) * getNormalSign(ray);
}

//--------------------------------------------------------------


DisplacementPlane::DisplacementPlane(glm::vec3 upperLeftCorner, float width, float heigth, Axis planeAxis, int maxU, int maxV,
	shared_ptr<ofImage> texture, shared_ptr<ofImage> normalMap, shared_ptr<ofImage> displacementMap, float displacementDepth)
	: NormalPlane(upperLeftCorner, width, heigth, planeAxis, maxU, maxV, texture, normalMap), displacementMap(displacementMap), displacementDepth(displacementDepth), calculateNormal(false)
{ 
	if (normalMap == nullptr)
		calculateNormal = true;

	if (displacementMap != nullptr)
	{
		addDisplacementToMesh();
		calculateBoundingBox();
	}
	
}

bool DisplacementPlane::intersects(const Ray& ray, glm::vec3& intersectPoint, glm::vec3& intersectNormal)
{
	bool rayIntersects = false;
	bool hitsBox = intersectsBoundingBox(ray);

	if (hitsBox)
	{
		int closestTri = -1;
		float distance = std::numeric_limits<float>::infinity();
		glm::vec3 point;
		glm::vec3 normal;

		for (int i = 0; i < heightMesh.triangles.size(); i++)
		{
			glm::vec3 curPoint;
			glm::vec3 curNormal;
			bool itIntersects = intersectsTriangle(ray, i, curPoint, curNormal);
			float curDistance = glm::distance(curPoint, ray.origin);

			if (itIntersects && curDistance < distance)
			{
				point = curPoint;
				normal = curNormal;
				closestTri = i;
				distance = curDistance;
				rayIntersects = true;
			}

		}

		//if we didn't intersect with any triangles, just return false
		if (closestTri == -1)
			return false;

		intersectPoint = point;

		//if we did intersect with something, but we don't have a normal map, then we need to calculate the normal
		if (calculateNormal)
		{
			//the 'normal' calculated by that function is not exactly normalized
			normal = glm::normalize(normal);

			glm::vec3 v1 = heightMesh.verts[heightMesh.triangles[closestTri].v1];
			glm::vec3 v2 = heightMesh.verts[heightMesh.triangles[closestTri].v2];
			glm::vec3 v3 = heightMesh.verts[heightMesh.triangles[closestTri].v3];
			//now that we have the intersect point and the direction of the normal, we need to figure out it's sign, which changes depending on the origin of the ray
			//code to determine the sign of the normal comes from here: https://math.stackexchange.com/a/214194
			glm::vec3 B_prime = v2 - v1;
			glm::vec3 C_prime = v3 - v1;
			glm::vec3 X_prime = ray.origin - v1;
			glm::mat3 M = glm::mat3(B_prime, C_prime, X_prime);
			float matDeterminant = glm::determinant(M);

			//by default, the normal is already the 'positive' normal, so we only need to check if it's 'negative'
			if (matDeterminant < 0)
				normal = -1 * normal;

			intersectNormal = normal;
		}
		//if there is a normal map, just get the normal from that map
		else
		{
			intersectNormal = NormalPlane::getNormalAt(intersectPoint, ray);
		}
	}

	return rayIntersects;
}

void DisplacementPlane::addDisplacementToMesh()
{
	float pixelWidth = width / (maxU * displacementMap->getWidth());
	float pixelHeight = height / (maxV * displacementMap->getHeight());

	//this makes it so that currentLeftCorner.x and currentLeftCorner.z refer to points on the plane while currentLeftCorner.y refers to a displacement away from the plane
	glm::vec3 upperLeftCorner = getUpperLeftCorner();
	const float X = upperLeftCorner.x;
	const float Y = upperLeftCorner.y;
	const float Z = upperLeftCorner.z;

	glm::vec3 lowerLeftCorner = glm::vec3(X, Y - pixelHeight, Z);
	glm::vec3 upperRightCorner = glm::vec3(X + pixelWidth, Y, Z);
	glm::vec3 lowerRightCorner = glm::vec3(X + pixelWidth, Y - pixelHeight, Z);

	int numVerts = 0;

	for (int i = 0; i < maxU; i++)
	{
		for (int j = 0; j < maxV; j++)
		{
			iterateOverDisplacementMap(pixelWidth, pixelHeight, X, Z, numVerts, upperLeftCorner, lowerLeftCorner, upperRightCorner, lowerRightCorner);
		}
	}
}

/// <summary>
/// Iterates over the displacement map add adds the vertices and triangles to the mesh
/// </summary>
/// <param name="UL">location of the upper-left hand corner for the first pixel</param>
/// <param name="LL">location of the lower-left hand corner for the first pixel</param>
/// <param name="UR">location of the upper-right hand corner for the first pixel</param>
/// <param name="LR">location of the lower-right hand corner for the first pixel</param>
void DisplacementPlane::iterateOverDisplacementMap(float pixelWidth, float pixelHeight, float baseX, float baseZ, int& numVerts, glm::vec3& UL, glm::vec3& LL, glm::vec3& UR, glm::vec3& LR)
{
	for (int pixelY = 0; pixelY < displacementMap->getHeight(); pixelY++)
	{
		for (int pixelX = 0; pixelX < displacementMap->getWidth(); pixelX++)
		{
			//we calculate the displacement of a pixel as the average of the height map in the surrounding pixels
			float displacement = displacementMap->getColor(pixelX, pixelY).r / 255.0;
			// upper left corner
			int adjacentPixelsUL = 1;
			float dzUL = displacement;

			if (pixelX != 0 && pixelY != 0){
				dzUL += displacementMap->getColor(pixelX - 1, pixelY - 1).r / 255.0;
				adjacentPixelsUL++;
			}
			if (pixelX != 0) {
				dzUL += displacementMap->getColor(pixelX - 1, pixelY).r / 255.0;
				adjacentPixelsUL++;
			}
			if (pixelY != 0) {
				dzUL += displacementMap->getColor(pixelX, pixelY - 1).r / 255.0;
				adjacentPixelsUL++;
			}
			
			UL.z += dzUL / adjacentPixelsUL;


			// lower left corner
			int adjacentPixelsLL = 1;
			float dzLL = displacement;

			if (pixelX != 0 && pixelY != displacementMap->getHeight() - 1) {
				dzLL += displacementMap->getColor(pixelX - 1, pixelY + 1).r / 255.0;
				adjacentPixelsLL++;
			}
			if (pixelX != 0) {
				dzLL += displacementMap->getColor(pixelX - 1, pixelY).r / 255.0;
				adjacentPixelsLL++;
			}
			if (pixelY != displacementMap->getHeight() - 1) {
				dzLL += displacementMap->getColor(pixelX, pixelY + 1).r / 255.0;
				adjacentPixelsLL++;
			}

			LL.z += dzLL / adjacentPixelsLL;

			//upper right corner
			int adjacentPixelsUR = 1;
			float dzUR = displacement;

			if (pixelX != displacementMap->getWidth() - 1 && pixelY != 0) {
				dzUR += displacementMap->getColor(pixelX + 1, pixelY - 1).r / 255.0;
				adjacentPixelsUR++;
			}
			if (pixelX != displacementMap->getWidth() - 1) {
				dzUR += displacementMap->getColor(pixelX + 1, pixelY).r / 255.0;
				adjacentPixelsUR++;
			}
			if (pixelY != 0) {
				dzUR += displacementMap->getColor(pixelX, pixelY - 1).r / 255.0;
				adjacentPixelsUR++;
			}

			UR.z += dzUR / adjacentPixelsUR;

			//lower right corner
			int adjacentPixelsLR = 1;
			float dzLR = displacement;

			if (pixelX != displacementMap->getWidth() - 1 && pixelY != displacementMap->getHeight() - 1) {
				dzLR += displacementMap->getColor(pixelX + 1, pixelY + 1).r / 255.0;
				adjacentPixelsLR++;
			}
			if (pixelX != displacementMap->getWidth() - 1) {
				dzLR += displacementMap->getColor(pixelX + 1, pixelY).r / 255.0;
				adjacentPixelsLR++;
			}
			if (pixelY != displacementMap->getHeight() - 1) {
				dzLR += displacementMap->getColor(pixelX, pixelY + 1).r / 255.0;
				adjacentPixelsLR++;
			}

			LR.z += dzLR / adjacentPixelsLR;

			//now that we have set the displacement of all of the vertices, we need to add them to the mesh
			heightMesh.verts.push_back(UL);
			heightMesh.verts.push_back(LL);
			heightMesh.verts.push_back(UR);
			heightMesh.verts.push_back(LR);

			//we also have to add the triangles, then we can update the number of vertices
			heightMesh.triangles.push_back(Tri(numVerts, numVerts + 1, numVerts + 2));
			heightMesh.triangles.push_back(Tri(numVerts + 2, numVerts + 1, numVerts + 3));

			numVerts += 4;

			//now that we've added the vertices, we have to move the points and reset the Z values
			UL.x += pixelWidth;
			UL.z = baseZ;

			LL.x += pixelWidth;
			LL.z = baseZ;

			UR.x += pixelWidth;
			UR.z = baseZ;

			LR.x += pixelWidth;
			LR.z = baseZ;
		}

		//now that we've iterated over a row, we have to go down a row
		UL.x = baseX;
		UL.y -= pixelHeight;

		LL.x = baseX;
		LL.y -= pixelHeight;

		UR.x = baseX + pixelWidth;
		UR.y -= pixelHeight;

		LR.x = baseX + pixelWidth;
		LR.y -= pixelHeight;
	}
}


void DisplacementPlane::calculateBoundingBox()
{
	float minX = std::numeric_limits<float>::infinity();
	float maxX = - std::numeric_limits<float>::infinity();
	float minY = std::numeric_limits<float>::infinity();
	float maxY = - std::numeric_limits<float>::infinity();
	float minZ = std::numeric_limits<float>::infinity();
	float maxZ = - std::numeric_limits<float>::infinity();

	for (glm::vec3 vert : heightMesh.verts)
	{
		if (vert[0] < minX)
			minX = vert[0];
		else if (vert[0] > maxX)
			maxX = vert[0];

		if (vert[1] < minY)
			minY = vert[1];
		else if (vert[1] > maxY)
			maxY = vert[1];

		if (vert[2] < minZ)
			minZ = vert[2];
		else if (vert[2] > maxZ)
			maxZ = vert[2];
	}

	aabbMin = glm::vec3(minX, minY, minZ);
	aabbMax = glm::vec3(maxX, maxY, maxZ);
}

//taken from https://www.scratchapixel.com/lessons/3d-basic-rendering/minimal-ray-tracer-rendering-simple-shapes/ray-box-intersection
bool DisplacementPlane::intersectsBoundingBox(const Ray& r)
{
	float tmin = (aabbMin.x - r.origin.x) / r.direction.x;
	float tmax = (aabbMax.x - r.origin.x) / r.direction.x;

	if (tmin > tmax) swap(tmin, tmax);

	float tymin = (aabbMin.y - r.origin.y) / r.direction.y;
	float tymax = (aabbMax.y - r.origin.y) / r.direction.y;

	if (tymin > tymax) swap(tymin, tymax);

	if ((tmin > tymax) || (tymin > tmax))
		return false;

	if (tymin > tmin)
		tmin = tymin;

	if (tymax < tmax)
		tmax = tymax;

	float tzmin = (aabbMin.z - r.origin.z) / r.direction.z;
	float tzmax = (aabbMax.z - r.origin.z) / r.direction.z;

	if (tzmin > tzmax) swap(tzmin, tzmax);

	if ((tmin > tzmax) || (tzmin > tmax))
		return false;

	if (tzmin > tmin)
		tmin = tzmin;

	if (tzmax < tmax)
		tmax = tzmax;

	return true;
}

bool DisplacementPlane::intersectsTriangle(const Ray& ray, int numTriangle, glm::vec3& intersectPoint, glm::vec3& intersectNormal)
{
	glm::vec3 v1 = heightMesh.verts[heightMesh.triangles[numTriangle].v1];
	glm::vec3 v2 = heightMesh.verts[heightMesh.triangles[numTriangle].v2];
	glm::vec3 v3 = heightMesh.verts[heightMesh.triangles[numTriangle].v3];

	glm::vec3 baryCoord, normal;

	//for whatever reason, glm::intersectRayTriangle and glm::intersectLineTriangle wouldn't work, so I had to find my own implementation
	bool intersectsTriangle = DisplacementPlane::intersectsTriangle(ray, v1, v2, v3, baryCoord, normal);

	if (intersectsTriangle)
	{
		intersectPoint.x = baryCoord.x * v1.x + baryCoord.y * v2.x + baryCoord.z * v3.x;
		intersectPoint.y = baryCoord.x * v1.y + baryCoord.y * v2.y + baryCoord.z * v3.y;
		intersectPoint.z = baryCoord.x * v1.z + baryCoord.y * v2.z + baryCoord.z * v3.z;

		if (glm::distance(ray.origin, intersectPoint) > ray.maxDistance)
			intersectsTriangle = false;		
	}

	return intersectsTriangle;
}


//modified from https://github.com/Jojendersie/gpugi/blob/5d18526c864bbf09baca02bfab6bcec97b7e1210/gpugi/shader/intersectiontests.glsl#L63
bool DisplacementPlane::intersectsTriangle(Ray ray, glm::vec3 p0, glm::vec3 p1, glm::vec3 p2, glm::vec3& baryCoords, glm::vec3& normal)
{
	const glm::vec3 e0 = p1 - p0;
	const glm::vec3 e1 = p0 - p2;
	normal = cross(e1, e0);

	const glm::vec3 e2 = (1.0 / glm::dot(normal, ray.direction)) * (p0 - ray.origin);
	const glm::vec3 i = glm::cross(ray.direction, e2);

	baryCoords.y = dot(i, e1);
	baryCoords.z = dot(i, e0);
	baryCoords.x = 1.0 - (baryCoords.z + baryCoords.y);
	float hit = glm::dot(normal, e2);

	return  (hit > .000001) && glm::all(glm::greaterThanEqual(baryCoords, glm::vec3(0.0)));
}