//* This file is part of the MOOSE framework
//* https://www.mooseframework.org
//*
//* All rights reserved, see COPYRIGHT for full restrictions
//* https://github.com/idaholab/moose/blob/master/COPYRIGHT
//*
//* Licensed under LGPL 2.1, please see LICENSE for details
//* https://www.gnu.org/licenses/lgpl-2.1.html

#pragma once

#include "MortarUserObject.h"

/**
 * Creates dof object to weighted gap map
 */
class TestWeightedGapUserObject : public MortarUserObject
{
public:
  static InputParameters validParams();

  TestWeightedGapUserObject(const InputParameters & parameters);

  virtual void initialize() override;
  virtual void execute() override;
  virtual void finalize() override;
  virtual void reinit() override {}

  Real getValue(const Node & node) const;

protected:
  void computeQpProperties();
  void computeQpIProperties();

  /// The base finite element problem
  FEProblemBase & _fe_problem;

  const MooseVariable & _var;

  /// Whether the dof objects are nodal; if they're not, then they're elemental
  const bool _nodal;

  /// Member for handling change of coordinate systems (xyz, rz, spherical)
  const MooseArray<Real> & _coord;

  /// Vector for computation of weighted gap with nodal normals
  RealVectorValue _qp_gap_nodal;

  /// A map from node to weighted gap and volume
  std::unordered_map<const DofObject *, std::pair<Real, Real>> _dof_to_weighted_gap;

  const VariableTestValue & _test;

  /// Quadrature point index for the mortar segments
  unsigned int _qp = 0;

  /// Test function index
  unsigned int _i = 0;
};

inline Real
TestWeightedGapUserObject::getValue(const Node & node) const
{
  auto & [weighted_gap, volume] =
      libmesh_map_find(_dof_to_weighted_gap, static_cast<const DofObject *>(&node));
  return weighted_gap / volume;
}
