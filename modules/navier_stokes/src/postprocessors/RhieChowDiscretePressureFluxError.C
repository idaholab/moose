//* This file is part of the MOOSE framework
//* https://mooseframework.inl.gov
//*
//* All rights reserved, see COPYRIGHT for full restrictions
//* https://github.com/idaholab/moose/blob/master/COPYRIGHT
//*
//* Licensed under LGPL 2.1, please see LICENSE for details
//* https://www.gnu.org/licenses/lgpl-2.1.html

#include "RhieChowDiscretePressureFluxError.h"

#include "FaceInfo.h"
#include "Function.h"
#include "MooseMesh.h"
#include "RhieChowMassFlux.h"

registerMooseObject("NavierStokesApp", RhieChowDiscretePressureFluxError);

InputParameters
RhieChowDiscretePressureFluxError::validParams()
{
  InputParameters params = GeneralPostprocessor::validParams();
  MooseEnum metric("l2 max_abs", "l2");
  params.addRequiredParam<UserObjectName>("rhie_chow_user_object",
                                          "The Rhie-Chow face-flux user object.");
  params.addRequiredParam<FunctionName>("exact_pressure",
                                        "The exact pressure function used to build the MMS.");
  params.addParam<MooseEnum>("metric",
                             metric,
                             "Whether to compute a face-measure-weighted L2 norm or a max "
                             "absolute mismatch.");
  params.addClassDescription("Computes the internal-face mismatch between the stored "
                             "pressure-equation face flux and the same discrete diffusion "
                             "operator applied to the exact pressure field.");
  return params;
}

RhieChowDiscretePressureFluxError::RhieChowDiscretePressureFluxError(
    const InputParameters & parameters)
  : GeneralPostprocessor(parameters),
    _mesh(_subproblem.mesh()),
    _rhie_chow(getUserObject<RhieChowMassFlux>("rhie_chow_user_object")),
    _exact_pressure(getFunction("exact_pressure")),
    _metric(getParam<MooseEnum>("metric") == "max_abs" ? Metric::MaxAbs : Metric::L2),
    _value(0.0)
{
}

void
RhieChowDiscretePressureFluxError::initialize()
{
  _value = 0.0;
}

void
RhieChowDiscretePressureFluxError::execute()
{
  for (const auto * fi : _mesh.faceInfo())
  {
    if (!fi || !fi->elemPtr() || !fi->neighborPtr())
      continue;

    const Real stored_flux = _rhie_chow.storedPressureEquationFlux(*fi);
    const Real exact_flux = _rhie_chow.exactInternalPressureEquationFlux(*fi, _exact_pressure);
    const Real diff = stored_flux - exact_flux;

    if (_metric == Metric::MaxAbs)
      _value = std::max(_value, std::abs(diff));
    else
      _value += diff * diff * fi->faceArea() * fi->faceCoord();
  }
}

void
RhieChowDiscretePressureFluxError::finalize()
{
  if (_metric == Metric::MaxAbs)
    _communicator.max(_value);
  else
  {
    _communicator.sum(_value);
    _value = std::sqrt(_value);
  }
}

Real
RhieChowDiscretePressureFluxError::getValue() const
{
  return _value;
}
