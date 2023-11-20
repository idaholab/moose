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
 * Optimization reporter that interfaces with TAO. Objective value, gradients,
 * and constraints need to calculated in the subapps.
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

  std::vector<dof_id_type> * _num_values_reporter;

  virtual dof_id_type getNumParams() const override;

  virtual void setICsandBounds() override;

private:
};
