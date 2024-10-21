//* This file is part of the MOOSE framework
//* https://www.mooseframework.org
//*
//* All rights reserved, see COPYRIGHT for full restrictions
//* https://github.com/idaholab/moose/blob/master/COPYRIGHT
//*
//* Licensed under LGPL 2.1, please see LICENSE for details
//* https://www.gnu.org/licenses/lgpl-2.1.html

#pragma once

#include "GeneralUserObject.h"
#include "MooseMesh.h"

class NonlinearSystemBase;

/**
 * Renormalization of a vector variable (i.e. a set of variables comprising a vector)
 */
class PointwiseRenormalizeVector : public GeneralUserObject
{
public:
  static InputParameters validParams();

  PointwiseRenormalizeVector(const InputParameters & parameters);

  void initialize() override;
  void execute() override;
  void finalize() override;

  // only needed for ElementUserObjects and NodalUseroObjects
  void threadJoin(const UserObject &) override {}

protected:
  /// reference to the mesh
  MooseMesh & _mesh;

  /// names of the variables to renormalize
  const std::vector<VariableName> & _var_names;

  // internal ID numbers of the variables to renormalize
  std::vector<unsigned int> _var_numbers;

  /// desired norm
  const Real _target_norm;
};
