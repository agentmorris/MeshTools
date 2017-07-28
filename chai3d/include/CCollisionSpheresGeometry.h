//===========================================================================
/*
    This file is part of the CHAI 3D visualization and haptics libraries.
    Copyright (C) 2003-2004 by CHAI 3D. All rights reserved.

    This library is free software; you can redistribute it and/or modify
    it under the terms of the GNU General Public License("GPL") version 2
    as published by the Free Software Foundation.

    For using the CHAI 3D libraries with software that can not be combined
    with the GNU GPL, and for taking advantage of the additional benefits
    of our support services, please contact CHAI 3D about acquiring a
    Professional Edition License.

    \author     Chris Sewell
    \file       CCollisionSpheresGeometry.h
    \version    1.0
    \date       01/2004
*/
//===========================================================================

//---------------------------------------------------------------------------
#ifndef CCollisionSpheresGeometryH
#define CCollisionSpheresGeometryH
//---------------------------------------------------------------------------
#include "CMaths.h"
#include "CTriangle.h"
#include <assert.h>
#include <list>
#include <map>
#include <math.h>
#ifndef _LINUX
#include <windows.h>
#endif
#include <GL/gl.h>
//---------------------------------------------------------------------------
using std::map;
using std::less;
using std::list;
/*!
    \file CCollisionSpheresGeometry.h
*/
//---------------------------------------------------------------------------
// FORWARD DECLARATIONS:
class cCollisionSpheresPoint;
class cCollisionSpheresEdge;
class cCollisionSpheresLeaf;

// TYPEDEFS:
//! Map of points to the edges they form.
typedef map<cCollisionSpheresPoint *, cCollisionSpheresEdge *,
        less<cCollisionSpheresPoint *> > PtEmap;
//---------------------------------------------------------------------------

//===========================================================================
//  - CLASS DEFINITION -
/*!
      \class    cCollisionSpheresPoint
      \brief    cCollisionSpheresPoint defines points used in the primitive
                shapes.
*/
//===========================================================================
class cCollisionSpheresPoint
{
  public:
    // CONSTRUCTOR:
    //! Constructor of cCollisionSpheresPoint.
    cCollisionSpheresPoint(double a_x = 0, double a_y = 0, double a_z = 0)
        { m_pos.x = a_x;  m_pos.y = a_y;  m_pos.z = a_z; }

    // PROPERTIES:
    //! Position of the point.
    cVector3d m_pos;

    //! Map of edges which have this point as an endpoint.
    PtEmap m_edgeMap;
};


//===========================================================================
//  - CLASS DEFINITION -
/*!
      \class    cCollisionSpheresEdge
      \brief    cCollisionSpheresEdge defines edges of shape primitives.
*/
//===========================================================================
class cCollisionSpheresEdge
{
  public:
    // CONSTRUCTOR:
    //! Constructor of cCollisionSpheresEdge.
    cCollisionSpheresEdge() { }
    cCollisionSpheresEdge(cCollisionSpheresPoint *a_a, cCollisionSpheresPoint *a_b) {
      initialize(a_a,a_b);
    }

    void initialize(cCollisionSpheresPoint *a_a, cCollisionSpheresPoint *a_b);

    // METHODS:
    //! Return the center of the edge.
    inline const cVector3d &getCenter() const  {return m_center;}
    //! Return the radius of the edge.
    inline double getRadius() const
        { if (m_D <= 0.0) return 0.0; return sqrt(m_D)/2; }

  private:
    // PROPERTIES:
    //! The two vertices of the edge.
    cCollisionSpheresPoint *m_end[2];
    //! The center of the edge.
    cVector3d m_center;
    //! The distance between the vertices.
    cVector3d m_d;
    //! The 2-norm of the edge.
    double m_D;
};


//===========================================================================
//  - CLASS DEFINITION -
/*!
      \class    cCollisionSpheresGenericShape
      \brief    cCollisionSpheresGenericShape is an abstract class for shape
                primitives (such as triangles or lines) which are surrounded
                by spheres for the collision detector.
*/
//===========================================================================
class cCollisionSpheresGenericShape
{
  public:
    // CONSTRUCTOR:
    //! Constructor of cCollisionSpheresGenericShape.
    cCollisionSpheresGenericShape() : m_sphere(NULL) { }
    virtual ~cCollisionSpheresGenericShape() {}

