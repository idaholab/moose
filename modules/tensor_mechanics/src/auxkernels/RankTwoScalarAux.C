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
  params.addRequiredParam<MaterialPropertyName>("rank_two_tensor", "The rank two material tensor name");
  MooseEnum scalar_options("VonMisesStress EquivalentPlasticStrain Hydrostatic L2norm MaxPrincipal MidPrincipal MinPrincipal VolumetricStrain FirstInvariant SecondInvariant ThirdInvariant");
  params.addParam<MooseEnum>("scalar_type", scalar_options, "Type of scalar output");

  return params;
}

RankTwoScalarAux::RankTwoScalarAux(const InputParameters & parameters) :
    AuxKernel(parameters),
    _tensor(getMaterialProperty<RankTwoTensor>("rank_two_tensor")),
    _scalar_type(getParam<MooseEnum>("scalar_type"))
{
}

Real
RankTwoScalarAux::computeValue()
{
  Real val;
  RankTwoTensor s;
  enum { x, y, z };

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
   case 4:
   case 5:
   case 6:
     val = calcEigenValues();
     break;
   case 7: // VolumetricStrain (Lagrangian)
     val = _tensor[_qp].trace() +
           _tensor[_qp](x,x)*_tensor[_qp](y,y) +
           _tensor[_qp](y,y)*_tensor[_qp](z,z) +
           _tensor[_qp](z,z)*_tensor[_qp](x,x) +
           _tensor[_qp](x,x)*_tensor[_qp](y,y)*_tensor[_qp](z,z);
     break;
   case 8: // FirstInvariant
     val = _tensor[_qp].trace();
     break;
   case 9: // SecondInvariant
     val = _tensor[_qp](x,x)*_tensor[_qp](y,y) +
           _tensor[_qp](y,y)*_tensor[_qp](z,z) +
           _tensor[_qp](z,z)*_tensor[_qp](x,x) -
           _tensor[_qp](x,y)*_tensor[_qp](y,x) -
           _tensor[_qp](y,z)*_tensor[_qp](z,y) -
           _tensor[_qp](x,z)*_tensor[_qp](z,x);
     break;
   case 10: // ThirdInvariant
     val = _tensor[_qp](x,x)*_tensor[_qp](y,y)*_tensor[_qp](z,z) -
           _tensor[_qp](x,x)*_tensor[_qp](z,y)*_tensor[_qp](y,z) +
           _tensor[_qp](x,y)*_tensor[_qp](z,x)*_tensor[_qp](y,z) -
           _tensor[_qp](x,y)*_tensor[_qp](y,x)*_tensor[_qp](z,z) +
           _tensor[_qp](x,z)*_tensor[_qp](y,x)*_tensor[_qp](z,y) -
           _tensor[_qp](x,z)*_tensor[_qp](z,x)*_tensor[_qp](y,y);
     break;
   default:
     mooseError("RankTwoScalarAux Error: Pass valid scalar type - VonMisesStress, EquivalentPlasticStrain, Hydrostatic, L2norm MaxPrincipal MidPrincipal MinPrincipal VolumetricStrain FirstInvariant SecondInvariant ThirdInvariant");
  }
  return val;
}

Real
RankTwoScalarAux::calcEigenValues()
{
  std::vector<Real> eigval(LIBMESH_DIM);
  Real val = 0.0;
  unsigned int max_index = 2;
  unsigned int mid_index = 1;
  unsigned int min_index = 0;

  if (LIBMESH_DIM == 2)
    max_index = 1;

  _tensor[_qp].symmetricEigenvalues(eigval);

  switch (_scalar_type)
  {
   case 4:
     val = eigval[max_index];
     break;
   case 5:
     if (LIBMESH_DIM == 2)
       mooseError("RankTwoScalarAux Error: No Mid Principal value when LIBMESH_DIM is 2");
     val = eigval[mid_index];
     break;
   case 6:
     val = eigval[min_index];
     break;
   default:
     mooseError("RankTwoScalarAux Error: Pass valid scalar type - VonMisesStress, EquivalentPlasticStrain, Hydrostatic, L2norm MaxPrincipal MidPrincipal MinPrincipal");
  }
  return val;
}
