//* This file is part of the MOOSE framework
//* https://www.mooseframework.org
//*
//* All rights reserved, see COPYRIGHT for full restrictions
//* https://github.com/idaholab/moose/blob/master/COPYRIGHT
//*
//* Licensed under LGPL 2.1, please see LICENSE for details
//* https://www.gnu.org/licenses/lgpl-2.1.html

#pragma once

#include "GeometricCutUserObject.h"

// Forward declarations

class LevelSetCutUserObject : public GeometricCutUserObject
{
public:
  static InputParameters validParams();

  LevelSetCutUserObject(const InputParameters & parameters);

  virtual bool cutElementByGeometry(const Elem * elem,
                                    std::vector<Xfem::CutEdge> & cut_edges,
                                    std::vector<Xfem::CutNode> & cut_nodes) const override;
  virtual bool cutElementByGeometry(const Elem * elem,
                                    std::vector<Xfem::CutFace> & cut_faces) const override;

  virtual bool cutFragmentByGeometry(std::vector<std::vector<Point>> & frag_edges,
                                     std::vector<Xfem::CutEdge> & cut_edges) const override;
  virtual bool cutFragmentByGeometry(std::vector<std::vector<Point>> & frag_faces,
                                     std::vector<Xfem::CutFace> & cut_faces) const override;

  virtual const std::vector<Point>
  getCrackFrontPoints(unsigned int num_crack_front_points) const override;

  virtual const std::vector<RealVectorValue>
  getCrackPlaneNormals(unsigned int num_crack_front_points) const override;

  /**
   * If the levelset value is positive, return 1, otherwise return 0.
   * @param node Pointer to the node
   * @return an unsigned int indicating the side
   */
  virtual CutSubdomainID getCutSubdomainID(const Node * node) const override;

protected:
  /// The variable number of the level set variable we using to define the cuts
  const unsigned int _level_set_var_number;

  /// System reference
  const System & _system;

  /// The subproblem solution vector
  const NumericVector<Number> & _solution;

  /// The ID for the negative side of the cut
  const CutSubdomainID _negative_id;

  /// The ID for the positive side of the cut
  const CutSubdomainID _positive_id;
};