    // METHODS:
    //! Return center.
    virtual const cVector3d &getCenter() const = 0;
    //! Return radius.
    virtual double getRadius() const = 0;
    //! Determine whether this primitive intersects the given primitive.
    virtual bool computeCollision(cCollisionSpheresGenericShape *a_other,
            cGenericObject*& a_colObject, cTriangle*& a_colTriangle,
            cVector3d& a_colPoint, double& a_colSquareDistance) = 0;
    //! Return pointer to bounding sphere of this primitive shape.
    virtual cCollisionSpheresLeaf* getSphere()  { return m_sphere; }
    //! Set pointer for the bounding sphere of this primitive shape.
    virtual void setSphere(cCollisionSpheresLeaf* a_sphere)
        { m_sphere = a_sphere; }
    //! Overloaded less than operator (for sorting).
    bool operator<(cCollisionSpheresGenericShape* a_other)
        { return (getCenter().get(m_split) < a_other->getCenter().get(m_split)); }

    // STATIC PROPERTIES:
    //! Axis on which to sort.
    static int m_split;

  private:
    // PROPERTIES:
    //! Pointer to the collision sphere surrounding the primitive.
    cCollisionSpheresLeaf *m_sphere;
};


//===========================================================================
//  - CLASS DEFINITION -
/*!
      \class    cCollisionSpheresTri
      \brief    cCollisionSpheresTri defines the triangle primitives that
                make up the mesh and are bounded by the collision spheres.
                It is essentially just a wrapper around a cTriangle object,
                to which it has a pointer (m_original).
*/
//===========================================================================
class cCollisionSpheresTri : public cCollisionSpheresGenericShape
{
  public:
    // CONSTRUCTOR:
    //! Constructor of cCollisionSpheresTri.
    cCollisionSpheresTri(cVector3d a, cVector3d b, cVector3d c);
    virtual ~cCollisionSpheresTri();

    // METHODS:
    //! Return whether triangle collides with given line.
    bool computeCollision(cCollisionSpheresGenericShape *a_other,
            cGenericObject*& a_colObject, cTriangle*& a_colTriangle,
            cVector3d& a_colPoint, double& a_colSquareDistance);
    //! Return the center of the triangle.
    inline const cVector3d &getCenter() const  { return m_center; }
    //! Return the radius of the triangle.
    inline double getRadius() const  { return m_radius; }
    //! Returns the cTriangle object in the mesh associated with this triangle.
    cTriangle* getOriginal() { return m_original; }
    //! Sets the cTriangle object in the mesh associated with this triangle.
    void setOriginal(cTriangle* a_original) { m_original = a_original; }

  protected:
    // PROPERTIES:
    //! The vertices of the triangle.
    cCollisionSpheresPoint m_corner[3];
    //! The edges of the triangle.
    cCollisionSpheresEdge m_side[3];
    //! The center of the triangle.
    cVector3d m_center;
    //! The radius of the triangle.
    double m_radius;
    //! The cTriangle object in the mesh associated with this triangle.
    cTriangle* m_original;
};


//===========================================================================
//  - CLASS DEFINITION -
/*!
      \class    cCollisionSpheresLine
      \brief    cCollisionSpheresLine defines a line primitive that may
                collide with other primitives.  It is used by the proxy
                algorithm.
*/
//===========================================================================
class cCollisionSpheresLine : public cCollisionSpheresGenericShape
{
  public:
    // CONSTRUCTOR:
    //! Constructor of cCollisionSpheresLine.
    cCollisionSpheresLine(cVector3d& a_segmentPointA, cVector3d& a_segmentPointB);
    virtual ~cCollisionSpheresLine() {}

    //! METHODS:
    //! Return the center of the line.
    inline const cVector3d &getCenter() const  { return m_center; }
    //! Return the radius of the line.
    inline double getRadius() const  { return m_radius; }
    //! Return whether this line intersects the given triangle.
    bool computeCollision(cCollisionSpheresGenericShape *a_other,
                  cGenericObject*& a_colObject, cTriangle*& a_colTriangle,
                  cVector3d& a_colPoint, double& a_colSquareDistance);
    //! Get first endpoint of the line.
    cVector3d getSegmentPointA() { return m_segmentPointA; }
    //! Get direction vector of the line.
    cVector3d getDir() { return m_dir; }

  protected:
    // PROPERTIES:
    //! The center of the line.
    cVector3d m_center;
    //! The radius of the line.
    double m_radius;
    //! The first endpoint of the line.
    cVector3d m_segmentPointA;
    //! The direction vector of the line.
    cVector3d m_dir;
};

//---------------------------------------------------------------------------
#endif
//---------------------------------------------------------------------------
