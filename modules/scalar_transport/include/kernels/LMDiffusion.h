/************************************************************/
/*                DO NOT MODIFY THIS HEADER                 */
/*   TMAP8: Tritium Migration Analysis Program, Version 8   */
/*                                                          */
/*   Copyright 2021 - 2022 Battelle Energy Alliance, LLC    */
/*                   ALL RIGHTS RESERVED                    */
/************************************************************/

#pragma once

#include "Kernel.h"

class LMDiffusion : public Kernel
{
public:
  LMDiffusion(const InputParameters & parameters);

  static InputParameters validParams();

protected:
  virtual Real computeQpResidual() override;
  virtual Real computeQpJacobian() override;
  virtual Real computeQpOffDiagJacobian(unsigned int jvar) override;

private:
  const unsigned int _v_var;
  const VariableSecond & _second_v;
  const VariablePhiSecond & _second_v_phi;
  const Real _lm_sign;
  const Real _diffusivity;
};
