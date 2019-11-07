//* This file is part of the MOOSE framework
//* https://www.mooseframework.org
//*
//* All rights reserved, see COPYRIGHT for full restrictions
//* https://github.com/idaholab/moose/blob/master/COPYRIGHT
//*
//* Licensed under LGPL 2.1, please see LICENSE for details
//* https://www.gnu.org/licenses/lgpl-2.1.html

#pragma once

#include "EigenKernel.h"

class MaterialEigenKernel : public EigenKernel
{
public:
  static InputParameters validParams();

  MaterialEigenKernel(const InputParameters & parameters);

protected:
  virtual Real computeQpResidual();

  std::string _propname;
  const MaterialProperty<Real> & _mat;
};
