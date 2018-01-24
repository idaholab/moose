//* This file is part of the MOOSE framework
//* https://www.mooseframework.org
//*
//* All rights reserved, see COPYRIGHT for full restrictions
//* https://github.com/idaholab/moose/blob/master/COPYRIGHT
//*
//* Licensed under LGPL 2.1, please see LICENSE for details
//* https://www.gnu.org/licenses/lgpl-2.1.html

#ifndef MATERIALTENSORAUX_H
#define MATERIALTENSORAUX_H

#include "AuxKernel.h"
#include "MaterialTensorCalculator.h"

class MaterialTensorAux;
class SymmTensor;

template <>
InputParameters validParams<MaterialTensorAux>();

class MaterialTensorAux : public AuxKernel
{
public:
  MaterialTensorAux(const InputParameters & parameters);

  virtual ~MaterialTensorAux() {}

protected:
  virtual Real computeValue();

  MaterialTensorCalculator _material_tensor_calculator;
  const MaterialProperty<SymmTensor> & _tensor;

  const bool _has_qp_select;
  const unsigned int _qp_select;
};

#endif // MATERIALTENSORAUX_H
