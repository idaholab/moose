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

#ifndef GEOMETRIC_CUT_H
#define GEOMETRIC_CUT_H

#include "libmesh/libmesh_common.h"
#include "libmesh/libmesh.h" // libMesh::invalid_uint
#include "libmesh/elem.h"

struct cutEdge
{
  unsigned int id1;
  unsigned int id2;
  Real distance;
};

class XFEM_geometric_cut
{
public:

  explicit
  XFEM_geometric_cut();

  XFEM_geometric_cut(Real x0_, Real y0_, Real x1_, Real y1_, Real t_start_, Real t_end_);

  ~XFEM_geometric_cut();

  bool cut_elem_by_geometry(const Elem* elem, std::vector<cutEdge> & cutEdges, Real time);

  Real crossprod_2d(Real ax, Real ay, Real bx, Real by);
  Real cut_fraction(Real time);

private:
  Real x0, x1, y0, y1;
  Real t_start, t_end;

};


#endif
