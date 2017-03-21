/****************************************************************/
/* MOOSE - Multiphysics Object Oriented Simulation Environment  */
/*                                                              */
/*          All contents are licensed under LGPL V2.1           */
/*             See LICENSE for full restrictions                */
/****************************************************************/

#ifndef XFEM_GEOMETRIC_CUT_H
#define XFEM_GEOMETRIC_CUT_H

#include "libmesh/libmesh_common.h"
#include "libmesh/libmesh.h" // libMesh::invalid_uint
#include "libmesh/elem.h"

using namespace libMesh;

struct CutEdge
{
  unsigned int id1;
  unsigned int id2;
  Real distance;
  unsigned int host_side_id;
};

struct CutFace
{
  unsigned int face_id;
  std::vector<unsigned int> face_edge;
  std::vector<Real> position;
};

class XFEMGeometricCut
{
public:
  XFEMGeometricCut(Real t0, Real t1);
  virtual ~XFEMGeometricCut();

  virtual bool active(Real time) = 0;

  virtual bool
  cutElementByGeometry(const Elem * elem, std::vector<CutEdge> & cut_edges, Real time) = 0;
  virtual bool
  cutElementByGeometry(const Elem * elem, std::vector<CutFace> & cut_faces, Real time) = 0;

  virtual bool cutFragmentByGeometry(std::vector<std::vector<Point>> & frag_edges,
                                     std::vector<CutEdge> & cut_edges,
                                     Real time) = 0;
  virtual bool cutFragmentByGeometry(std::vector<std::vector<Point>> & frag_faces,
                                     std::vector<CutFace> & cut_faces,
                                     Real time) = 0;

  Real cutFraction(Real time);

protected:
  Real _t_start;
  Real _t_end;
  Real crossProduct2D(Real ax, Real ay, Real bx, Real by);
};

#endif
