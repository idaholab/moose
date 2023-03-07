//* This file is part of the MOOSE framework
//* https://www.mooseframework.org
//*
//* All rights reserved, see COPYRIGHT for full restrictions
//* https://github.com/idaholab/moose/blob/master/COPYRIGHT
//*
//* Licensed under LGPL 2.1, please see LICENSE for details
//* https://www.gnu.org/licenses/lgpl-2.1.html

#pragma once

#include "Action.h"
#include "DerivativeMaterialPropertyNameInterface.h"

/**
 * Action that adds the elastic driving force for each order parameter
 */
class PolycrystalElasticDrivingForceAction : public Action,
                                             public DerivativeMaterialPropertyNameInterface
{
public:
  static InputParameters validParams();

  PolycrystalElasticDrivingForceAction(const InputParameters & params);

  virtual void act();

private:
  /// Number of order parameters used in the model
  const unsigned int _op_num;

  /// Base name for the order parameters
  std::string _var_name_base;
  const std::string _base_name;
  std::string _elasticity_tensor_name;
};
