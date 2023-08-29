/****************************************************************************/
/*                        DO NOT MODIFY THIS HEADER                         */
/*                                                                          */
/* MALAMUTE: MOOSE Application Library for Advanced Manufacturing UTilitiEs */
/*                                                                          */
/*           Copyright 2021 - 2023, Battelle Energy Alliance, LLC           */
/*                           ALL RIGHTS RESERVED                            */
/****************************************************************************/

#pragma once

#include "ADKernelValue.h"

class PhaseFieldTwoPhaseSurfaceTension : public ADVectorKernelValue
{
public:
  static InputParameters validParams();  
  PhaseFieldTwoPhaseSurfaceTension(const InputParameters & parameters);

protected:
  virtual ADRealVectorValue precomputeQpResidual() override;

  const ADVariableValue & _auxpf;
  const ADVariableGradient & _grad_pf;
  const Real & _coeff;

};
