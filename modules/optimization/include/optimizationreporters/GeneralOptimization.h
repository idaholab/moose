//* This file is part of the MOOSE framework
//* https://www.mooseframework.org
//*
//* All rights reserved, see COPYRIGHT for full restrictions
//* https://github.com/idaholab/moose/blob/master/COPYRIGHT
//*
//* Licensed under LGPL 2.1, please see LICENSE for details
//* https://www.gnu.org/licenses/lgpl-2.1.html

#pragma once

#include "OptimizationReporterBase.h"
#include "libmesh/id_types.h"
#include "libmesh/libmesh_common.h"

/**
 */
class GeneralOptimization : public OptimizationReporterBase
{
public:
  static InputParameters validParams();
  GeneralOptimization(const InputParameters & parameters);

  virtual Real computeObjective() override;

protected:
  /// Reporter that will hold the objective value
  Real & _objective_val;

  int _number_params_value;
  // Reporter that holds the total number of parameters
  int & _number_params;

  virtual dof_id_type getNumParams() const override;
  virtual void setICsandBounds() override;

private:
};
