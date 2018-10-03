//* This file is part of the MOOSE framework
//* https://www.mooseframework.org
//*
//* All rights reserved, see COPYRIGHT for full restrictions
//* https://github.com/idaholab/moose/blob/master/COPYRIGHT
//*
//* Licensed under LGPL 2.1, please see LICENSE for details
//* https://www.gnu.org/licenses/lgpl-2.1.html

#ifndef NANINTERFACETESTKERNEL_H
#define NANINTERFACETESTKERNEL_H

#include "Kernel.h"

class NaNInterfaceTestKernel;
class NaNInterfaceTestFluidProperties;

template <>
InputParameters validParams<NaNInterfaceTestKernel>();

/**
 * Kernel to test NaNInterface using NaNInterfaceTestFluidProperties
 */
class NaNInterfaceTestKernel : public Kernel
{
public:
  NaNInterfaceTestKernel(const InputParameters & parameters);

protected:
  virtual Real computeQpResidual() override;

  /// Test fluid properties
  const NaNInterfaceTestFluidProperties & _nan_interface_test_fp;
};

#endif // NANINTERFACETESTKERNEL_H
