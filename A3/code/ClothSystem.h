#ifndef CLOTHSYSTEM_H
#define CLOTHSYSTEM_H

#include <vecmath.h>
#include <vector>
#include <GL/glut.h>

#include "particleSystem.h"

class ClothSystem: public ParticleSystem
{
///ADD MORE FUNCTION AND FIELDS HERE
public:
	int numRows, numCols;
	float scale;

	ClothSystem(int rows, int cols);

	int indexOf(int i, int j);
	vector<Vector3f> evalF(vector<Vector3f> state);
	void drawRect(int i, int j, Vector3f *normals);
	void draw();

private:

};


#endif
