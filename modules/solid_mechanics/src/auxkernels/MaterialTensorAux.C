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
  params.addParam<int>("qp_select", -1, "The quad point you want evaluated");
  return params;
}

MaterialTensorAux::MaterialTensorAux( const std::string & name, InputParameters parameters ) :
    AuxKernel( name, parameters ),
    _material_tensor_calculator( name, parameters),
    _tensor( getMaterialProperty<SymmTensor>( getParam<std::string>("tensor") ) ),
    _qp_select(parameters.get<int>("qp_select"))
{
}

Real
MaterialTensorAux::computeValue()
{
  RealVectorValue direction;
  int qp_call = _qp;
  if (_qp_select >= 0 && _qp_select < _q_point.size())
    qp_call = _qp_select;
  else
  {
    if (_qp_select != -1)
    {
      _console << "qp_select = " << _qp_select << std::endl;
      _console << "qp = " << _qp << std::endl;
      _console << "q_point.size() = " << _q_point.size() << std::endl;
      mooseError("The parameter qp_select is not valid");
    }
  }

  Real value = _material_tensor_calculator.getTensorQuantity(_tensor[qp_call],
                                                             &_q_point[qp_call],
                                                               direction);
  return value;
}
