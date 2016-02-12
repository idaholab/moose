/****************************************************************/
/*               DO NOT MODIFY THIS HEADER                      */
/* MOOSE - Multiphysics Object Oriented Simulation Environment  */
/*                                                              */
/*           (c) 2010 Battelle Energy Alliance, LLC             */
/*                   ALL RIGHTS RESERVED                        */
/*                                                              */
/*          Prepared by Battelle Energy Alliance, LLC           */
/*            Under Contract No. DE-AC07-05ID14517              */
/*            With the U. S. Department of Energy               */
/*                                                              */
/*            See COPYRIGHT for full restrictions               */
/****************************************************************/

#ifndef XFEM_GEOMETRIC_CUT_3D_H
#define XFEM_GEOMETRIC_CUT_3D_H

#include "XFEMGeometricCut.h"

using namespace libMesh;

class XFEMGeometricCut3D : public XFEMGeometricCut
{
public:

  XFEMGeometricCut3D(Real t0, Real t1);

  virtual ~XFEMGeometricCut3D();

  virtual bool cutElementByGeometry(const Elem* elem,
                                    std::vector<cutEdge> & cutEdges,
                                    Real time);
  virtual bool cutElementByGeometry(const Elem* elem,
                                    std::vector<cutFace> & cutFaces,
                                    Real time);

  virtual bool cutFragmentByGeometry(std::vector<std::vector<Point> > & frag_edges,
                                    std::vector<cutEdge> & cutEdges,
                                    Real time);
  virtual bool cutFragmentByGeometry(std::vector<std::vector<Point> > & frag_faces,
                                    std::vector<cutFace> & cutFaces,
                                    Real time);

protected:

  Point _center;
  Point _normal;

  virtual bool intersectWithEdge(Point p1, Point p2, Point &pint);

  virtual bool isInsideCutPlane(Point p) = 0;

  bool isInsideEdge(Point p1, Point p2, Point p);

  Real getRelativePosition(Point p1, Point p2, Point p);
};


#endif
