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

#ifndef XFEM_GEOMETRIC_CUT_H
#define XFEM_GEOMETRIC_CUT_H

#include "libmesh/libmesh_common.h"
#include "libmesh/libmesh.h" // libMesh::invalid_uint
#include "libmesh/elem.h"
#include "MooseError.h"

using namespace libMesh;

struct cutEdge
{
  unsigned int id1;
  unsigned int id2;
  Real distance;
  unsigned int host_side_id;
};

struct cutFace
{
  unsigned int face_id;
  unsigned int face_edge1;
  unsigned int face_edge2;
  Real dist1;
  Real dist2;
};

class XFEM_geometric_cut
{
public:

  XFEM_geometric_cut(Real t0, Real t1);
  virtual ~XFEM_geometric_cut();

  virtual bool cut_elem_by_geometry(const Elem* elem, std::vector<cutEdge> & cutEdges,
                                    Real time) = 0;
  virtual bool cut_elem_by_geometry(const Elem* elem, std::vector<cutFace> & cutFaces,
                                    Real time) = 0;

  virtual bool cut_frag_by_geometry(std::vector<std::vector<Point> > & frag_edges,
                                    std::vector<cutEdge> & cutEdges, Real time) = 0;
  virtual bool cut_frag_by_geometry(std::vector<std::vector<Point> > & frag_faces,
                                    std::vector<cutFace> & cutFaces, Real time) = 0;

  Real crossprod_2d(Real ax, Real ay, Real bx, Real by);
  Real cut_fraction(Real time);

protected:
  Real t_start, t_end;
};


#endif
