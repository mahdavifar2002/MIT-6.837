#include "surf.h"
#include "extra.h"
using namespace std;

namespace
{

    // We're only implenting swept surfaces where the profile curve is
    // flat on the xy-plane.  This is a check function.
    static bool checkFlat(const Curve &profile)
    {
        for (unsigned i=0; i<profile.size(); i++)
            if (profile[i].V[2] != 0.0 ||
                profile[i].T[2] != 0.0 ||
                profile[i].N[2] != 0.0)
                return false;

        return true;
    }
}


// Printing a Vector3f structure
void printVec( Vector3f vec )
{
	cerr << "\t>>> " << vec[0] << "," << vec[1] << "," << vec[2] << endl;
}

void generateTriangleMesh(Surface &surface, unsigned sweep_num, unsigned profile_num)
{
	for (unsigned sweep_i = 0; sweep_i < sweep_num; sweep_i++)
	{
		for (unsigned i = 1; i < profile_num; i++)
		{
			surface.VF.push_back(Tup3u(	i-1 + profile_num * sweep_i,
										i   + profile_num * sweep_i,
										i-1 + profile_num * ((sweep_i+1) % sweep_num)));

			surface.VF.push_back(Tup3u(	i   + profile_num * sweep_i,
										i   + profile_num * ((sweep_i+1) % sweep_num),
										i-1 + profile_num * ((sweep_i+1) % sweep_num)));
		}
	}
}

Surface makeSurfRev(const Curve &profile, unsigned steps)
{
    Surface surface;

    if (!checkFlat(profile))
    {
        cerr << "surfRev profile curve must be flat on xy plane." << endl;
        exit(0);
    }

    // TODO: Here you should build the surface.  See surf.h for details.
    const unsigned sweep_num = 15;
    const unsigned profile_num = profile.size();

    for (unsigned sweep_i = 0; sweep_i < sweep_num; sweep_i++)
	{
		float theta = 2*M_PI * sweep_i/sweep_num;
		Matrix3f R_y = Matrix3f::rotateY(theta);

		for (unsigned i = 0; i < profile_num; i++)
		{
			surface.VV.push_back(R_y * profile[i].V);
			surface.VN.push_back(R_y * (-profile[i].N));
		}
	}

	generateTriangleMesh(surface, sweep_num, profile_num);

    cerr << "\t>>> makeSurfRev called.\n\t>>> Returning the surface." << endl;

    return surface;
}

Surface makeGenCyl(const Curve &profile, const Curve &sweep )
{
    Surface surface;

    if (!checkFlat(profile))
    {
        cerr << "genCyl profile curve must be flat on xy plane." << endl;
        exit(0);
    }

    // TODO: Here you should build the surface.  See surf.h for details.
    const unsigned sweep_num = sweep.size();
    const unsigned profile_num = profile.size();

    for (unsigned sweep_i = 0; sweep_i < sweep_num; sweep_i++)
	{
		//				   i goes to ->		 j goes to->	   k goes to ->
		Matrix3f rotation (sweep[sweep_i].N, sweep[sweep_i].B, sweep[sweep_i].T);
		Vector3f translate (sweep[sweep_i].V);

		for (unsigned i = 0; i < profile_num; i++)
		{
			surface.VV.push_back(rotation * profile[i].V + translate);
			surface.VN.push_back(rotation * (-profile[i].N));
		}
	}

	generateTriangleMesh(surface, sweep_num, profile_num);

    cerr << "\t>>> makeGenCyl called.\n\t>>> Returning the surface." <<endl;

    return surface;
}

void drawSurface(const Surface &surface, bool shaded)
{
    // Save current state of OpenGL
    glPushAttrib(GL_ALL_ATTRIB_BITS);

    if (shaded)
    {
        // This will use the current material color and light
        // positions.  Just set these in drawScene();
        glEnable(GL_LIGHTING);
        glPolygonMode(GL_FRONT_AND_BACK, GL_FILL);

        // This tells openGL to *not* draw backwards-facing triangles.
        // This is more efficient, and in addition it will help you
        // make sure that your triangles are drawn in the right order.
        glEnable(GL_CULL_FACE);
        glCullFace(GL_BACK);
    }
    else
    {
        glDisable(GL_LIGHTING);
        glPolygonMode(GL_FRONT_AND_BACK, GL_LINE);

        glColor4f(0.4f,0.4f,0.4f,1.f);
        glLineWidth(1);
    }

    glBegin(GL_TRIANGLES);
    for (unsigned i=0; i<surface.VF.size(); i++)
    {
        glNormal(surface.VN[surface.VF[i][0]]);
        glVertex(surface.VV[surface.VF[i][0]]);
        glNormal(surface.VN[surface.VF[i][1]]);
        glVertex(surface.VV[surface.VF[i][1]]);
        glNormal(surface.VN[surface.VF[i][2]]);
        glVertex(surface.VV[surface.VF[i][2]]);
    }
    glEnd();

    glPopAttrib();
}

void drawNormals(const Surface &surface, float len)
{
    // Save current state of OpenGL
    glPushAttrib(GL_ALL_ATTRIB_BITS);

    glDisable(GL_LIGHTING);
    glColor4f(0,1,1,1);
    glLineWidth(1);

    glBegin(GL_LINES);
    for (unsigned i=0; i<surface.VV.size(); i++)
    {
        glVertex(surface.VV[i]);
        glVertex(surface.VV[i] + surface.VN[i] * len);
    }
    glEnd();

    glPopAttrib();
}

void outputObjFile(ostream &out, const Surface &surface)
{

    for (unsigned i=0; i<surface.VV.size(); i++)
        out << "v  "
            << surface.VV[i][0] << " "
            << surface.VV[i][1] << " "
            << surface.VV[i][2] << endl;

    for (unsigned i=0; i<surface.VN.size(); i++)
        out << "vn "
            << surface.VN[i][0] << " "
            << surface.VN[i][1] << " "
            << surface.VN[i][2] << endl;

    out << "vt  0 0 0" << endl;

    for (unsigned i=0; i<surface.VF.size(); i++)
    {
        out << "f  ";
        for (unsigned j=0; j<3; j++)
        {
            unsigned a = surface.VF[i][j]+1;
            out << a << "/" << "1" << "/" << a << " ";
        }
        out << endl;
    }
}
