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

  virtual void initialize() override;
  virtual void execute() override{};
  virtual void finalize() override{};
  virtual const std::vector<Point>
  getCrackFrontPoints(unsigned int num_crack_front_points) const override;

  virtual bool active(Real time) const override;
  virtual bool cutElementByGeometry(const Elem * elem,
                                    std::vector<CutEdge> & cut_edges,
                                    std::vector<CutNode> & cut_nodes,
                                    Real time) const override;
  virtual bool cutElementByGeometry(const Elem * elem,
                                    std::vector<CutFace> & cut_faces,
                                    Real time) const override;
  virtual bool cutFragmentByGeometry(std::vector<std::vector<Point>> & frag_edges,
                                     std::vector<CutEdge> & cut_edges,
                                     Real time) const override;
  virtual bool cutFragmentByGeometry(std::vector<std::vector<Point>> & frag_faces,
                                     std::vector<CutFace> & cut_faces,
                                     Real time) const override;

protected:
  std::unique_ptr<MeshBase> _cut_mesh;
  const unsigned int _cut_elem_nnode = 3; // the cutter mesh has triangluar elements only
  const Real _const_intersection = 0.01;  // used to define intersection points
  std::vector<BoundaryID> _boundary;
  std::vector<std::vector<BoundaryID>> _front;
  std::map<BoundaryID, std::shared_ptr<CutPoint>> _boundary_map;
  std::vector<std::shared_ptr<CutEdge>> _boundary_edges;
  Real _size_control; // _size_control is used for cutter mesh refinement and front advancement
  MooseMesh & _mesh;
  std::vector<std::vector<BoundaryID>> _active_boundary;
  std::vector<std::vector<Point>> _active_direction;
  std::vector<unsigned int> _inactive_boundary_pos;
  bool _stop;
  unsigned int _n_step_growth;
  bool _grow;

  // check if a line intersects withunsigned intof an element
  virtual bool intersectWithEdge(const Point & p1,
                                 const Point & p2,
                                 const std::vector<Point> & _vertices,
                                 Point & pint) const;

  // find directional intersection along the positive extension of the vector from p1 to p2
  bool findIntersection(const Point & p1,
                        const Point & p2,
                        const std::vector<Point> & vertices,
                        Point & pint) const;

  // check if point p is inside the edge p1-p2
  bool isInsideEdge(const Point & p1, const Point & p2, const Point & p) const;

  // get the relative position of p from p1
  Real getRelativePosition(const Point & p1, const Point & p2, const Point & p) const;

  // check if point p is inside a plane
  bool isInsideCutPlane(const std::vector<Point> & _vertices, Point p) const;

  // find the normal vector to an element
  void findElemNormal(const std::vector<Point> & _vertices, Point & _normal, Point & _center) const;

  // find boundary nodes of the cutter mesh
  // this is a simple algorithm simply based on the added angle = 360 degrees
  // works fine for planar cutting surface for curved cutting surface, need to re-work this
  // subroutine to make it more general
  void findBoundaryNodes();

  // find boundary edges of the cutter mesh
  void findBoundaryEdges();

  // sort boundary nodes to be in the right order along the boundary
  void sortBoundaryNodes();

  // find distance between two nodes
  Real findDistance(dof_id_type node1, dof_id_type node2);

  // if boundary nodes are too sparse, add nodes in between
  void refineBoundary();

  // find all active boundary nodes in the cutter mesh
  // find boundary nodes that will grow; nodes outside of the structural mesh are inactive
  void findActiveBoundaryNodes();

  // find growth direction at each active node
  void findActiveBoundaryDirection();

  // grow the cutter mesh
  void growFront();

  // sort the front nodes
  void sortFrontNodes();

  // find front-structure intersections
  void findFrontIntersection();

  // refine the mesh at the front
  void refineFront();

  // create tri3 elements between the new front and the old front
  void triangulation();

  // join active boundaries and inactive boundaries to be the new boundary
  void joinBoundary();
};

#endif
