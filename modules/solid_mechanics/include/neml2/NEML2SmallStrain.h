//* This file is part of the MOOSE framework
//* https://mooseframework.inl.gov
//*
//* All rights reserved, see COPYRIGHT for full restrictions
//* https://github.com/idaholab/moose/blob/master/COPYRIGHT
//*
//* Licensed under LGPL 2.1, please see LICENSE for details
//* https://www.gnu.org/licenses/lgpl-2.1.html

#pragma once

#ifdef NEML2_ENABLED

// MOOSE includes
#include "NEML2PreKernel.h"

class NEML2SmallStrain : public NEML2PreKernel
{
public:
  static InputParameters validParams();

  NEML2SmallStrain(const InputParameters & parameters);

protected:
  /// Calculate small strain from displacement gradients
  void forward() override;

  /// Displacement gradients
  ///@{
  const neml2::Tensor * _grad_disp_x;
  const neml2::Tensor * _grad_disp_y;
  const neml2::Tensor * _grad_disp_z;
  ///@}
};

#endif
