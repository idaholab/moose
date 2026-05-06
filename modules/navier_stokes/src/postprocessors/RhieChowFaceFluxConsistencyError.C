//* This file is part of the MOOSE framework
//* https://mooseframework.inl.gov
//*
//* All rights reserved, see COPYRIGHT for full restrictions
//* https://github.com/idaholab/moose/blob/master/COPYRIGHT
//*
//* Licensed under LGPL 2.1, please see LICENSE for details
//* https://www.gnu.org/licenses/lgpl-2.1.html

#include "RhieChowFaceFluxConsistencyError.h"

#include "RhieChowMassFlux.h"

registerMooseObject("NavierStokesApp", RhieChowFaceFluxConsistencyError);

InputParameters
RhieChowFaceFluxConsistencyError::validParams()
{
  InputParameters params = GeneralPostprocessor::validParams();
  MooseEnum quantity("l2 internal_l2 boundary_l2", "l2");
  params.addRequiredParam<UserObjectName>("rhie_chow_user_object",
                                          "The Rhie-Chow face-flux user object.");
  params.addRequiredParam<MooseEnum>(
      "quantity", quantity, "Which face-flux consistency norm to report.");
  params.addClassDescription(
      "Reports the current Rhie-Chow volumetric face-flux consistency audit, i.e. the mismatch "
      "between the stored face flux and the flux reconstructed from the current velocity field.");
  return params;
}

RhieChowFaceFluxConsistencyError::RhieChowFaceFluxConsistencyError(
    const InputParameters & parameters)
  : GeneralPostprocessor(parameters),
    _rhie_chow(getUserObject<RhieChowMassFlux>("rhie_chow_user_object")),
    _quantity(getParam<MooseEnum>("quantity") == "internal_l2"
                  ? Quantity::InternalL2
                  : getParam<MooseEnum>("quantity") == "boundary_l2" ? Quantity::BoundaryL2
                                                                     : Quantity::TotalL2)
{
}

void
RhieChowFaceFluxConsistencyError::initialize()
{
}

void
RhieChowFaceFluxConsistencyError::execute()
{
}

void
RhieChowFaceFluxConsistencyError::finalize()
{
}

Real
RhieChowFaceFluxConsistencyError::getValue() const
{
  const auto audit = _rhie_chow.faceFluxConsistencyAudit();

  switch (_quantity)
  {
    case Quantity::InternalL2:
      return audit.internal_l2_norm;
    case Quantity::BoundaryL2:
      return audit.boundary_l2_norm;
    case Quantity::TotalL2:
    default:
      return audit.l2_norm;
  }
}
