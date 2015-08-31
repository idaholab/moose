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

#ifndef XFEM_SQUARE_CUT_H
#define XFEM_SQUARE_CUT_H

#include "XFEM_geometric_cut.h"

class XFEM_square_cut : public XFEM_geometric_cut
{
public:

  XFEM_square_cut(std::vector<Real> square_nodes);
  ~XFEM_square_cut();

  virtual bool cut_elem_by_geometry(const Elem* elem, std::vector<cutEdge> & cutEdges, Real time);
  virtual bool cut_elem_by_geometry(const Elem* elem, std::vector<cutFace> & cutFaces, Real time);

  virtual bool cut_frag_by_geometry(std::vector<std::vector<Point> > & frag_edges,
                            std::vector<cutEdge> & cutEdges, Real time);
  virtual bool cut_frag_by_geometry(std::vector<std::vector<Point> > & frag_faces,
                            std::vector<cutFace> & cutFaces, Real time);

private:

  std::vector<Point> _vertices;
  Point _center;
  Point _normal;

private:

  bool intersect_with_edge(Point p1, Point p2, Point &pint);
  bool isInsideCutPlane(Point p);
};

#endif
