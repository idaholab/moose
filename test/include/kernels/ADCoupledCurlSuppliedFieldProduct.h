//* This file is part of the MOOSE framework
//* https://mooseframework.inl.gov
//*
//* All rights reserved, see COPYRIGHT for full restrictions
//* https://github.com/idaholab/moose/blob/master/COPYRIGHT
//*
//* Licensed under LGPL 2.1, please see LICENSE for details
//* https://www.gnu.org/licenses/lgpl-2.1.html

#pragma once

#include "ADKernel.h"

/**
 *  Supplies the product of the curl of a coupled vector variable
 *  and a user supplied field
 */
class ADCoupledCurlSuppliedFieldProduct : public ADKernel
{
public:
  static InputParameters validParams();

  ADCoupledCurlSuppliedFieldProduct(const InputParameters & parameters);

protected:
  virtual ADReal computeQpResidual() override;

private:
  /// Curl of a coupled vector variable
  const ADVectorVariableCurl & _coupled_curl;
  /// x component function
  const Function & _function_x;
  /// y component function
  const Function & _function_y;
  /// z component function
  const Function & _function_z;
};
