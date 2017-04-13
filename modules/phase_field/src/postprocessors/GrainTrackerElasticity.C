/****************************************************************/
/* MOOSE - Multiphysics Object Oriented Simulation Environment  */
/*                                                              */
/*          All contents are licensed under LGPL V2.1           */
/*             See LICENSE for full restrictions                */
/****************************************************************/

#include "GrainTrackerElasticity.h"
#include "EulerAngleProvider.h"
#include "RotationTensor.h"

template <>
InputParameters
validParams<GrainTrackerElasticity>()
{
  InputParameters params = validParams<GrainTracker>();
  params.addParam<bool>("random_rotations",
                        true,
                        "Generate random rotations when the Euler Angle "
                        "provider runs out of data (otherwise error "
                        "out)");
  params.addRequiredParam<std::vector<Real>>("C_ijkl", "Unrotated stiffness tensor");
  params.addParam<MooseEnum>(
      "fill_method", RankFourTensor::fillMethodEnum() = "symmetric9", "The fill method");
  params.addRequiredParam<UserObjectName>("euler_angle_provider",
                                          "Name of Euler angle provider user object");
  return params;
}

GrainTrackerElasticity::GrainTrackerElasticity(const InputParameters & parameters)
  : GrainDataTracker<RankFourTensor>(parameters),
    _random_rotations(getParam<bool>("random_rotations")),
    _C_ijkl(getParam<std::vector<Real>>("C_ijkl"),
            getParam<MooseEnum>("fill_method").getEnum<RankFourTensor::FillMethod>()),
    _euler(getUserObject<EulerAngleProvider>("euler_angle_provider"))
{
}

RankFourTensor
GrainTrackerElasticity::newGrain(unsigned int new_grain_id)
{
  EulerAngles angles;

  if (new_grain_id < _euler.getGrainNum())
    angles = _euler.getEulerAngles(new_grain_id);
  else
  {
    if (_random_rotations)
      angles.random();
    else
      mooseError("GrainTrackerElasticity has run out of grain rotation data.");
  }

  RankFourTensor C_ijkl = _C_ijkl;
  C_ijkl.rotate(RotationTensor(RealVectorValue(angles)));

  return C_ijkl;
}
