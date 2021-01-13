#include "ofApp.h"
#include <fstream>
#include <iostream>
#include <limits>
#include <chrono>
#include "PlaneObjects.h"
#include "SphereObjects.h"
#include <glm/gtx/intersect.hpp>

/**
 * @author Jordan Conragan
 */

//--------------------------------------------------------------
void ofApp::loadMesh(char meshId)
{
	whatToRender = RenderObjectType::MESH;
	selectedVert = -1;

	switch (meshId)
	{
		case 'o' : m = loadOctahedron(); break;
	}
}

Mesh ofApp::loadOctahedron()
{
	cout << "Loading octahedron" << endl;
	Mesh octahedron;

	octahedron.verts.push_back(glm::vec3(5, 5, 0));
	octahedron.verts.push_back(glm::vec3(5, -5, 0));
	octahedron.verts.push_back(glm::vec3(-5, 5, 0));
	octahedron.verts.push_back(glm::vec3(-5, -5, 0));
	octahedron.verts.push_back(glm::vec3(0, 0, 5 * sqrt(2)));
	octahedron.verts.push_back(glm::vec3(0, 0, -5 * sqrt(2)));

	octahedron.triangles.push_back(Tri(4, 1, 0));
	octahedron.triangles.push_back(Tri(4, 3, 1));
	octahedron.triangles.push_back(Tri(4, 3, 2));
	octahedron.triangles.push_back(Tri(4, 1, 0));
	octahedron.triangles.push_back(Tri(4, 2, 0));

	octahedron.triangles.push_back(Tri(5, 1, 0));
	octahedron.triangles.push_back(Tri(5, 3, 1));
	octahedron.triangles.push_back(Tri(5, 3, 2));
	octahedron.triangles.push_back(Tri(5, 1, 0));
	octahedron.triangles.push_back(Tri(5, 2, 0));

	cout << octahedron;
	return octahedron;
}

void ofApp::loadScene()
{
	whatToRender = RenderObjectType::SCENE;

	shared_ptr<ofImage> moonTex = make_shared<ofImage>("moon_texture.jpg");
	shared_ptr<ofImage> waterTex = make_shared<ofImage>("Water_001_COLOR.jpg");
	shared_ptr<ofImage> waterNormal = make_shared<ofImage>("Water_001_NORM.jpg");
	shared_ptr<ofImage> starTex = make_shared<ofImage>("star.png");

	NormalPlane water(glm::vec3(-50, 0, -50), 100, 100, Plane::Axis::XZ, 20, 20, waterTex, waterNormal);
	TexturedPlane stars(glm::vec3(-192, 180, -170), 384, 216, Plane::Axis::XY, 1, 1, starTex);
	water.setReflective(true);
	water.setReflectance(.5);
	TexturedSphere moon(glm::vec3(0, 15, -100), 7, moonTex, ofColor::black, ofDegToRad(90));

	TransparentSphere halo(glm::vec3(0, 13.5, -90), 12, ofColor(255, 255, 255, 100));

	Spotlight light(glm::vec3(0, 15, -55), 550, glm::vec3(0, 0, -1), ofDegToRad(45));

	scene.addSceneObject(water);
	scene.addSceneObject(stars);
	scene.addSceneObject(moon);
	scene.addSceneObject(halo);

	scene.addLight(light);
}

//--------------------------------------------------------------

ofImage ofApp::renderScene()
{
	auto t1 = std::chrono::high_resolution_clock::now();

	int width = ofGetWidth();
	int height = ofGetHeight();

	ofImage img;
	img.allocate(width, height, OF_IMAGE_COLOR);

	if (whatToRender == RenderObjectType::SCENE) //only ray trace the scene if it is currently being displayed
	{
		for (int x = 0; x < width; x++)
		{
			for (int y = 0; y < height; y++)
			{
				glm::vec3 screen3DPt = sceneCam.screenToWorld(glm::vec3(x, y, 0));
				glm::vec3 rayOrigin = sceneCam.getPosition();
				glm::vec3 rayDir = glm::normalize(screen3DPt - rayOrigin);

				Ray ray(rayOrigin, rayDir);

				ofColor colorAtPixel = scene.intersectRayScene(ray);

				img.setColor(x, y, colorAtPixel);
			}
			//TODO: this is buggy, fix it
			cout << left << setw(5) <<  x * 100.f / width << "% Complete\r";
		}
		cout << endl;
	}

	auto t2 = std::chrono::high_resolution_clock::now();
	auto duration = std::chrono::duration_cast<std::chrono::milliseconds>(t2 - t1).count();

	cout << "Ray tracing took " << duration << " milliseconds" << endl;

	return img;
}

