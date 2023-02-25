
#include "pendulumSystem.h"

PendulumSystem::PendulumSystem(int numParticles):ParticleSystem(numParticles)
{
	m_numParticles = numParticles;
	
	// initializing the state based on the number of particles
	for (int i = 0; i < m_numParticles; i++) {
		
		// for this system, we care about the position and the velocity

		if (i == 0) {
			
			m_vVecState.push_back(Vector3f(0, 0, 0));	// x
			m_vVecState.push_back(Vector3f(0, 0, 0));	// v
		}
		else {
			m_vVecState.push_back(Vector3f(i - m_numParticles/2.0, -i, 0));	// x
			m_vVecState.push_back(Vector3f(0, 0, 0));	// v
		}
	}

	float length = 1;
	float stiffness = 50;
	
	for (int i = 0; i < m_numParticles; i++) {
		add_spring(i, mod(i+1), length, stiffness);
	}

	add_fixed_particle(0);
}


// TODO: implement evalF
// for a given state, evaluate f(X,t)
vector<Vector3f> PendulumSystem::evalF(vector<Vector3f> state)
{
	vector<Vector3f> f;

	float mass = 1;
	float drag_coefficient = 0.5;

	init_f(state, f);
	apply_gravity_forces(state, f);
	apply_drag_forces(state, f, drag_coefficient, mass);
	apply_spring_forces(state, f, mass);
	apply_fixed_particles(state, f);

	return f;
}

// render the system (ie draw the particles)
void PendulumSystem::draw()
{
	vector<Vector3f> state = getState();

	for (int i = 0; i < m_numParticles; i++) {
		Vector3f pos = state[2*i];//  position of particle i
		glPushMatrix();
		glTranslatef(pos[0], pos[1], pos[2] );
		glutSolidSphere(0.075f,10.0f,10.0f);
		glPopMatrix();

		drawSprings();
	}
}
