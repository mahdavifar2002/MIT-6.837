#include "ClothSystem.h"

//TODO: Initialize here
ClothSystem::ClothSystem(int rows, int cols)
{
	scale = 0.2;

	numRows = rows;
	numCols = cols;
	m_numParticles = numRows * numCols;

	for (int axis = 0; axis < 3; axis++) {
		swing[axis] = false;
		swing_forwad[axis] = true;
	}
	swing_length = 8;

	// characteristics of types of strings
	float structural_length = 1 * scale;
	float structural_stiffness = 450 * scale;
	float shear_length = sqrt(2) * scale;
	float shear_stiffness = 450 * scale;
	float flex_length = 2 * scale;
	float flex_stiffness = 450 * scale;

	// initializing the state based on the number of particles
	for (int i = 0; i < numRows; i++) {
		for (int j = 0; j < numCols; j++) {
		
			// for this system, we care about the position and the velocity
			Vector3f top_left = Vector3f(-numRows/2, numCols/2, 0) * scale;
			m_vVecState.push_back(Vector3f(j, 0, i) * scale + top_left);	// x
			m_vVecState.push_back(Vector3f(0, 0, 0));				// v

			// add structural springs
			if (i < numRows - 1)
				add_spring(indexOf(i, j), indexOf(i+1, j), structural_length, structural_stiffness);
			if (j < numCols - 1)
				add_spring(indexOf(i, j), indexOf(i, j+1), structural_length, structural_stiffness);
			
			// add shear springs
			if (i < numRows - 1 && j < numCols - 1)
				add_spring(indexOf(i, j), indexOf(i+1, j+1), shear_length, shear_stiffness);
			if (i > 0 && j < numCols - 1)
				add_spring(indexOf(i, j), indexOf(i-1, j+1), shear_length, shear_stiffness);
			
			// add flex springs
			if (i < numRows - 2)
				add_spring(indexOf(i, j), indexOf(i+2, j), flex_length, flex_stiffness);
			if (j < numCols - 2)
				add_spring(indexOf(i, j), indexOf(i, j+2), flex_length, flex_stiffness);
			
		}
	}

	for (int j = 0; j < numCols; j++)
		add_fixed_particle(indexOf(0, j));
	// add_fixed_particle(indexOf(0, 0));
	// add_fixed_particle(indexOf(0, numCols - 1));

	add_obstacle(Vector3f(0, -2.5, 0), 2.50f);		// cube
	add_obstacle(Vector3f(0, -1005, 0), 1000.0f);	// floor
}

int ClothSystem::indexOf(int i, int j)
{
	return i * numRows + j;
}


// TODO: implement evalF
// for a given state, evaluate f(X,t)
vector<Vector3f> ClothSystem::evalF(vector<Vector3f> state)
{
	vector<Vector3f> f;

	float mass = 1 * scale;
	float drag_coefficient = 0.5;
	float wind = 25;

	init_f(state, f);
	apply_gravity_forces(state, f);
	apply_drag_forces(state, f, drag_coefficient, mass);
	apply_spring_forces(state, f, mass);
	apply_wind_forces(state, f, wind, mass);
	apply_collision_forces(state, f, mass);
	apply_fixed_particles(state, f);

	return f;
}


// This function simplifies calling gl of a vector vectex or normal.
inline void glNormal3d(Vector3f vec) { glNormal3d(vec[0], vec[1], vec[2]); }
inline void glVertex3d(Vector3f vec) { glVertex3d(vec[0], vec[1], vec[2]); }

inline void drawTriangle(Vector3f a, Vector3f b, Vector3f c, Vector3f n_a, Vector3f n_b, Vector3f n_c)
{
	// Vector3f normal = Vector3f::cross(a-b, c-b);

	GLfloat color[4] = {0.9, 0.9, 0.9, 1.0};
	glMaterialfv(GL_FRONT_AND_BACK, GL_AMBIENT_AND_DIFFUSE, color);

	glBegin(GL_TRIANGLES);
	glNormal3d(n_a);
	glVertex3d(a);
	glNormal3d(n_c);
	glVertex3d(c);
	glNormal3d(n_b);
	glVertex3d(b);
	glEnd();

	glBegin(GL_TRIANGLES);
	glNormal3d(-n_a);
	glVertex3d(a);
	glNormal3d(-n_b);
	glVertex3d(b);
	glNormal3d(-n_c);
	glVertex3d(c);
	glEnd();
}


