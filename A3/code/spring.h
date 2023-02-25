#ifndef SPRING_H
#define SPRING_H

#include <vector>
#include <vecmath.h>
#include <GL/glut.h>

using namespace std;

class Spring
{
public:

	int i, j;
	float len, stiff;

	Spring(int first, int second, float length, float stiffness) {
		i = first;
		j = second;
		len = length;
		stiff = stiffness;
	}

	Vector3f getForce(Vector3f p_i, Vector3f p_j) {
		Vector3f d = p_i - p_j;
		return -stiff * (d.abs() - len) * d / d.abs();
	}

// protected:
	
};

#endif
