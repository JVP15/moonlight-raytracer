#pragma once

#include "ofMain.h"
#include "Scene.h"
#include "GraphicalStructs.h"

/**
 * @author Jordan Conragan
 */

class ofApp : public ofBaseApp {

	public:
		const float VERTEX_CLICKABLE_RADIUS = .1;

		/**
		* Available meshes are:
		* - o for an octohedron mesh
		*/
		void loadMesh(char meshId);
		Mesh loadOctahedron();
		void loadScene();

		ofImage renderScene();

		void setup();
		void update();
		void draw();

		void keyPressed(int key);
		void keyReleased(int key);
		void mouseMoved(int x, int y );
		void mouseDragged(int x, int y, int button);
		void mousePressed(int x, int y, int button);
		void mouseReleased(int x, int y, int button);
		void mouseEntered(int x, int y);
		void mouseExited(int x, int y);
		void windowResized(int w, int h);
		void dragEvent(ofDragInfo dragInfo);
		void gotMessage(ofMessage msg);

	private:
		ofEasyCam easyCam;
		ofCamera sceneCam;

		ofCamera* cam;

		enum class RenderObjectType {MESH, SCENE};

		RenderObjectType whatToRender;

		Scene scene;
		Mesh m;

		int selectedVert;
};
