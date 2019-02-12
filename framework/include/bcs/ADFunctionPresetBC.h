//* This file is part of the MOOSE framework
//* https://www.mooseframework.org
//*
//* All rights reserved, see COPYRIGHT for full restrictions
//* https://github.com/idaholab/moose/blob/master/COPYRIGHT
//*
//* Licensed under LGPL 2.1, please see LICENSE for details
//* https://www.gnu.org/licenses/lgpl-2.1.html

#ifndef ADFUNCTIONPRESETBC_H
#define ADFUNCTIONPRESETBC_H

#include "ADPresetNodalBC.h"

// Forward Declarations
template <ComputeStage>
class ADFunctionPresetBC;
class Function;

declareADValidParams(ADFunctionPresetBC);

/**
 * Defines a boundary condition that forces the value to be a user specified
 * function at the boundary.
 */
template <ComputeStage compute_stage>
class ADFunctionPresetBC : public ADPresetNodalBC<compute_stage>
{
public:
  ADFunctionPresetBC(const InputParameters & parameters);

protected:
  /**
   * Evaluate the function at the current quadrature point and timestep.
   */
  virtual ADReal computeQpValue() override;

  /// Function being used for evaluation of this BC
  Function & _func;

  usingPresetNodalBCMembers;
};

#endif // ADFUNCTIONPRESETBC_H
