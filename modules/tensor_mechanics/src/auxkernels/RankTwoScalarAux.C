/****************************************************************/
/* MOOSE - Multiphysics Object Oriented Simulation Environment  */
/*                                                              */
/*          All contents are licensed under LGPL V2.1           */
/*             See LICENSE for full restrictions                */
/****************************************************************/
#include "RankTwoScalarAux.h"
#include "RankTwoScalarTools.h"

template<>
InputParameters validParams<RankTwoScalarAux>()
{
  InputParameters params = validParams<AuxKernel>();
  params.addClassDescription("Compute a scalar property of a RankTwoTensor");
  params.addRequiredParam<MaterialPropertyName>("rank_two_tensor", "The rank two material tensor name");
  MooseEnum scalar_options("VonMisesStress EquivalentPlasticStrain Hydrostatic L2norm MaxPrincipal MidPrincipal MinPrincipal VolumetricStrain FirstInvariant SecondInvariant ThirdInvariant AxialStress HoopStress RadialStress TriaxialityStress ComponentDirection Direction");
  params.addParam<MooseEnum>("scalar_type", scalar_options, "Type of scalar output");
  params.addParam<unsigned int>("selected_qp", "Evaluate the tensor at this quadpoint.  This option only needs to be used if you are interested in a particular quadpoint in each element: otherwise do not include this parameter in your input file");
  params.addParamNamesToGroup("selected_qp", "Advanced");

  params.addParam<Point>("point1", Point(), "Start point for axis used to calculate some cylinderical material tensor quantities" );
  params.addParam<Point>("point2", Point(0, 1, 0), "End point for axis used to calculate some material tensor quantities");
  params.addParam<Point>("direction", Point(0, 0, 1), "Direction vector");

  return params;
}

RankTwoScalarAux::RankTwoScalarAux(const InputParameters & parameters) :
    AuxKernel(parameters),
    _tensor(getMaterialProperty<RankTwoTensor>("rank_two_tensor")),
    _scalar_type(getParam<MooseEnum>("scalar_type")),
    _has_selected_qp(isParamValid("selected_qp")),
    _selected_qp(_has_selected_qp ? getParam<unsigned int>("selected_qp") : 0),
    _point1(parameters.get<Point>("point1")),
    _point2(parameters.get<Point>("point2")),
    _input_direction(parameters.get<Point>("direction")/parameters.get<Point>("direction").norm())
{
}

Real
RankTwoScalarAux::computeValue()
{
  Real value = 0.0;
  RealVectorValue direction;
  direction.zero();

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
      value = RankTwoScalarTools::vonMisesStress(_tensor[qp]);
      break;
    case 1:
      ///For plastic strain tensor (ep), tr(ep) = 0 is considered
      value = RankTwoScalarTools::equivalentPlasticStrain(_tensor[qp]);
      break;
    case 2:
      value = RankTwoScalarTools::hydrostatic(_tensor[qp]);
      break;
    case 3:
      value = RankTwoScalarTools::L2norm(_tensor[qp]);
      break;
    case 4:
      value = RankTwoScalarTools::maxPrinciple(_tensor[qp]);
      break;
    case 5:
      value = RankTwoScalarTools::midPrinciple(_tensor[qp]);
      break;
    case 6:
      value = RankTwoScalarTools::minPrinciple(_tensor[qp]);
      break;
    case 7:
      value = RankTwoScalarTools::volumetricStrain(_tensor[qp]);
      break;
    case 8:
      value = RankTwoScalarTools::firstInvariant(_tensor[qp]);
      break;
    case 9:
      value = RankTwoScalarTools::secondInvariant(_tensor[qp]);
      break;
    case 10:
      value = RankTwoScalarTools::thirdInvariant(_tensor[qp]);
      break;
    case 11:
      value = RankTwoScalarTools::axialStress(_tensor[qp], _point1, _point2, direction);
      break;
    case 12:
      value = RankTwoScalarTools::hoopStress(_tensor[qp], _point1, _point2, & _q_point[_qp], direction);
      break;
    case 13:
      value = RankTwoScalarTools::radialStress(_tensor[qp], _point1, _point2, & _q_point[_qp], direction);
      break;
    case 14:
      value = RankTwoScalarTools::triaxialityStress(_tensor[qp]);
      break;
    case 15:
      value = RankTwoScalarTools::directionValueTensor(_tensor[qp], _input_direction);
   default:
     mooseError("RankTwoScalarAux Error: Pass valid scalar type - VonMisesStress, EquivalentPlasticStrain, Hydrostatic, L2norm, MaxPrincipal, MidPrincipal, MinPrincipal, VolumetricStrain, FirstInvariant, SecondInvariant, ThirdInvariant, AxialStress, HoopStress, RadialStress, TriaxialityStress, ComponentDirection, or Direction");
  }
  return value;
}
