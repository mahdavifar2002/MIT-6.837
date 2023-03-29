#ifndef PARTICLESYSTEM_H
#define PARTICLESYSTEM_H

#include <vector>
#include <vecmath.h>
#include <GL/glut.h>
#include <ctime>
#include "spring.h"

using namespace std;

inline int get_time()
{
	return time(NULL);
}

class Sphere {
public:
	Sphere(Vector3f c, float r):center(c), radius(r){}

	Vector3f center;
	float radius;
};

class ParticleSystem
{
public:

	ParticleSystem(int numParticles=0);

	int m_numParticles;

	// for a given state, evaluate derivative f(X,t)
	virtual vector<Vector3f> evalF(vector<Vector3f> state) = 0;
	
	// getter method for the system's state
	vector<Vector3f> getState(){ return m_vVecState; };
	
	// setter method for the system's state
	void setState(const vector<Vector3f>  & newState) { m_vVecState = newState; };
	
	virtual void draw() = 0;

	int mod(int i){ return (i + m_numParticles) % m_numParticles; }

	void drawline(int i, int j)
	{
		Vector3f p_i = m_vVecState[2*i];//  position of particle i
		Vector3f p_j = m_vVecState[2*j];//  position of particle j
		glLineWidth(5.0f);
		glBegin(GL_LINES);
		glVertex3d(p_i[0], p_i[1], p_i[2]);
		glVertex3d(p_j[0], p_j[1], p_j[2]);
		glEnd();
		glLineWidth(1.0f);
	}

	void drawSprings()
	{
		for (size_t s = 0; s < springs.size(); s++)
			drawline(springs[s].i, springs[s].j);
	}
	
	void add_spring(int i, int j, float length, float stiffness)
	{
		springs.push_back(Spring(i, j, length, stiffness));
	}

	void add_fixed_particle(int i)
	{
		fixed_particles.push_back(i);
	}

	void add_obstacle(Vector3f center, float radius)
	{
		obstacles.push_back(Sphere(center, radius));
	}

	void init_f(vector<Vector3f> state, vector<Vector3f> &f)
	{
		for (int i = 0; i < m_numParticles; i++)
		{
			Vector3f v = state[2*i + 1];
			f.push_back(v);

			Vector3f a = Vector3f(0, 0, 0);
			f.push_back(a);
		}
	}

	void apply_gravity_forces(vector<Vector3f> state, vector<Vector3f> &f)
	{
		Vector3f g = Vector3f(0, -9.8, 0);
		
		for (int i = 0; i < m_numParticles; i++)
			f[2*i + 1] += g;
	}


	void apply_drag_forces(vector<Vector3f> state, vector<Vector3f> &f, float drag_coefficient, float mass)
	{
		for (int i = 0; i < m_numParticles; i++)
		{
			Vector3f v = state[2*i + 1];
			f[2*i + 1] += -v * drag_coefficient / mass;
		}
	}

	void apply_spring_forces(vector<Vector3f> state, vector<Vector3f> &f, float mass)
	{
		for (size_t s = 0; s < springs.size(); s++)
		{
			Spring spring = springs[s];
			Vector3f p_i = state[2 * spring.i];
			Vector3f p_j = state[2 * spring.j];
			f[2*spring.i + 1] += spring.getForce(p_i, p_j) / mass;
			f[2*spring.j + 1] += spring.getForce(p_j, p_i) / mass;
		}
	}

	void apply_collision_forces(vector<Vector3f> state, vector<Vector3f> &f, float mass)
	{
		for (size_t o = 0; o < obstacles.size(); o++)
		{
			Vector3f center = obstacles[o].center;
			float radius = obstacles[o].radius;
			
			for (int i = 0; i < m_numParticles; i++)
			{
				Vector3f p_i = state[2*i];
				Vector3f v_i = state[2*i + 1];

				float k = 160;		// spring model for collision response
				float c_paral = 40;	// damping factor
				float c_perp = 5;	// friction effect

				if ((p_i - center).abs() < radius + 0.1)
				{
					float dist = max(((p_i - center).abs() - radius) / 1e-2, 1.0);
					Vector3f d = center - p_i;
					Vector3f n = d / d.abs();
					Vector3f v_paral = Vector3f::dot(v_i, n) * n;
					Vector3f v_perp = v_i - v_paral;
					Vector3f F = -k/(dist)*n
								 -c_paral*v_paral
								 -c_perp*v_perp/v_perp.abs();
					f[2*i + 1] += F;
				}

				// // 1
				// if ((p_i - center + v_i*h).abs() < radius + 1e-2) // && Vector3f::dot(v_i, (p_i - center)) < 0)
				// {
				// 	f[2*i] = ((center + (p_i - center)/(p_i - center).abs() * (radius + 1e-1)) - p_i) / h;
				// 	f[2*i + 1] -= ((center + (p_i - center)/(p_i - center).abs() * (1e-1)) - p_i) / h;

				// 	// best so far
				// 	f[2*i] += (p_i - center);
				// 	f[2*i + 1] += (p_i - center);

				// 	if (Vector3f::dot(f[2*i + 1], (p_i - center)) < 0) {
				// 		f[2*i + 1] = Vector3f(0, 0, 0);
				// 	}
				// }
				// // 2
				// else if((p_i - center + 2*v_i*h).abs() < radius + 1e-2)
				// {
				// 	Vector3f b = p_i - center;
				// 	Vector3f v_paral = Vector3f::dot(v_i, b) * b / b.absSquared();

				// 	f[2*i + 1] = -v_paral / h;
				// }
			}
		}
	}

