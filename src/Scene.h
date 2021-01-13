#pragma once

#include "GraphicalStructs.h"
#include "SceneObjects.h"

class Scene
{
public:
	template<typename T>
	void addLight(T& light)
	{
		auto light_ptr = make_shared<T>(light);
		lights.push_back(light_ptr);
	}

	template<typename T>
	void addSceneObject(T& sceneObject)
	{
		auto obj_ptr = make_shared<T>(sceneObject);
		surfaces.push_back(obj_ptr);
	}

	void draw();
	ofColor intersectRayScene(const Ray& ray, bool reflection = false);

private:
	const ofColor DEFAULT_COLOR = ofColor::black;
	const float AMBIENT_SHADING_INTENSITY = .18;
	const float SPECTRAL_POWER = 1000;
	const float SHADOW_NORMAL_MULTIPLIER = .01;

	vector<shared_ptr<Light>> lights;
	vector<shared_ptr<SceneObject>> surfaces;

	ofColor calculateShading(const Ray& ray, SceneObject& object, glm::vec3& intersectPoint, glm::vec3& intersectNormal);
};