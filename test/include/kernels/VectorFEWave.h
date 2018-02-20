//* This file is part of the MOOSE framework
//* https://www.mooseframework.org
//*
//* All rights reserved, see COPYRIGHT for full restrictions
//* https://github.com/idaholab/moose/blob/master/COPYRIGHT
//*
//* Licensed under LGPL 2.1, please see LICENSE for details
//* https://www.gnu.org/licenses/lgpl-2.1.html

#ifndef VECTORFEWAVE_H
#define VECTORFEWAVE_H

#include "VectorKernel.h"
#include "MaterialProperty.h"

// Forward Declaration
class VectorFEWave;

template <>
InputParameters validParams<VectorFEWave>();

class VectorFEWave : public VectorKernel
{
public:
  VectorFEWave(const InputParameters & parameters);

protected:
  virtual Real computeQpResidual() override;
  virtual Real computeQpJacobian() override;

  Function & _x_ffn;
  Function & _y_ffn;
  Function & _z_ffn;
};

#endif // VECTORFEWAVE_H
