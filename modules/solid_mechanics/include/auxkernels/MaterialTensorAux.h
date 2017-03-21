/****************************************************************/
/* MOOSE - Multiphysics Object Oriented Simulation Environment  */
/*                                                              */
/*          All contents are licensed under LGPL V2.1           */
/*             See LICENSE for full restrictions                */
/****************************************************************/

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
