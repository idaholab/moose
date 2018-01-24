//* This file is part of the MOOSE framework
//* https://www.mooseframework.org
//*
//* All rights reserved, see COPYRIGHT for full restrictions
//* https://github.com/idaholab/moose/blob/master/COPYRIGHT
//*
//* Licensed under LGPL 2.1, please see LICENSE for details
//* https://www.gnu.org/licenses/lgpl-2.1.html

#ifndef FUNCTIONDIRACSOURCE_H
#define FUNCTIONDIRACSOURCE_H

// Moose Includes
#include "DiracKernel.h"

// Forward Declarations
class FunctionDiracSource;
class Function;

template <>
InputParameters validParams<FunctionDiracSource>();

class FunctionDiracSource : public DiracKernel
{
public:
  FunctionDiracSource(const InputParameters & parameters);

  virtual void addPoints() override;

protected:
  virtual Real computeQpResidual() override;

  Function & _function;
  Point _p;
};

#endif // FunctionDiracSource_H
