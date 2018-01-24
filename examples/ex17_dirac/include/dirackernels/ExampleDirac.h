//* This file is part of the MOOSE framework
//* https://www.mooseframework.org
//*
//* All rights reserved, see COPYRIGHT for full restrictions
//* https://github.com/idaholab/moose/blob/master/COPYRIGHT
//*
//* Licensed under LGPL 2.1, please see LICENSE for details
//* https://www.gnu.org/licenses/lgpl-2.1.html

#ifndef EXAMPLEDIRAC_H
#define EXAMPLEDIRAC_H

// Moose Includes
#include "DiracKernel.h"

// Forward Declarations
class ExampleDirac;

template <>
InputParameters validParams<ExampleDirac>();

class ExampleDirac : public DiracKernel
{
public:
  ExampleDirac(const InputParameters & parameters);

  virtual void addPoints() override;
  virtual Real computeQpResidual() override;

protected:
  Real _value;
  Point _point;
};

#endif // EXAMPLEDIRAC_H
