/****************************************************************/
/* MOOSE - Multiphysics Object Oriented Simulation Environment  */
/*                                                              */
/*          All contents are licensed under LGPL V2.1           */
/*             See LICENSE for full restrictions                */
/****************************************************************/
#include "RankTwoScalarAux.h"

template<>
InputParameters validParams<RankTwoScalarAux>()
{
  InputParameters params = validParams<AuxKernel>();
  params.addClassDescription("Compute a scalar property of a RankTwoTensor");
  params.addRequiredParam<std::string>("rank_two_tensor", "The rank two material tensor name");
  MooseEnum scalar_options("VonMisesStress EquivalentPlasticStrain Hydrostatic L2norm");
  params.addParam<MooseEnum>("scalar_type", scalar_options, "Type of scalar output");

  return params;
}

RankTwoScalarAux::RankTwoScalarAux(const std::string & name, InputParameters parameters) :
  AuxKernel(name, parameters),
  _tensor(getMaterialProperty<RankTwoTensor>(getParam<std::string>("rank_two_tensor"))),
  _scalar_type(getParam<MooseEnum>("scalar_type"))
{
}

Real
RankTwoScalarAux::computeValue()
{
  Real val;
  RankTwoTensor s;

  switch (_scalar_type)
  {
   case 0:
     s = _tensor[_qp].deviatoric();//Calculates deviatoric tensor
     val = std::pow(3.0/2.0 * s.doubleContraction(s), 0.5);//Calculates sqrt(3/2*s:s)
     break;
   case 1:
     ///For plastic strain tensor (ep), tr(ep) = 0 is considered
     val = std::pow(2.0/3.0 * _tensor[_qp].doubleContraction(_tensor[_qp]), 0.5);//Calculates sqrt(2/3*ep:ep)
     break;
   case 2:
     val = _tensor[_qp].trace()/3.0;
     break;
   case 3:
     val = _tensor[_qp].L2norm();
     break;
   default:
     mooseError("RankTwoScalarAux Error: Pass valid scalar type - VonMisesStress, EquivalentPlasticStrain, Hydrostatic, L2norm");
  }
  return val;
}
