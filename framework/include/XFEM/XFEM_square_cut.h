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

  Point _v1;
  Point _v2;
  Point _v3;
  Point _v4;
  Point _center;
  Point _normal;

private:

  bool intersect_with_edge(Point p1, Point p2, Point &pint);
  int plane_normal_line_exp_int_3d(double pp[3], double normal[3], double p1[3], double p2[3], double pint[3]);
  bool line_exp_is_degenerate_nd(int dim_num, double p1[], double p2[]);
  double r8vec_norm(int n, double a[]);
  void r8vec_copy(int n, double a1[], double a2[]);
  bool r8vec_eq(int n, double a1[], double a2[]);
  double r8vec_dot_product(int n, double a1[], double a2[]);
  Point cross_product(Point p1, Point p2);
  Real dot_product(Point p1, Point p2);
  bool isInsideCutPlane(Point p);
  bool isInsideEdge(Point p1, Point p2, Point p);
  Real getRelativePosition(Point p1, Point p2, Point p);
  void normalize(Point & p);
};

#endif
