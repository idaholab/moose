//* This file is part of the MOOSE framework
//* https://mooseframework.inl.gov
//*
//* All rights reserved, see COPYRIGHT for full restrictions
//* https://github.com/idaholab/moose/blob/master/COPYRIGHT
//*
//* Licensed under LGPL 2.1, please see LICENSE for details
//* https://www.gnu.org/licenses/lgpl-2.1.html

#include "RhieChowCellContinuityResidual.h"

#include "MooseMesh.h"
#include "RhieChowMassFlux.h"

#include "libmesh/elem.h"

#include <unordered_map>

registerMooseObject("NavierStokesApp", RhieChowCellContinuityResidual);

InputParameters
RhieChowCellContinuityResidual::validParams()
{
  InputParameters params = GeneralPostprocessor::validParams();
  MooseEnum metric("l2 max_abs", "l2");
  params.addRequiredParam<UserObjectName>("rhie_chow_user_object",
                                          "The Rhie-Chow face-flux user object.");
  params.addParam<MooseEnum>("metric",
                             metric,
                             "Whether to compute a cell-volume-weighted L2 norm or a max "
                             "absolute cell divergence from the stored face mass fluxes.");
  params.addClassDescription("Computes the discrete cell continuity residual implied by the "
                             "current Rhie-Chow face mass fluxes.");
  return params;
}

RhieChowCellContinuityResidual::RhieChowCellContinuityResidual(const InputParameters & parameters)
  : GeneralPostprocessor(parameters),
    _mesh(_subproblem.mesh()),
    _rhie_chow(getUserObject<RhieChowMassFlux>("rhie_chow_user_object")),
    _metric(getParam<MooseEnum>("metric") == "max_abs" ? Metric::MaxAbs : Metric::L2),
    _value(0.0)
{
}

void
RhieChowCellContinuityResidual::initialize()
{
  _value = 0.0;
}

void
RhieChowCellContinuityResidual::execute()
{
  std::unordered_map<dof_id_type, Real> cell_mass_imbalance;

  for (const auto * fi : _rhie_chow.flowFacesForAudit())
  {
    const Real flux = _rhie_chow.getMassFlux(*fi) * fi->faceArea() * fi->faceCoord();

    if (const auto * elem = fi->elemPtr())
      cell_mass_imbalance[elem->id()] += flux;

    if (const auto * neighbor = fi->neighborPtr())
      cell_mass_imbalance[neighbor->id()] -= flux;
  }

  for (const auto & [elem_id, imbalance] : cell_mass_imbalance)
  {
    const auto & elem_info = _mesh.elemInfo(elem_id);
    const Real volume = elem_info.volume();
    const Real divergence = volume > 0.0 ? imbalance / volume : 0.0;

    if (_metric == Metric::MaxAbs)
      _value = std::max(_value, std::abs(divergence));
    else
      _value += divergence * divergence * volume;
  }
}

void
RhieChowCellContinuityResidual::finalize()
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
RhieChowCellContinuityResidual::getValue() const
{
  return _value;
}