void ClothSystem::drawRect(int i, int j, Vector3f *normals)
{
	Vector3f a = m_vVecState[2*indexOf(i, j)];
	Vector3f b = m_vVecState[2*indexOf(i, j+1)];
	Vector3f c = m_vVecState[2*indexOf(i+1, j+1)];
	Vector3f d = m_vVecState[2*indexOf(i+1, j)];

	Vector3f n_a = normals[numRows*(i) + j];
	Vector3f n_b = normals[numRows*(i) + j+1];
	Vector3f n_c = normals[numRows*(i+1) + j+1];
	Vector3f n_d = normals[numRows*(i+1) + j];

	drawTriangle(a, b, c, n_a, n_b, n_c);
	drawTriangle(c, d, a, n_c, n_d, n_a);

	// glBegin(GL_POLYGON);
	// glVertex3d(a);
	// glVertex3d(b);
	// glVertex3d(c);
	// glVertex3d(d);
	// glEnd();
}

///TODO: render the system (ie draw the particles)
void ClothSystem::draw()
{
	// draw the cube
	Vector3f center = Vector3f(0, -2.5, 0);
	float radius = 2.5;
	GLfloat color[4] = {1, 1, 0, 1.0};
	glMaterialfv(GL_FRONT_AND_BACK, GL_AMBIENT_AND_DIFFUSE, color);
	glPushMatrix();
	glTranslatef(center[0], center[1], center[2]);
	glutSolidSphere(radius,40,40);
	glPopMatrix();

	GLfloat ctrlpoints2[5][4][3] = {
			{{-0.4, -0.4, 0.0}, {-0.2, -0.3, -0.1},	{0.0, -0.5, 0.1},	{0.4, -0.4, 0.0}},
			{{-0.4, -0.2, 0.0},	{-0.2, -0.1, -0.1},	{0.0, -0.3, 0.1},	{0.4, -0.2, 0.0}},
			{{-0.4,  0.0, 0.0},	{-0.2,  0.1, -0.1},	{0.0, -0.1, 0.1},	{0.4,  0.0, 0.0}},
			{{-0.4,  0.4, 0.0},	{-0.2,  0.5, -0.1},	{0.0,  0.3, 0.1},	{0.4,  0.4, 0.0}},
			{{-0.4,  0.5, 0.2},	{-0.2,  0.6,  0.1},	{0.0,  0.5, 0.3},	{0.4,  0.5, 0.2}}
	};

	Vector3f ctrlpoints[numRows][numCols] = {};

	for (int i = 0; i < numRows; i++)
		for (int j = 0; j < numCols; j++) {
			ctrlpoints[i][j][0] = m_vVecState[2*indexOf(i, j)].x();
			ctrlpoints[i][j][1] = m_vVecState[2*indexOf(i, j)].y();
			ctrlpoints[i][j][2] = m_vVecState[2*indexOf(i, j)].z();

		}

	// for (int i = 0; i < m_numParticles; i++) {
	// 	Vector3f pos = state[2*i];//  position of particle i
	// 	glPushMatrix();
	// 	glTranslatef(pos[0], pos[1], pos[2] );
	// 	glutSolidSphere(0.075f,10.0f,10.0f);
	// 	glPopMatrix();
	// }
	
	Vector3f normals[numRows][numCols] = {};

	for (int i = 0; i < numRows - 1; i++) {
		for (int j = 0; j < numCols - 1; j++) {
			Vector3f a = m_vVecState[2*indexOf(i, j)];
			Vector3f b = m_vVecState[2*indexOf(i, j+1)];
			Vector3f c = m_vVecState[2*indexOf(i+1, j+1)];
			Vector3f d = m_vVecState[2*indexOf(i+1, j)];

			
			Vector3f normal = Vector3f::cross(a-b, c-b); // normal of (a, b, c)
			normals[i][j] += normal;
			normals[i][j+1] += normal;
			normals[i+1][j+1] += normal;

			normal = Vector3f::cross(c-d, a-d); // normal of (c, d, a)
			normals[i+1][j+1] += normal;
			normals[i+1][j] += normal;
			normals[i][j] += normal;
		}
	}

	for (int i = 0; i < numRows; i++) {
		for (int j = 0; j < numCols; j++) {
			// draw structural springs
			// if (i < numRows - 1)
			// 	drawline(indexOf(i,j), indexOf(i+1, j));
			// if (j < numCols - 1)
			// 	drawline(indexOf(i,j), indexOf(i, j+1));
			if (i < numRows - 1 && j < numCols - 1) {
				drawRect(i, j, &normals[0][0]);
			}
		}
	}
}

