//* This file is part of the MOOSE framework
//* https://www.mooseframework.org
//*
//* All rights reserved, see COPYRIGHT for full restrictions
//* https://github.com/idaholab/moose/blob/master/COPYRIGHT
//*
//* Licensed under LGPL 2.1, please see LICENSE for details
//* https://www.gnu.org/licenses/lgpl-2.1.html

#include "GrainTrackerElasticity_OR.h"
#include "EulerAngleProvider.h"
#include "RotationTensor.h"

registerMooseObject("PhaseFieldApp", GrainTrackerElasticity_OR);

InputParameters
GrainTrackerElasticity_OR::validParams()
{
  InputParameters params = GrainTracker::validParams();
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
  params.addParam<Real>("Euler_angles_OR_1", 0, "Euler angle in direction 1 for orientation relationship rotation");
  params.addParam<Real>("Euler_angles_OR_2", 0, "Euler angle in direction 2 for orientation relationship rotation");
  params.addParam<Real>("Euler_angles_OR_3", 0, "Euler angle in direction 3 for orientation relationship rotation");
  return params;
}

GrainTrackerElasticity_OR::GrainTrackerElasticity_OR(const InputParameters & parameters)
  : GrainDataTracker<RankFourTensor>(parameters),
    _random_rotations(getParam<bool>("random_rotations")),
    _C_ijkl(getParam<std::vector<Real>>("C_ijkl"),
            getParam<MooseEnum>("fill_method").getEnum<RankFourTensor::FillMethod>()),
    _euler(getUserObject<EulerAngleProvider>("euler_angle_provider")),
    _Euler_angles_OR_1(getParam<Real>("Euler_angles_OR_1")),
    _Euler_angles_OR_2(getParam<Real>("Euler_angles_OR_2")),
    _Euler_angles_OR_3(getParam<Real>("Euler_angles_OR_3"))
{
}

RankFourTensor
GrainTrackerElasticity_OR::newGrain(unsigned int new_grain_id)
{
  EulerAngles angles;

  if (new_grain_id < _euler.getGrainNum())
  {
      angles = _euler.getEulerAngles(new_grain_id);
  }
  else
  {
    if (_random_rotations)
    {
      angles.random();
    }
    else
      mooseError("GrainTrackerElasticity has run out of grain rotation data.");
  }

  EulerAngles angles_OR;
  angles_OR.phi1 = _Euler_angles_OR_1;
  angles_OR.Phi = _Euler_angles_OR_2;
  angles_OR.phi2 = _Euler_angles_OR_3;

  RankFourTensor C_ijkl = _C_ijkl;

  // Rotate for the orientation relationship
  C_ijkl.rotate(RotationTensor(RealVectorValue(angles_OR)));
  // Rotate for the grain texture
  C_ijkl.rotate(RotationTensor(RealVectorValue(angles))); // Only works becasue the grain texture rotation is around z, which is the last rotation for OR

  return C_ijkl;
}
