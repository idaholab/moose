#include "LayeredAverageRZ.h"

registerMooseObject("MooseApp", LayeredAverageRZ);

template <>
InputParameters
validParams<LayeredAverageRZ>()
{
  InputParameters params = validParams<LayeredAverage>();
  params += validParams<RZSymmetry>();
  params.addRequiredParam<Real>("length",
                                "The length of the block in the direction given by 'axis_dir'.");
  return params;
}

LayeredAverageRZ::LayeredAverageRZ(const InputParameters & parameters)
  : LayeredAverage(parameters), RZSymmetry(parameters)
{
  _direction_min = getParam<Point>("axis_point")(_direction);
  _direction_max = _direction_min + getParam<Real>("length");
}

void
LayeredAverageRZ::execute()
{
  LayeredIntegral::execute();

  Real current_elem_volume = 0.;
  for (unsigned int qp = 0; qp < _qrule->n_points(); qp++)
  {
    const Real circumference = computeCircumference(_q_point[qp]);
    current_elem_volume += _JxW[qp] * circumference;
  }

  unsigned int layer = getLayer(_current_elem->centroid());
  _layer_volumes[layer] += current_elem_volume;
}

Real
LayeredAverageRZ::computeIntegral()
{
  Real sum = 0;

  for (_qp = 0; _qp < _qrule->n_points(); _qp++)
  {
    const Real circumference = computeCircumference(_q_point[_qp]);
    sum += _JxW[_qp] * circumference * computeQpIntegral();
  }
  return sum;
}
