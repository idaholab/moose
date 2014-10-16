/****************************************************************/
/*               DO NOT MODIFY THIS HEADER                      */
/* MOOSE - Multiphysics Object Oriented Simulation Environment  */
/*                                                              */
/*           (c) 2010 Battelle Energy Alliance, LLC             */
/*                   ALL RIGHTS RESERVED                        */
/*                                                              */
/*          Prepared by Battelle Energy Alliance, LLC           */
/*            Under Contract No. DE-AC07-05ID14517              */
/*            With the U. S. Department of Energy               */
/*                                                              */
/*            See COPYRIGHT for full restrictions               */
/****************************************************************/

#ifndef MATERIALTENSORAUX_H
#define MATERIALTENSORAUX_H

#include "AuxKernel.h"
#include "MaterialTensorCalculator.h"

class MaterialTensorAux;
class SymmTensor;

template<>
InputParameters validParams<MaterialTensorAux>();

class MaterialTensorAux : public AuxKernel
{
public:
  MaterialTensorAux( const std::string & name, InputParameters parameters );

  virtual ~MaterialTensorAux() {}

protected:
  virtual Real computeValue();

  MaterialTensorCalculator _material_tensor_calculator;
  MaterialProperty<SymmTensor> & _tensor;

  const bool _has_qp_select;
  const unsigned int _qp_select;
};

#endif // MATERIALTENSORAUX_H
