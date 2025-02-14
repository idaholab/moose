//* This file is part of the MOOSE framework
//* https://www.mooseframework.org
//*
//* All rights reserved, see COPYRIGHT for full restrictions
//* https://github.com/idaholab/moose/blob/master/COPYRIGHT
//*
//* Licensed under LGPL 2.1, please see LICENSE for details
//* https://www.gnu.org/licenses/lgpl-2.1.html

#pragma once

#include "GenericKernelCurl.h"

/**
 *  Weak form contribution corresponding to the curl(curl(E)) where E is the
 *  electric field vector
 */
 template <bool is_ad>
 class CurlCurlFieldTempl : public GenericKernelCurl<is_ad>
{
public:
  static InputParameters validParams();

  CurlCurlFieldTempl(const InputParameters & parameters);

protected:
  virtual GenericReal<is_ad> computeQpResidual() override;
  virtual Real computeQpJacobian() override;

  /// Scalar coefficient
  Real _coeff;

  usingGenericKernelCurlMembers;
};

typedef CurlCurlFieldTempl<false> CurlCurlField;
typedef CurlCurlFieldTempl<true> ADCurlCurlField;
