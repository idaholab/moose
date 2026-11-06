//* This file is part of the MOOSE framework
//* https://mooseframework.inl.gov
//*
//* All rights reserved, see COPYRIGHT for full restrictions
//* https://github.com/idaholab/moose/blob/master/COPYRIGHT
//*
//* Licensed under LGPL 2.1, please see LICENSE for details
//* https://www.gnu.org/licenses/lgpl-2.1.html

#pragma once

#include "GeneralPostprocessor.h"

#include <unordered_map>

class WeightedGapUserObject;
class AutomaticMortarGeneration;
class MooseMesh;

/**
 * Normal contact force of one mortar contact pair, from its weighted gap user object
 */
class MortarContactForce : public GeneralPostprocessor
{
public:
  static InputParameters validParams();

  MortarContactForce(const InputParameters & parameters);

  virtual void initialize() override;
  virtual void execute() override;
  virtual void finalize() override;
  virtual PostprocessorValue getValue() const override;

protected:
  /// Weighted gap user object of the pair
  const WeightedGapUserObject & _weighted_gap_uo;

  /// Component to report (0, 1, 2), or 3 for the magnitude
  const unsigned int _component;

  /// Displaced mesh, on which the pair's mortar interface lives
  const MooseMesh & _displaced_mesh;

  /// Mortar interface of the pair, which holds the nodal normals
  const AutomaticMortarGeneration & _amg;

  /// Map from a secondary node to its lower-dimensional secondary elements
  const std::unordered_map<dof_id_type, std::vector<const Elem *>> & _node_to_elems;

  /// Contact force
  RealVectorValue _force;
};
