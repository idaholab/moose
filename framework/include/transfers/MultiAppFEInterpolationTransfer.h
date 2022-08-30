//* This file is part of the MOOSE framework
//* https://www.mooseframework.org
//*
//* All rights reserved, see COPYRIGHT for full restrictions
//* https://github.com/idaholab/moose/blob/master/COPYRIGHT
//*
//* Licensed under LGPL 2.1, please see LICENSE for details
//* https://www.gnu.org/licenses/lgpl-2.1.html

#pragma once

#include "MultiAppConservativeTransfer.h"

/**
 * Transfers a vector of variables. The local variable values are computed for each field using
 * the libmesh mesh_function API, rather than an arbitrary interpolation from values on nearby nodes
 * The interpolation coefficients are tied to the variable type of the transferred variable.
 */
class MultiAppFEInterpolationTransfer : public MultiAppConservativeTransfer
{
public:
  static InputParameters validParams();

  MultiAppFEInterpolationTransfer(const InputParameters & parameters);

  virtual void execute() override;

protected:
  /// The number of variables to transfer
  unsigned int _var_size;
  bool _error_on_miss;

private:
  /**
   * Performs the transfer for the variable of index i
   */
  void transferVariable(unsigned int i);

  bool usesMooseAppCoordTransform() const override { return true; }
};
