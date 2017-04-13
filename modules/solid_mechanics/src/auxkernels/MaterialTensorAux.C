/****************************************************************/
/* MOOSE - Multiphysics Object Oriented Simulation Environment  */
/*                                                              */
/*          All contents are licensed under LGPL V2.1           */
/*             See LICENSE for full restrictions                */
/****************************************************************/

#include "MaterialTensorAux.h"
#include "SymmTensor.h"

template <>
InputParameters
validParams<MaterialTensorAux>()
{
  InputParameters params = validParams<AuxKernel>();
  params += validParams<MaterialTensorCalculator>();
  params.addRequiredParam<MaterialPropertyName>("tensor", "The material tensor name.");
  params.addParam<unsigned int>("qp_select", "The quad point you want evaluated");
  return params;
}

MaterialTensorAux::MaterialTensorAux(const InputParameters & parameters)
  : AuxKernel(parameters),
    _material_tensor_calculator(parameters),
    _tensor(getMaterialProperty<SymmTensor>("tensor")),
    _has_qp_select(isParamValid("qp_select")),
    _qp_select(_has_qp_select ? getParam<unsigned int>("qp_select") : 0)
{
}

Real
MaterialTensorAux::computeValue()
{
  RealVectorValue direction;
  unsigned int qp_call;

  if (_has_qp_select)
  {
    if (_qp_select < _q_point.size())
      qp_call = _qp_select;
    else
    {
      Moose::err << "qp_select = " << _qp_select << std::endl;
      Moose::err << "qp = " << _qp << std::endl;
      Moose::err << "q_point.size() = " << _q_point.size() << std::endl;
      mooseError("The parameter qp_select is not valid");
    }
  }
  else
    qp_call = _qp;

  Real value =
      _material_tensor_calculator.getTensorQuantity(_tensor[qp_call], _q_point[qp_call], direction);
  return value;
}
