#include "SigmoidTrendAuxBase.h"
#include "KLE_uo.h"

InputParameters
SigmoidTrendAuxBase::validParams()
{
  InputParameters params = AuxKernel::validParams();
  params.addRequiredParam<UserObjectName>("kl_user_object", "KLExpansionUserObject to sample");
  params.addRequiredParam<Point>("start_point", "Starting point of the centerline segment");
  params.addRequiredParam<Point>("end_point", "Ending point of the centerline segment");
  params.addRequiredParam<Real>("scale_max", "Trend value at distance = 0 (on the centerline)");
  params.addRequiredParam<Real>("scale_min", "Trend value far from the centerline");
  params.addRequiredParam<Real>("midpoint_of_sigmoid", "Distance at the sigmoid's midpoint");
  params.addParam<Real>("slope_at_midpoint", 1.0, "Steepness of the transition");
  return params;
}

SigmoidTrendAuxBase::SigmoidTrendAuxBase(const InputParameters & parameters)
  : AuxKernel(parameters),
    _kl_uo(getUserObject<KLExpansionUserObject>("kl_user_object")),
    _x1(getParam<Point>("start_point")),
    _x2(getParam<Point>("end_point")),
    _scale_hi(getParam<Real>("scale_max")),
    _scale_lo(getParam<Real>("scale_min")),
    _midpoint_of_sigmoid(getParam<Real>("midpoint_of_sigmoid")),
    _slope_at_midpoint(getParam<Real>("slope_at_midpoint"))
{
}

Real
SigmoidTrendAuxBase::distanceToSegment(const Point & p) const
{
  const Point x1x2 = _x2 - _x1;
  const Point x1x0 = p - _x1;
  const Point x2x0 = p - _x2;

  if (x1x2 * x1x0 <= 0)
    return x1x0.norm();
  else if (x1x2 * x2x0 >= 0)
    return x2x0.norm();
  else
    return (x1x0.cross(x2x0)).norm() / x1x2.norm();
}

Real
SigmoidTrendAuxBase::trendValue(Real d) const
{
  const Real z = _slope_at_midpoint * (d - _midpoint_of_sigmoid);

  Real sigmoid_scale;
  // this is written to prevent overflow

  if (z >= 0.0)
  {
    const Real e = std::exp(-z);
    sigmoid_scale = 1.0 / (1.0 + e);
  }
  else
  {
    const Real e = std::exp(z);
    sigmoid_scale = e / (1.0 + e);
  }

  return _scale_lo + (_scale_hi - _scale_lo) * (1.0 - sigmoid_scale);
}
