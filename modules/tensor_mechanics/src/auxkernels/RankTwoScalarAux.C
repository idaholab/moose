/****************************************************************/
/* MOOSE - Multiphysics Object Oriented Simulation Environment  */
/*                                                              */
/*          All contents are licensed under LGPL V2.1           */
/*             See LICENSE for full restrictions                */
/****************************************************************/

#include "RankTwoScalarAux.h"
#include "RankTwoScalarTools.h"

template <>
InputParameters
validParams<RankTwoScalarAux>()
{
  InputParameters params = validParams<AuxKernel>();
  params.addClassDescription("Compute a scalar property of a RankTwoTensor");
  params.addRequiredParam<MaterialPropertyName>("rank_two_tensor",
                                                "The rank two material tensor name");
  params.addParam<MooseEnum>(
      "scalar_type", RankTwoScalarTools::scalarOptions(), "Type of scalar output");
  params.addParam<unsigned int>(
      "selected_qp",
      "Evaluate the tensor at this quadpoint.  This option only needs to be used if "
      "you are interested in a particular quadpoint in each element: otherwise do "
      "not include this parameter in your input file");
  params.addParamNamesToGroup("selected_qp", "Advanced");

  params.addParam<Point>(
      "point1",
      Point(0, 0, 0),
      "Start point for axis used to calculate some cylinderical material tensor quantities");
  params.addParam<Point>("point2",
                         Point(0, 1, 0),
                         "End point for axis used to calculate some material tensor quantities");
  params.addParam<Point>("direction", Point(0, 0, 1), "Direction vector");
  return params;
}

RankTwoScalarAux::RankTwoScalarAux(const InputParameters & parameters)
  : AuxKernel(parameters),
    _tensor(getMaterialProperty<RankTwoTensor>("rank_two_tensor")),
    _scalar_type(getParam<MooseEnum>("scalar_type")),
    _has_selected_qp(isParamValid("selected_qp")),
    _selected_qp(_has_selected_qp ? getParam<unsigned int>("selected_qp") : 0),
    _point1(parameters.get<Point>("point1")),
    _point2(parameters.get<Point>("point2")),
    _input_direction(parameters.get<Point>("direction") / parameters.get<Point>("direction").norm())
{
}

Real
RankTwoScalarAux::computeValue()
{
  unsigned int qp = _qp;
  if (_has_selected_qp)
  {
    if (_selected_qp >= _q_point.size())
    {
      Threads::spin_mutex::scoped_lock lock(Threads::spin_mtx);
      mooseError("RankTwoScalarAux.  selected_qp specified as ",
                 _selected_qp,
                 " but there are only ",
                 _q_point.size(),
                 " quadpoints in the element");
    }
    qp = _selected_qp;
  }

  return RankTwoScalarTools::getQuantity(
      _tensor[qp], _scalar_type, _point1, _point2, _q_point[qp], _input_direction);
}