	void apply_self_collision_forces(vector<Vector3f> state, vector<Vector3f> &f, float scale, float mass)
	{
		int scale_rev = 5;
		float scale2 = scale;
		vector<int> grid[scale_rev][scale_rev][scale_rev];

		for (int i = 0; i < m_numParticles; i++)
		{
			Vector3f p_i = state[2*i];

			int x_ind = int(abs(p_i.x() / scale2)) % scale_rev;
			int y_ind = int(abs(p_i.y() / scale2)) % scale_rev;
			int z_ind = int(abs(p_i.z() / scale2)) % scale_rev;

			// printf("%d %d %d\n", x_ind, y_ind, z_ind);

			vector<int>* cell = &(grid[x_ind][y_ind][z_ind]);
			
			for (unsigned int ind = 0; ind < cell->size(); ind++)
			{
				int j = (*cell)[ind];
				Vector3f p_j = state[2*j];

				if ((p_i - p_j).abs() < scale * 1.5)
				{
					printf("collision detected! %ld\n", cell->size());
					
					float structural_length = 1 * scale;
					float structural_stiffness = 1000 * scale;

					Spring spring = Spring(i, j, structural_length, structural_stiffness);
					f[2*i + 1] += spring.getForce(p_i, p_j) / mass;
					f[2*j + 1] += spring.getForce(p_j, p_i) / mass;
				}


				
			}

			cell->push_back(i);
		}
	}

	void apply_wind_forces(vector<Vector3f> state, vector<Vector3f> &f, double wind, float mass)
	{
		if (!wind_exist)
			return;

		double my_rand = (double) rand() / (RAND_MAX) - 0.5;
		double my_rand2 = (double) rand() / (RAND_MAX) - 0.5;

		for (int i = 0; i < m_numParticles; i++)
		{

			Vector3f wind_force = Vector3f(wind*(my_rand2), 0, wind * i/m_numParticles * (my_rand));
			f[2*i + 1] += wind_force / mass;
		}
	}

	void apply_fixed_particles(vector<Vector3f> state, vector<Vector3f> &f)
	{
		for (size_t ind = 0; ind < fixed_particles.size(); ind++)
		{
			int i = fixed_particles[ind];
			// Vector3f p_i = state[2*i];
			Vector3f p_mid = state[2*fixed_particles[fixed_particles.size()/2]];

			f[2*i] = Vector3f(0, 0, 0);
			f[2*i + 1] = Vector3f(0, 0, 0);

			Vector3f unit[3] = {Vector3f::RIGHT, Vector3f::UP, -Vector3f::FORWARD};
			for (int axis = 0; axis < 3; axis++)
				if (swing[axis])
				{
					if (swing_forwad[axis] && p_mid[axis] > -swing_length)
						f[2*i] += -5 * unit[axis];
					else
					if (!swing_forwad[axis] && p_mid[axis] < swing_length)
						f[2*i] += +5 * unit[axis];
					else {
						swing_forwad[axis] = !swing_forwad[axis];
						f[2*i] += (swing_forwad[axis] ? -5 : 5) * unit[axis];
					}
				}
		}
	}

	bool getSwing(int axis) { return swing[axis]; }
	void toggleSwing(int axis) { swing[axis] = !swing[axis]; }
	
	void toggleWind() { wind_exist = !wind_exist; }

protected:

	vector<Vector3f> m_vVecState;
	vector<Spring> springs;
	vector<int> fixed_particles;
	vector<Sphere> obstacles;

	bool swing[3];
	bool swing_forwad[3];
	float swing_length;

	bool wind_exist;
};

#endif
