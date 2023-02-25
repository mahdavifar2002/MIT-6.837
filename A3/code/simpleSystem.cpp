
#include "simpleSystem.h"

using namespace std;

SimpleSystem::SimpleSystem()
{
    m_vVecState.push_back(Vector3f(0.5, 0.5, 0));
}

// for a given state, evaluate f(X,t)
vector<Vector3f> SimpleSystem::evalF(vector<Vector3f> state)
{
	vector<Vector3f> f;

	for (size_t i = 0; i < state.size(); i ++)
	{
		Vector3f particle = state[i];
		Vector3f f_particle = Vector3f(-particle.y(), particle.x(), 0);
		f.push_back(f_particle);
	}

	return f;
}

// render the system (ie draw the particles)
void SimpleSystem::draw()
{
	vector<Vector3f> state = getState();
	for (size_t i = 0; i < state.size(); i++)
	{
		Vector3f pos = state[i];//YOUR PARTICLE POSITION
		glPushMatrix();
		glTranslatef(pos[0], pos[1], pos[2] );
		glutSolidSphere(0.075f,10.0f,10.0f);
		glPopMatrix();
	}
}
