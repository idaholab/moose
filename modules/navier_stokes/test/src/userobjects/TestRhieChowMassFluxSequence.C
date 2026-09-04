//* This file is part of the MOOSE framework
//* https://mooseframework.inl.gov
//*
//* All rights reserved, see COPYRIGHT for full restrictions
//* https://github.com/idaholab/moose/blob/master/COPYRIGHT
//*
//* Licensed under LGPL 2.1, please see LICENSE for details

#include "TestRhieChowMassFluxSequence.h"
#include "RhieChowMassFlux.h"

registerMooseObject("NavierStokesTestApp", TestRhieChowMassFluxSequence);

InputParameters
TestRhieChowMassFluxSequence::validParams()
{
  InputParameters params = GeneralUserObject::validParams();
  params.addRequiredParam<UserObjectName>("rhie_chow_user_object", "The Rhie-Chow user object.");
  params.addRequiredParam<MooseEnum>(
      "operation", MooseEnum("update_candidate reuse_face_flux"), "The invalid sequence to test.");
  return params;
}

TestRhieChowMassFluxSequence::TestRhieChowMassFluxSequence(const InputParameters & parameters)
  : GeneralUserObject(parameters),
    _rhie_chow(
        const_cast<RhieChowMassFlux &>(getUserObject<RhieChowMassFlux>("rhie_chow_user_object"))),
    _operation(getParam<MooseEnum>("operation"))
{
}

void
TestRhieChowMassFluxSequence::execute()
{
  if (_operation == "reuse_face_flux")
    _rhie_chow.preparePISOCorrector();

  _rhie_chow.preparePressureRelaxation();
}
