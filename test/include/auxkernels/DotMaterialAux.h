//* This file is part of the MOOSE framework
//* https://www.mooseframework.org
//*
//* All rights reserved, see COPYRIGHT for full restrictions
//* https://github.com/idaholab/moose/blob/master/COPYRIGHT
//*
//* Licensed under LGPL 2.1, please see LICENSE for details
//* https://www.gnu.org/licenses/lgpl-2.1.html

#ifndef DOTMATERIALAUX_H
#define DOTMATERIALAUX_H

// MOOSE includes
#include "AuxKernel.h"

// Forward declarations
class DotMaterialAux;

template <>
InputParameters validParams<DotMaterialAux>();

/**
 * Test class for testing functionality of getMaterialPropertyDot for AuxKernels
 */
class DotMaterialAux : public AuxKernel
{
public:
  DotMaterialAux(const InputParameters & parameters);

protected:
  virtual Real computeValue();

private:
  const MaterialProperty<Real> & _dot;
};

#endif // DOTMATERIALAUX_H
