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
  params.addParam<unsigned int>("selected_qp", "Evaluate the tensor at this quadpoint.  This only needs to be used if you are interested in a particular quadpoint in each element: otherwise do not include this parameter in your input file");
  params.addParamNamesToGroup("selected_qp", "Advanced");
  return params;
}

RankTwoScalarAux::RankTwoScalarAux(const InputParameters & parameters) :
    AuxKernel(parameters),
    _tensor(getMaterialProperty<RankTwoTensor>("rank_two_tensor")),
    _scalar_type(getParam<MooseEnum>("scalar_type")),
    _has_selected_qp(isParamValid("selected_qp")),
    _selected_qp(_has_selected_qp ? getParam<unsigned int>("selected_qp") : 0)
{
}

Real
RankTwoScalarAux::computeValue()
{
  Real val;
  RankTwoTensor s;
  enum { x, y, z };

  unsigned int qp = _qp;
  if (_has_selected_qp)
  {
    if (_selected_qp >= _q_point.size())
      mooseError("RankTwoScalarAux.  selected_qp specified as " << _selected_qp << " but there are only " << _q_point.size() << " quadpoints in the element");
    qp = _selected_qp;
  }

  switch (_scalar_type)
  {
   case 0:
     s = _tensor[qp].deviatoric();//Calculates deviatoric tensor
     val = std::pow(3.0/2.0 * s.doubleContraction(s), 0.5);//Calculates sqrt(3/2*s:s)
     break;
   case 1:
     ///For plastic strain tensor (ep), tr(ep) = 0 is considered
     val = std::pow(2.0/3.0 * _tensor[qp].doubleContraction(_tensor[qp]), 0.5);//Calculates sqrt(2/3*ep:ep)
     break;
   case 2:
     val = _tensor[qp].trace()/3.0;
     break;
   case 3:
     val = _tensor[qp].L2norm();
     break;
   case 4:
   case 5:
   case 6:
     val = calcEigenValues(qp);
     break;
   case 7: // VolumetricStrain (Lagrangian)
     val = _tensor[qp].trace() +
           _tensor[qp](x,x)*_tensor[qp](y,y) +
           _tensor[qp](y,y)*_tensor[qp](z,z) +
           _tensor[qp](z,z)*_tensor[qp](x,x) +
           _tensor[qp](x,x)*_tensor[qp](y,y)*_tensor[qp](z,z);
     break;
   case 8: // FirstInvariant
     val = _tensor[qp].trace();
     break;
   case 9: // SecondInvariant
     val = _tensor[qp](x,x)*_tensor[qp](y,y) +
           _tensor[qp](y,y)*_tensor[qp](z,z) +
           _tensor[qp](z,z)*_tensor[qp](x,x) -
           _tensor[qp](x,y)*_tensor[qp](y,x) -
           _tensor[qp](y,z)*_tensor[qp](z,y) -
           _tensor[qp](x,z)*_tensor[qp](z,x);
     break;
   case 10: // ThirdInvariant
     val = _tensor[qp](x,x)*_tensor[qp](y,y)*_tensor[qp](z,z) -
           _tensor[qp](x,x)*_tensor[qp](z,y)*_tensor[qp](y,z) +
           _tensor[qp](x,y)*_tensor[qp](z,x)*_tensor[qp](y,z) -
           _tensor[qp](x,y)*_tensor[qp](y,x)*_tensor[qp](z,z) +
           _tensor[qp](x,z)*_tensor[qp](y,x)*_tensor[qp](z,y) -
           _tensor[qp](x,z)*_tensor[qp](z,x)*_tensor[qp](y,y);
     break;
   default:
     mooseError("RankTwoScalarAux Error: Pass valid scalar type - VonMisesStress, EquivalentPlasticStrain, Hydrostatic, L2norm MaxPrincipal MidPrincipal MinPrincipal VolumetricStrain FirstInvariant SecondInvariant ThirdInvariant");
  }
  return val;
}

Real
RankTwoScalarAux::calcEigenValues(unsigned int qp)
{
  std::vector<Real> eigval(LIBMESH_DIM);
  Real val = 0.0;
  unsigned int max_index = 2;
  unsigned int mid_index = 1;
  unsigned int min_index = 0;

  if (LIBMESH_DIM == 2)
    max_index = 1;

  _tensor[qp].symmetricEigenvalues(eigval);


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
