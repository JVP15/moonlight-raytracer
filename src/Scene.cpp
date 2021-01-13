#include "Scene.h"
#include "ofMain.h"
#include "stdlib.h"
#include <memory>

void Scene::draw()
{
	ofFill();
	for (auto l : lights)
		l->draw();

	ofNoFill();
	for (int i = 0; i < surfaces.size(); i++)
	{
		surfaces[i]->draw();
	}
}

ofColor Scene::intersectRayScene(const Ray& ray, bool reflection)
{
	ofColor colorAtRay = DEFAULT_COLOR;

	float closestDistance = std::numeric_limits<float>::infinity(); //represents the distance of the currently closest object
	glm::vec3 closestPoint;
	glm::vec3 closestNormal;
	int closestObjectIndex = -1;

	for (int i = 0; i < surfaces.size(); i++)
	{
		glm::vec3 intersectPoint;
		glm::vec3 intersectNormal;
		bool bIntersect = surfaces[i]->intersects(ray, intersectPoint, intersectNormal);

		//if the object is transparent, add the color (and do a bunch of opacity math) to the colorAtRay
		if (bIntersect && surfaces[i]->isTransparent())
		{
			ofColor transparentColor = calculateShading(ray, *surfaces[i], intersectPoint, intersectNormal);
			//this is built on the assumption that, if we're intersecting a transparent object and the colorAtRay is 255, then we haven't intersected any object before so we can just set the opacity to the current color
			if (colorAtRay.a == 255)
			{
				colorAtRay.a = transparentColor.a;
			}
				
			//if the transparency isn't 255, then do alpha addition (which I have to do manually because for some reason, it doesn't work so well with OF)
			else
			{
				//taken from https://en.wikipedia.org/wiki/Alpha_compositing
				float srcAlpha = colorAtRay.a / 255.0;
				float dstAlpha = transparentColor.a / 255.0;
				float outAlpha = srcAlpha + dstAlpha * (1 - srcAlpha);
				colorAtRay.a = 255.0 * outAlpha;
			}

			//OF uses the alpha value of the left term (which we've already taken care of)
			colorAtRay += transparentColor;
		}
		//otherwise, ray trace as usual
		else if (bIntersect)
		{
			float currentDistance = glm::distance2(intersectPoint, ray.origin);

			if (currentDistance < closestDistance) //if we find an intersection, test to see if it is the closest one
			{
				closestDistance = currentDistance;
				closestPoint = intersectPoint;
				closestNormal = intersectNormal;
				closestObjectIndex = i;
			}
		}
	}

	if (closestObjectIndex != -1)
	{
		//this helps prevent any transparent objects from combining their color too much with we are ray tracing
		float transparencyMultiplier = colorAtRay.a / 255.0;

		colorAtRay = transparencyMultiplier * colorAtRay + calculateShading(ray, *surfaces[closestObjectIndex], closestPoint, closestNormal);
	}

	return colorAtRay;
}

ofColor Scene::calculateShading(const Ray& ray, SceneObject& object, glm::vec3& intersectPoint, glm::vec3& intersectNormal)
{
	ofColor finalColor = object.getDiffuseColor(intersectPoint) * AMBIENT_SHADING_INTENSITY;

	//for the time being, if an object is transparent, don't provide any ambient light for it. I think it looks better this way
	if (object.isTransparent())
		finalColor = ofColor::black;

	glm::vec3 shadowRayOrigin = intersectPoint + SHADOW_NORMAL_MULTIPLIER * intersectNormal;
	
	for (auto light : lights)
	{
		Ray rayToLight = light->getRayToLight(shadowRayOrigin);
		
		bool lightBlocked = false;
		float percentLightReachedObject = 1.0;

		for (shared_ptr<SceneObject> blockingObject : surfaces)
		{
			glm::vec3 junk1, junk2;
			lightBlocked = blockingObject->intersects(rayToLight, junk1, junk2);
				
			//if there is a transparent object blocking another object, then reduce the amount of light that reaches the object, but don't block out the object entirely
			if (lightBlocked && blockingObject->isTransparent())
			{
				lightBlocked = false;
				//blockingObject.color.alpha / 255 gives us the %light that gets blocked, so subtracting it from 1 gives us the %light that makes it through
				percentLightReachedObject *= 1.0 - blockingObject->getDiffuseColor().a / 255.0;
			}
			else if (lightBlocked)
				break;
				
		}
		
		if (!lightBlocked)
		{
			glm::vec3 lightVec = percentLightReachedObject * light->lightAt(intersectPoint);

			ofColor lambertShading = object.getDiffuseColor(intersectPoint) * max(0.f, glm::dot(lightVec, intersectNormal));

			//ray.direction points from viewer to the point, but h bisects the light vector and a viewing vector that points from the point to the viewer, hence why we subtract ray.direction
			glm::vec3 h = (lightVec - ray.direction) / glm::length(lightVec - ray.direction);

			ofColor phongShading = object.getSpectralColor(intersectPoint) * glm::length(lightVec) * pow(max(0.f, glm::dot(h, intersectNormal)), SPECTRAL_POWER);

			finalColor += lambertShading + phongShading;

			//OF doesn't seem to handle opacity very well, so I have to manually set the opacity 
			finalColor.a = lambertShading.a;
		}
	}

	//if the object is reflective, then basically repeat the process all over again
	if (object.isReflective())
	{
		//vector reflection taken from https://math.stackexchange.com/a/13263
		glm::vec3 reflectionDirection = ray.direction - 2 * (glm::dot(ray.direction, intersectNormal)) * intersectNormal;

		//the point is offset by a little bit just so that we don't end up reflecting with ourself
		Ray reflectionRay(intersectPoint + reflectionDirection * SHADOW_NORMAL_MULTIPLIER, reflectionDirection);

		ofColor reflection = intersectRayScene(reflectionRay, true);

		finalColor += object.getReflectance() * reflection;
	}

	return finalColor;
}