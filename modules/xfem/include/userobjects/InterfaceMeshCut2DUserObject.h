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
 * Mesh cutter for 2D material interface problems.
 */

class XFEMMovingInterfaceVelocityBase;

class InterfaceMeshCut2DUserObject : public InterfaceMeshCutUserObjectBase
{
public:
  static InputParameters validParams();

  InterfaceMeshCut2DUserObject(const InputParameters & parameters);

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
  /// Map of element normal
  std::map<unsigned int, Point> _element_normal;
};
