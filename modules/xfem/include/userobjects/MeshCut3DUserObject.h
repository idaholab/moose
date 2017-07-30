/****************************************************************/
/* MOOSE - Multiphysics Object Oriented Simulation Environment  */
/*                                                              */
/*          All contents are licensed under LGPL V2.1           */
/*             See LICENSE for full restrictions                */
/****************************************************************/

#ifndef MESH_CUT_3D_USEROBJECT_H
#define MESH_CUT_3D_USEROBJECT_H

#include "GeometricCutUserObject.h"

// Forward declarations
class MeshCut3DUserObject;

template <>
InputParameters validParams<MeshCut3DUserObject>();

class MeshCut3DUserObject : public GeometricCutUserObject
{
public:
  MeshCut3DUserObject(const InputParameters & parameters);
  ~MeshCut3DUserObject();
  virtual bool active(Real time) const;
  virtual bool
  cutElementByGeometry(const Elem * elem, std::vector<CutEdge> & cut_edges, Real time) const;
  virtual bool
  cutElementByGeometry(const Elem * elem, std::vector<CutFace> & cut_faces, Real time) const;
  virtual bool cutFragmentByGeometry(std::vector<std::vector<Point>> & frag_edges,
                                     std::vector<CutEdge> & cut_edges,
                                     Real time) const;
  virtual bool cutFragmentByGeometry(std::vector<std::vector<Point>> & frag_faces,
                                     std::vector<CutFace> & cut_faces,
                                     Real time) const;

protected:
  std::unique_ptr<MeshBase> _cut_mesh;
  std::vector<int> _boundary;
  std::vector<std::vector<int>> _front;
  std::map<int, CutPoint *> _boundary_map;
  std::vector<CutEdge *> _boundary_edges;
  Real _size_control;
  MooseMesh & _mesh;
  std::vector<std::vector<int>> _active_boundary;
  std::vector<std::vector<Point>> _active_direction;
  std::vector<int> _inactive_boundary;
  bool _stop;
  unsigned int _n_step_growth;
  bool _grow;

  virtual bool intersectWithEdge(const Point & p1,
                                 const Point & p2,
                                 const std::vector<Point> & _vertices,
                                 Point & pint) const;
  bool findIntersection(const Point & p1,
                        const Point & p2,
                        const std::vector<Point> & vertices,
                        Point & pint) const;
  bool isInsideEdge(const Point & p1, const Point & p2, const Point & p) const;
  Real getRelativePosition(const Point & p1, const Point & p2, const Point & p) const;
  bool isInsideCutPlane(const std::vector<Point> & _vertices, Point p) const;
  bool findElemNormal(const std::vector<Point> & _vertices, Point & _normal, Point & _center) const;
  void findBoundaryNodes();
  void findBoundaryEdges();
  void sortBoundaryNodes();
  Real findDistance(unsigned int node1, unsigned int node2);
  void refineBoundary();
  void findActiveBoundaryNodes();
  void findActiveBoundaryDirection();
  void growFront();
  void sortFrontNodes();
  void findFrontIntersection();
  void refineFront();
  void triangulation();
  void join();
};

#endif
