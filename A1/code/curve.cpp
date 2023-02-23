#include "curve.h"
#include "extra.h"
#ifdef WIN32
#include <windows.h>
#endif
#include <GL/gl.h>
using namespace std;

namespace
{
    // Approximately equal to.  We don't want to use == because of
    // precision issues with floating point.
    inline bool approx( const Vector3f& lhs, const Vector3f& rhs )
    {
        const float eps = 1e-8f;
        return ( lhs - rhs ).absSquared() < eps;
    }


}

// Printing a Vector3f structure
void cerrVec( Vector3f vec )
{
	cerr << "\t>>> " << vec[0] << "," << vec[1] << "," << vec[2] << endl;
}

// Getting f(a) for 0 <= a <= 1
// Source of the method used:
// Solomon, Justin. “Introduction to Computer Graphics (Lecture 2): Cubic curves”.
// Recorded at Youtube, December 30, 2020. https://youtu.be/mUUxnrLpCDc?t=4096.
Vector3f getBezierAt( vector< Vector3f > P, float a )
{
	vector< Vector3f > P_next;

	while ( P.size() > 1 )
	{
//		cout << "P.size = " << P.size() << endl;
		P_next.clear();
		for( unsigned i = 0; i < P.size() - 1; ++i ) {
			P_next.push_back((a) * P[i+1] + (1-a) * P[i]);
//			cerrVec(P_next.back());
		}
		P.swap(P_next);
	}

	return P[0];
}

// Making Curve structure out of a vector of points
Curve makeCurve( const vector< Vector3f >& points )
{
	Curve curve;
	Vector3f V, T, N, B(0, 0, 1);

	for( unsigned i = 0; i < points.size(); ++i )
	{
		V = points[i];
		T = i > 0 ? (points[i] - points[i-1]).normalized() : (points[i+1] - points[i]).normalized();
		N = Vector3f::cross(B, T).normalized();
		B = Vector3f::cross(T, N).normalized();
		curve.push_back(CurvePoint{V, T, N, B});
		cerrVec(T);
	}

    // Check for error in normal vectors for closed curves
    cerr << "\t>>> Fixing closed curve\n";
    CurvePoint front = curve.front();
    CurvePoint back = curve.back();

    if (approx(front.V, back.V))
	{
		Vector3f diff_T = front.T - back.T;
		Vector3f diff_N = front.N - back.N;
		Vector3f diff_B = front.B - back.B;

		for (unsigned i = 0; i < curve.size(); i++)
		{
			curve[i].T += diff_T * (float)i / curve.size();
			curve[i].N += diff_N * (float)i / curve.size();
			curve[i].B += diff_B * (float)i / curve.size();
			curve[i].T.normalize();
			curve[i].N.normalize();
//			curve[i].B.normalize();
			curve[i].B = Vector3f::cross(curve[i].T, curve[i].N).normalized();
		}
    }

	return curve;
}

Curve evalBezier( const vector< Vector3f >& P, unsigned steps )
{
    // Check
    if( P.size() < 4 || P.size() % 3 != 1 )
    {
//        cerr << "evalBezier must be called with 3n+1 control points." << endl;
        exit( 0 );
    }

    // TODO:
    // You should implement this function so that it returns a Curve
    // (e.g., a vector< CurvePoint >).  The variable "steps" tells you
    // the number of points to generate on each piece of the spline.
    // At least, that's how the sample solution is implemented and how
    // the SWP files are written.  But you are free to interpret this
    // variable however you want, so long as you can control the
    // "resolution" of the discretized spline curve with it.

    // Make sure that this function computes all the appropriate
    // Vector3fs for each CurvePoint: V,T,N,B.
    // [NBT] should be unit and orthogonal.

    // Also note that you may assume that all Bezier curves that you
    // receive have G1 continuity.  Otherwise, the TNB will not be
    // be defined at points where this does not hold.

//    cerr << "\t>>> evalBezier has been called with the following input:" << endl;

//    cerr << "\t>>> Control points (type vector< Vector3f >): "<< endl;
    for( unsigned i = 0; i < P.size(); ++i )
    {
//        cerr << "\t>>> " << P[i][0] << "," << P[i][1] << "," << P[i][2] << endl;
    }

//    cerr << "\t>>> Steps (type steps): " << steps << endl;

	// Generating points of the curve
	vector< Vector3f > points;

	for ( unsigned piece = 0; piece < (P.size() - 1) / 3; piece++ )
	{
		// Taking each 4 control points
		vector< Vector3f > bezier (&P[3*piece],&P[3*piece + 4]);

		// pushing `steps` number of points into `points`
		for ( unsigned i = (piece > 0); i <= steps; i++ )
			points.push_back(getBezierAt(bezier, (float)i/steps));
	}

	// Generating all necessary vectors and pushing tuples into curve struct
	Curve curve = makeCurve(points);

    // Right now this will just return this empty curve.
    return curve;
}

