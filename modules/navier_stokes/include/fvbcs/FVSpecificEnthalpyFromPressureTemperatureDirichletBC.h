//* This file is part of the MOOSE framework
//* https://mooseframework.inl.gov
//*
//* All rights reserved, see COPYRIGHT for full restrictions
//* https://github.com/idaholab/moose/blob/master/COPYRIGHT
//*
//* Licensed under LGPL 2.1, please see LICENSE for details
//* https://www.gnu.org/licenses/lgpl-2.1.html

#pragma once

#include "FVDirichletBCBase.h"

class SinglePhaseFluidProperties;

/**
 * Computes the boundary value of the specific enthalpy from pressure and temperature variables
 */
class FVSpecificEnthalpyFromPressureTemperatureDirichletBC : public FVDirichletBCBase
{
public:
  FVSpecificEnthalpyFromPressureTemperatureDirichletBC(const InputParameters & parameters);

  static InputParameters validParams();

protected:
  ADReal boundaryValue(const FaceInfo & fi, const Moose::StateArg & state) const override;

  /// Single phase fluid property user object
  const SinglePhaseFluidProperties & _fp;
  /// The functor computing the pressure value for this BC
  const Moose::Functor<ADReal> & _pressure;
  /// The functor computing the temperature value for this BC
  const Moose::Functor<ADReal> & _temperature;
};
