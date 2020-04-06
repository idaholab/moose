//* This file is part of the MOOSE framework
//* https://www.mooseframework.org
//*
//* All rights reserved, see COPYRIGHT for full restrictions
//* https://github.com/idaholab/moose/blob/master/COPYRIGHT
//*
//* Licensed under LGPL 2.1, please see LICENSE for details
//* https://www.gnu.org/licenses/lgpl-2.1.html

#pragma once

#include "AuxKernel.h"
#include "MaterialTensorCalculator.h"

class SymmTensor;

class MaterialTensorAux : public AuxKernel
{
public:
  static InputParameters validParams();

  MaterialTensorAux(const InputParameters & parameters);

  virtual ~MaterialTensorAux() {}

protected:
  virtual Real computeValue();

  MaterialTensorCalculator _material_tensor_calculator;
  const MaterialProperty<SymmTensor> & _tensor;

  const bool _has_qp_select;
  const unsigned int _qp_select;
};
