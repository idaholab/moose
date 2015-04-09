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

#include "XFEM_geometric_cut.h"

class XFEM_geometric_cut_2d : public XFEM_geometric_cut
{
public:

  XFEM_geometric_cut_2d(Real x0_, Real y0_, Real x1_, Real y1_, Real t_start_, Real t_end_);
  ~XFEM_geometric_cut_2d();

  bool cut_elem_by_geometry(const Elem* elem, std::vector<cutEdge> & cutEdges, Real time);
  bool cut_elem_by_geometry(const Elem* elem, std::vector<cutFace> & cutFaces, Real time);

  bool cut_frag_by_geometry(std::vector<std::vector<Point> > & frag_edges,
                            std::vector<cutEdge> & cutEdges, Real time);
  bool cut_frag_by_geometry(std::vector<std::vector<Point> > & frag_faces,
                            std::vector<cutFace> & cutFaces, Real time);

private:
  Real x0, x1, y0, y1;
};

#endif
