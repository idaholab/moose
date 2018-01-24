//* This file is part of the MOOSE framework
//* https://www.mooseframework.org
//*
//* All rights reserved, see COPYRIGHT for full restrictions
//* https://github.com/idaholab/moose/blob/master/COPYRIGHT
//*
//* Licensed under LGPL 2.1, please see LICENSE for details
//* https://www.gnu.org/licenses/lgpl-2.1.html

#ifndef MATERIALEIGENKERNEL_H
#define MATERIALEIGENKERNEL_H

#include "EigenKernel.h"

// Forward Declarations
class MaterialEigenKernel;

template <>
InputParameters validParams<MaterialEigenKernel>();

class MaterialEigenKernel : public EigenKernel
{
public:
  MaterialEigenKernel(const InputParameters & parameters);

protected:
  virtual Real computeQpResidual();

  std::string _propname;
  const MaterialProperty<Real> & _mat;
};

#endif // MATERIALEIGENKERNEL_H
