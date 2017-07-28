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

    \author:    <http://www.chai3d.org>
    \author:    Chris Sewell
    \version    1.1
    \date       01/2004
*/
//===========================================================================
/*!
    \file     CCollisionAABBBox.h
*/
//---------------------------------------------------------------------------
#ifndef CCollisionAABBBoxH
#define CCollisionAABBBoxH
//---------------------------------------------------------------------------
#include "CMaths.h"
#include "CTriangle.h"
#include "CVertex.h"
#include <vector>
//---------------------------------------------------------------------------

//===========================================================================
/*!
      \class    cCollisionAABBox
      \brief    cCollisionAABBox contains the properties and methods of an
                axis-aligned bounding box, as used in the AABB collision
                detection algorithm.
*/
//===========================================================================
class cCollisionAABBBox
{
  public:
    // CONSTRUCTOR & DESTRUCTOR:
    //! Default constructor of cCollisionAABBBox.
    cCollisionAABBBox() { };
    //! Constructor of cCollisionAABBBox.
    cCollisionAABBBox(const cVector3d& a_min, const cVector3d& a_max)
        { setValue(a_min, a_max); }
    //! Destructor of cCollisionAABBBox.
    virtual ~cCollisionAABBBox() { };

    // METHODS:
    //! Return the center of the bounding box.
    inline cVector3d getCenter() const { return (m_center); }
    //! Return the extent (half the width) of the bounding box.
    inline cVector3d getExtent() const { return (m_extent); }
    //! Set the center of the bounding box.
    inline void setCenter(const cVector3d& a_center)  { m_center = a_center; }
    //! Set the extent (half the width) of the bounding box.
    inline void setExtent(const cVector3d& a_extent) { m_extent = a_extent; }
    //! Set the center and extent of the box based on two points.
    inline void setValue(const cVector3d& a_min, const cVector3d& a_max)
    {
        m_extent = cMul(0.5, cSub(a_max, a_min));
        m_center = cAdd(a_min, m_extent);
        m_min = a_min;
        m_max = a_max;
    }
    
    
    //===========================================================================
    /*!
    Test whether this box contains the given point.

    \fn       bool cCollisionAABBBox::contains(const cVector3d& a_p) const
    \return   Returns whether this box contains this point
    */
    //===========================================================================
    inline bool contains(const cVector3d& a_p) const {
      // check that each of the point's coordinates are within the box's range
      if (a_p.x > m_min.x && a_p.y > m_min.y && a_p.z > m_min.z &&
        a_p.x < m_max.x && a_p.y < m_max.y && a_p.z < m_max.z) return true;
      return false;
    }


    //===========================================================================
    /*!
    Set the bounding box to bound the two given bounding boxes.

    \fn       void cCollisionAABBBox::enclose(const cCollisionAABBBox& a_boxA, const cCollisionAABBBox& a_boxB)
    \param    a_boxA   The first bounding box to be enclosed.
    \param    a_boxB   The other bounding box to be enclosed.
    */
    //===========================================================================
    inline void enclose(const cCollisionAABBBox& a_boxA, const cCollisionAABBBox& a_boxB) {
      // find the minimum coordinate along each axis
      cVector3d lower(cMin(a_boxA.getLowerX(), a_boxB.getLowerX()),
        cMin(a_boxA.getLowerY(), a_boxB.getLowerY()),
        cMin(a_boxA.getLowerZ(), a_boxB.getLowerZ()));

      // find the maximum coordinate along each axis
      cVector3d upper(cMax(a_boxA.getUpperX(), a_boxB.getUpperX()),
        cMax(a_boxA.getUpperY(), a_boxB.getUpperY()),
        cMax(a_boxA.getUpperZ(), a_boxB.getUpperZ()));

      // set the center and extent of this box to enclose the two extreme points
      setValue(lower, upper);
    }
    

    //===========================================================================
    /*!
    Modify the bounding box as needed to bound the given point.

    \fn       void cCollisionAABBBox::enclose (const cVector3d& a_point)
    \param    a_point  The point to be bounded.
    */
    //===========================================================================
    inline void enclose (const cVector3d& a_point) 
    {
        // decrease coordinates as needed to include given point
        cVector3d lower(cMin(getLowerX(), a_point.x), cMin(getLowerY(), a_point.y),
        cMin(getLowerZ(), a_point.z));

        // increase coordinates as needed to include given point
        cVector3d upper(cMax(getUpperX(), a_point.x), cMax(getUpperY(), a_point.y),
          cMax(getUpperZ(), a_point.z));

        // set the center and extent of this box to enclose the given point
        setValue(lower, upper);
    }
    
    //! Modify the bounding box to bound another box
    inline void enclose(const cCollisionAABBBox& a_box) { enclose(*this, a_box); }

    //! Initialize a bounding box to center at origin and infinite extent.
    inline void setEmpty()
    {
        const double CHAI_INFINITY = 1.0e50;
        m_center.zero();
        m_extent = cVector3d(-CHAI_INFINITY, -CHAI_INFINITY, -CHAI_INFINITY);
        m_min.set(CHAI_INFINITY, CHAI_INFINITY, CHAI_INFINITY);
        m_max.set(-CHAI_INFINITY, -CHAI_INFINITY, -CHAI_INFINITY);
    }

    //! Return the smallest coordinate along X axis.
    inline double getLowerX() const  { return (m_min.x); }
    //! Return the largest coordinate along X axis.
    inline double getUpperX() const  { return (m_max.x); }
    //! Return the smallest coordinate along Y axis.
    inline double getLowerY() const  { return (m_min.y); }
    //! Return the largest coordinate along Y axis.
    inline double getUpperY() const  { return (m_max.y); }
    //! Return the smallest coordinate along Z axis.
    inline double getLowerZ() const  { return (m_min.z); }
    //! Return the largest coordinate along Z axis.
    inline double getUpperZ() const  { return (m_max.z); }
    //! Return the length of the longest axis of the bounding box.
    double size() const;

    //===========================================================================
    /*!
    Return the index of the longest axis of the bounding box.

    \fn       int cCollisionAABBBox::longestAxis() const
    \return   Return the index of the longest axis of the box.
    */
    //===========================================================================
    inline int longestAxis() const {
      // if extent of x axis is greatest, return index 0
      if ((m_extent.x >= m_extent.y) && (m_extent.x >= m_extent.z)) return 0;
      else if ((m_extent.y >= m_extent.x) && (m_extent.y >= m_extent.z)) return 1;
      return 2;          
    }

    //! Draw the edges of the bounding box.
    inline void render()
    {
        cDrawWireBox(m_min.x, m_max.x, m_min.y, m_max.y, m_min.z, m_max.z);
    }

    // PROPERTIES:
    //! The center of the bounding box.
    cVector3d m_center;
    //! The extent (half the width) of the bounding box.
    cVector3d m_extent;
    //! The minimum point (along each axis) of the bounding box.
    cVector3d m_min;
    //! The maximum point (along each axis) of the bounding box.
    cVector3d m_max;
};

//---------------------------------------------------------------------------
#endif
//---------------------------------------------------------------------------

