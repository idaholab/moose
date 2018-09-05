//* This file is part of the MOOSE framework
//* https://www.mooseframework.org
//*
//* All rights reserved, see COPYRIGHT for full restrictions
//* https://github.com/idaholab/moose/blob/master/COPYRIGHT
//*
//* Licensed under LGPL 2.1, please see LICENSE for details
//* https://www.gnu.org/licenses/lgpl-2.1.html

#ifndef MATDIFFUSIONTEST_H
#define MATDIFFUSIONTEST_H

#include "Kernel.h"
#include "MaterialProperty.h"

// Forward Declaration
class MatDiffusionTest;

template <>
InputParameters validParams<MatDiffusionTest>();

class MatDiffusionTest : public Kernel
{
public:
  MatDiffusionTest(const InputParameters & parameters);

protected:
  virtual Real computeQpResidual();

  virtual Real computeQpJacobian();

  std::string _prop_name;
  const MaterialProperty<Real> * _diff;
};

#endif // MATDIFFUSIONTEST_H
