/****************************************************************/
/* MOOSE - Multiphysics Object Oriented Simulation Environment  */
/*                                                              */
/*          All contents are licensed under LGPL V2.1           */
/*             See LICENSE for full restrictions                */
/****************************************************************/

#ifndef XFEM_GEOMETRIC_CUT_2D_H
#define XFEM_GEOMETRIC_CUT_2D_H

#include "XFEMGeometricCut.h"

class XFEMGeometricCut2D : public XFEMGeometricCut
{
public:

  XFEMGeometricCut2D(Real x0, Real y0, Real x1, Real y1, Real t_start, Real t_end);
  ~XFEMGeometricCut2D();

  virtual bool cutElementByGeometry(const Elem* elem, std::vector<CutEdge> & cut_edges, Real time);
  virtual bool cutElementByGeometry(const Elem* elem, std::vector<CutFace> & cut_faces, Real time);

  virtual bool cutFragmentByGeometry(std::vector<std::vector<Point> > & frag_edges,
                            std::vector<CutEdge> & cut_edges, Real time);
  virtual bool cutFragmentByGeometry(std::vector<std::vector<Point> > & frag_faces,
                            std::vector<CutFace> & cut_faces, Real time);

private:
  Real _x0;
  Real _x1;
  Real _y0;
  Real _y1;
};

#endif
