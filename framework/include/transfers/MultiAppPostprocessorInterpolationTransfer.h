//* This file is part of the MOOSE framework
//* https://www.mooseframework.org
//*
//* All rights reserved, see COPYRIGHT for full restrictions
//* https://github.com/idaholab/moose/blob/master/COPYRIGHT
//*
//* Licensed under LGPL 2.1, please see LICENSE for details
//* https://www.gnu.org/licenses/lgpl-2.1.html

#pragma once

// MOOSE includes
#include "MultiAppTransfer.h"

// Forward declarations
class MultiAppPostprocessorInterpolationTransfer;

template <>
InputParameters validParams<MultiAppPostprocessorInterpolationTransfer>();

/**
 * Transfers from spatially varying PostprocessorInterpolations in a MultiApp to the "master"
 * system.
 */
class MultiAppPostprocessorInterpolationTransfer : public MultiAppTransfer
{
public:
  static InputParameters validParams();

  MultiAppPostprocessorInterpolationTransfer(const InputParameters & parameters);

  virtual void execute() override;

protected:
  PostprocessorName _postprocessor;
  AuxVariableName _to_var_name;

  unsigned int _num_points;
  Real _power;
  MooseEnum _interp_type;
  Real _radius;

  bool _nodal;
};
