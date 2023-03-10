#ifndef EXTRA_H
#define EXTRA_H

#ifdef WIN32
#include <windows.h>
#endif
#include <GL/gl.h>

#ifndef M_PI
#define M_PI  3.14159265358979
#endif

// Inline functions to help with drawing
inline void glVertex( const Vector3f& a )
{
    glVertex3fv(a.m_elements);
}

inline void glNormal( const Vector3f& a )
{
    glNormal3fv(a.m_elements);
}

inline void glLoadMatrix( const Matrix4f& m )
{
    glLoadMatrixf( m.m_elements );
}

inline void glMultMatrix( const Matrix4f& m )
{
    glMultMatrixf( m.m_elements );
}
#endif
