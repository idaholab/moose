//* This file is part of the MOOSE framework
//* https://www.mooseframework.org
//*
//* All rights reserved, see COPYRIGHT for full restrictions
//* https://github.com/idaholab/moose/blob/master/COPYRIGHT
//*
//* Licensed under LGPL 2.1, please see LICENSE for details
//* https://www.gnu.org/licenses/lgpl-2.1.html

#pragma once

#include "InterfaceMeshCutUserObjectBase.h"
#include "libmesh/exodusII_io.h"
#include "libmesh/explicit_system.h"
#include "libmesh/equation_systems.h"

/**
 * Mesh cutter for 3D material interface problems.
 */

class XFEMMovingInterfaceVelocityBase;

class InterfaceMeshCut3DUserObject : public InterfaceMeshCutUserObjectBase
{
public:
  static InputParameters validParams();

  InterfaceMeshCut3DUserObject(const InputParameters & parameters);

  virtual void initialize() override;

  virtual bool cutElementByGeometry(const Elem * elem,
                                    std::vector<Xfem::CutEdge> & cut_edges,
                                    std::vector<Xfem::CutNode> & cut_nodes) const override;
  virtual bool cutElementByGeometry(const Elem * elem,
                                    std::vector<Xfem::CutFace> & cut_faces) const override;
  virtual bool cutFragmentByGeometry(std::vector<std::vector<Point>> & frag_edges,
                                     std::vector<Xfem::CutEdge> & cut_edges) const override;
  virtual bool cutFragmentByGeometry(std::vector<std::vector<Point>> & frag_faces,
                                     std::vector<Xfem::CutFace> & cut_faces) const override;

  /**
   * calculate the signed distance value for a given point.
   * @param p Coordinate of point
   * @return Signed distance
   */
  virtual Real calculateSignedDistance(Point p) const override;

  virtual Point nodeNomal(const unsigned int & node_id) override;

protected:
  /**
    Check if a line intersects with an element
   */
  bool intersectWithEdge(const Point & p1,
                         const Point & p2,
                         const std::vector<Point> & _vertices,
                         Point & pint) const;

  /**
    Check if point p is inside the edge p1-p2
   */
  bool isInsideEdge(const Point & p1, const Point & p2, const Point & p) const;

  /**
    Get the relative position of p from p1
   */
  Real getRelativePosition(const Point & p1, const Point & p2, const Point & p) const;

  /**
    Check if point p is inside a plane
   */
  bool isInsideCutPlane(const std::vector<Point> & _vertices, const Point & p) const;

  /// Map of pseudo normal of element plane, three nodes and three sides of each element
  std::map<unsigned int, std::array<Point, 7>> _pseudo_normal;
};