//--------------------------------------------------------------
void ofApp::setup()
{
	selectedVert = -1;

	easyCam.setDistance(30.0);
	easyCam.lookAt(glm::vec3(0, 0, 0));
	easyCam.setNearClip(.1);

	sceneCam.setNearClip(.1);
	sceneCam.setPosition(glm::vec3(0, 2, 15));
	sceneCam.lookAt(glm::vec3(0, 0, 0));

	cam = &easyCam;

	ofSetBackgroundColor(ofColor::black);

	whatToRender = RenderObjectType::MESH; //start off by rendering a mesh

	loadMesh('o'); //load the octohedron by default

}

//--------------------------------------------------------------
void ofApp::update(){

}

//--------------------------------------------------------------
void ofApp::draw()
{
	cam->begin();

	if (whatToRender == RenderObjectType::MESH)
	{
		ofSetColor(ofColor::white);
		m.draw();

		if (selectedVert != -1)
		{
			ofSetColor(ofColor::yellow);
			ofDrawSphere(m.verts[selectedVert], VERTEX_CLICKABLE_RADIUS);

			for (Tri t : m.triangles)
			{
				if (t.contains(selectedVert))
					ofDrawTriangle(m.verts[t.v1], m.verts[t.v2], m.verts[t.v3]);
			}
		}
	}
	else if(whatToRender == RenderObjectType::SCENE)
	{
		scene.draw();
	}
	
	cam->end();
}

//--------------------------------------------------------------
void ofApp::keyPressed(int key)
{
	if (key == 'c')
	{
		if (cam == &easyCam)
			cam = &sceneCam;
		else
			cam = &easyCam;
	}
	else if (key == 's')
	{
		loadScene();
	}
	else if (key == 'r')
	{
		string filename = "renderedScene.png";
		cout << "Rendering scene using ray tracing..." << endl;
		ofImage img = renderScene();
		img.save(filename);
		cout << "Rendering complete. Image saved to " << filename << endl;
	}
	else
		loadMesh(key);
}

//--------------------------------------------------------------
void ofApp::keyReleased(int key)
{

}

//--------------------------------------------------------------
void ofApp::mouseMoved(int x, int y ){

}

//--------------------------------------------------------------
void ofApp::mouseDragged(int x, int y, int button){

}

//--------------------------------------------------------------
void ofApp::mousePressed(int x, int y, int button)
{
	if (whatToRender == RenderObjectType::MESH)
	{
		//implements vertex selection and adjacent triangle highlighting
		glm::vec3 screen3DPt = cam->screenToWorld(glm::vec3(x, y, 0));
		glm::vec3 rayOrigin = cam->getPosition();
		glm::vec3 rayDir = glm::normalize(screen3DPt - rayOrigin);

		int closestVert = -1;
		float minDistance = std::numeric_limits<float>::infinity();

		for (int i = 0; i < m.verts.size(); i++)
		{
			glm::vec3 intersectPoint;
			glm::vec3 intersectNormal;
			bool bIntersect = glm::intersectRaySphere(rayOrigin, rayDir, m.verts[i], VERTEX_CLICKABLE_RADIUS,
				intersectPoint, intersectNormal);

			if (bIntersect)
			{
				float currentDistance = glm::distance2(intersectPoint, rayOrigin);

				if (currentDistance < minDistance)
				{
					minDistance = currentDistance;
					closestVert = i;
				}
			}
		}

		if (closestVert != -1)
		{
			if (closestVert == selectedVert)
				selectedVert = -1;
			else
				selectedVert = closestVert;
		}
	}
}

//--------------------------------------------------------------
void ofApp::mouseReleased(int x, int y, int button){

}

//--------------------------------------------------------------
void ofApp::mouseEntered(int x, int y){

}

//--------------------------------------------------------------
void ofApp::mouseExited(int x, int y){

}

//--------------------------------------------------------------
void ofApp::windowResized(int w, int h){

}

//--------------------------------------------------------------
void ofApp::gotMessage(ofMessage msg){

}

//--------------------------------------------------------------
void ofApp::dragEvent(ofDragInfo dragInfo){ 

}