Curve evalBspline( const vector< Vector3f >& P, unsigned steps )
{
    // Check
    if( P.size() < 4 )
    {
//        cerr << "evalBspline must be called with 4 or more control points." << endl;
        exit( 0 );
    }

    // TODO:
    // It is suggested that you implement this function by changing
    // basis from B-spline to Bezier.  That way, you can just call
    // your evalBezier function.

//    cerr << "\t>>> evalBSpline has been called with the following input:" << endl;

//    cerr << "\t>>> Control points (type vector< Vector3f >): "<< endl;
    for( unsigned i = 0; i < P.size(); ++i )
    {
//        cerr << "\t>>> " << P[i][0] << "," << P[i][1] << "," << P[i][2] << endl;
    }

    // Matrix to change basis from B-Spline to Bezier
    Matrix4f change_basis (	1./6.,		0,		0,		0,
							2./3.,	2./3.,	1./3.,	1./6.,
							1./6.,	1./3.,	2./3.,	2./3.,
								0,		0,		0,	1./6.);

	// Vector of equivalent Bezier control points
	vector< Vector3f > Q;

	// Iterate through each four B-Spline control points
	for ( unsigned i = 0; i < P.size() - 3; i++ )
	{
		Matrix4f geometry (	Vector4f(P[i + 0], 0),
							Vector4f(P[i + 1], 0),
							Vector4f(P[i + 2], 0),
							Vector4f(P[i + 3], 0));

		geometry = geometry * change_basis;

		Q.push_back( Vector3f(geometry.getCol(0).xyz()) );
		Q.push_back( Vector3f(geometry.getCol(1).xyz()) );
		Q.push_back( Vector3f(geometry.getCol(2).xyz()) );
		if (i == P.size() - 4) Q.push_back( Vector3f(geometry.getCol(3).xyz()) );
	}

	return evalBezier(Q, steps);


//    cerr << "\t>>> Steps (type steps): " << steps << endl;
//    cerr << "\t>>> Returning empty curve." << endl;

    // Return the curve.
    return Curve();
}

Curve evalCircle( float radius, unsigned steps )
{
    // This is a sample function on how to properly initialize a Curve
    // (which is a vector< CurvePoint >).

    // Preallocate a curve with steps+1 CurvePoints
    Curve R( steps+1 );

    // Fill it in counterclockwise
    for( unsigned i = 0; i <= steps; ++i )
    {
        // step from 0 to 2pi
        float t = 2.0f * M_PI * float( i ) / steps;

        // Initialize position
        // We're pivoting counterclockwise around the y-axis
        R[i].V = radius * Vector3f( cos(t), sin(t), 0 );

        // Tangent vector is first derivative
        R[i].T = Vector3f( -sin(t), cos(t), 0 );

        // Normal vector is second derivative
        R[i].N = Vector3f( -cos(t), -sin(t), 0 );

        // Finally, binormal is facing up.
        R[i].B = Vector3f( 0, 0, 1 );
    }

    return R;
}

void drawCurve( const Curve& curve, float framesize )
{
    // Save current state of OpenGL
    glPushAttrib( GL_ALL_ATTRIB_BITS );

    // Setup for line drawing
    glDisable( GL_LIGHTING );
    glColor4f( 1, 1, 1, 1 );
    glLineWidth( 1 );

    // Draw curve
    glBegin( GL_LINE_STRIP );
    for( unsigned i = 0; i < curve.size(); ++i )
    {
        glVertex( curve[ i ].V );
    }
    glEnd();

    glLineWidth( 1 );

    // Draw coordinate frames if framesize nonzero
    if( framesize != 0.0f )
    {
        Matrix4f M;

        for( unsigned i = 0; i < curve.size(); ++i )
        {
            M.setCol( 0, Vector4f( curve[i].N, 0 ) );
            M.setCol( 1, Vector4f( curve[i].B, 0 ) );
            M.setCol( 2, Vector4f( curve[i].T, 0 ) );
            M.setCol( 3, Vector4f( curve[i].V, 1 ) );

            glPushMatrix();
            glMultMatrix( M );
            glScaled( framesize, framesize, framesize );
            glBegin( GL_LINES );
            glColor3f( 1, 0, 0 ); glVertex3d( 0, 0, 0 ); glVertex3d( 1, 0, 0 );
            glColor3f( 0, 1, 0 ); glVertex3d( 0, 0, 0 ); glVertex3d( 0, 1, 0 );
            glColor3f( 0, 0, 1 ); glVertex3d( 0, 0, 0 ); glVertex3d( 0, 0, 1 );
            glEnd();
            glPopMatrix();
        }
    }

    // Pop state
    glPopAttrib();
}

