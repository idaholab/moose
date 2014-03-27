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

#include "MaterialTensorAux.h"
#include "SymmTensor.h"

template<>
InputParameters validParams<MaterialTensorAux>()
{
  InputParameters params = validParams<AuxKernel>();
  params += validParams<MaterialTensorCalculator>();
  params.addRequiredParam<std::string>("tensor", "The material tensor name.");
  return params;
}

MaterialTensorAux::MaterialTensorAux( const std::string & name, InputParameters parameters ) :
    AuxKernel( name, parameters ),
    _material_tensor_calculator( name, parameters),
    _tensor( getMaterialProperty<SymmTensor>( getParam<std::string>("tensor") ) )
{
}

Real
MaterialTensorAux::computeValue()
{
  RealVectorValue direction;
  Real value = _material_tensor_calculator.getTensorQuantity(_tensor[_qp],
                                                             &_q_point[_qp],
                                                             direction);
  return value;
}
