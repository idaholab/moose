/************************************************************/
/*                DO NOT MODIFY THIS HEADER                 */
/*   TMAP8: Tritium Migration Analysis Program, Version 8   */
/*                                                          */
/*   Copyright 2021 - 2022 Battelle Energy Alliance, LLC    */
/*                   ALL RIGHTS RESERVED                    */
/************************************************************/

#pragma once

#include "LMKernel.h"

class CoupledForceLM : public LMKernel
{
public:
  CoupledForceLM(const InputParameters & parameters);

  static InputParameters validParams();

protected:
  ADReal precomputeQpResidual() override;

  const unsigned int _v_var;
  const ADVariableValue & _v;
  const Real _coef;
};
