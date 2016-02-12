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

#ifndef XFEM_GEOMETRIC_CUT_2D_H
#define XFEM_GEOMETRIC_CUT_2D_H

#include "XFEMGeometricCut.h"

class XFEMGeometricCut2D : public XFEMGeometricCut
{
public:

  XFEMGeometricCut2D(Real x0_, Real y0_, Real x1_, Real y1_, Real t_start_, Real t_end_);
  ~XFEMGeometricCut2D();

  virtual bool cutElementByGeometry(const Elem* elem, std::vector<cutEdge> & cutEdges, Real time);
  virtual bool cutElementByGeometry(const Elem* elem, std::vector<cutFace> & cutFaces, Real time);

  virtual bool cutFragmentByGeometry(std::vector<std::vector<Point> > & frag_edges,
                            std::vector<cutEdge> & cutEdges, Real time);
  virtual bool cutFragmentByGeometry(std::vector<std::vector<Point> > & frag_faces,
                            std::vector<cutFace> & cutFaces, Real time);

private:
  Real x0, x1, y0, y1;
};

#endif
