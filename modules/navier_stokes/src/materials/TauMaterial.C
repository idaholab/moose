//* This file is part of the MOOSE framework
//* https://www.mooseframework.org
//*
//* All rights reserved, see COPYRIGHT for full restrictions
//* https://github.com/idaholab/moose/blob/master/COPYRIGHT
//*
//* Licensed under LGPL 2.1, please see LICENSE for details
//* https://www.gnu.org/licenses/lgpl-2.1.html

#include "TauMaterial.h"
#include "NS.h"
#include "GeometryFunctions.h"
#include "MooseMesh.h"
#include "InputErrorChecking.h"

namespace nms = NS;

defineADValidParams(
    TauMaterial,
    ADMaterial,
    params.addClassDescription("Material providing generic methods required for "
                               "evaluating the stabilization term for SUPG schemes");

    MooseEnum TauMethodEnum("switching minimum", "switching");
    MooseEnum AdvectiveLimitEnum("incompressible compressible combined", "incompressible");

    params.addParam<MooseEnum>(
        "method",
        TauMethodEnum,
        "How to combine the intrinsic time scale ('switching' or 'minimum')");

    params.addParam<MooseEnum>("temporal_limit",
                               getOnOffEnum(),
                               "How to form the temporal limit ('none' or 'standard')");

    params.addParam<MooseEnum>(
        "advective_limit",
        AdvectiveLimitEnum,
        "How to form the advective limit ('incompressible', 'compressible', or 'combined'");

    params.addParam<MooseEnum>("diffusive_limit",
                               getOnOffEnum(),
                               "How to form the diffusive limit ('none' or 'standard')");

    // if using the compressible advection limit, we need the speed of sound provided by
    // a fluid properties object
    params.addParam<UserObjectName>(nms::fluid,
                                    "The name of the fluid properties "
                                    "object to use");

    params.addParam<RealVectorValue>(
        "multipliers",
        RealVectorValue(1.0, 1.0, 1.0),
        "Multipliers to apply to the temporal, advective, and diffusive limits");

    params.addParam<bool>("viscous_stress", false, "Whether the viscous stress term is included "
        "in the momentum equation.");

    params.addRangeCheckedParam<Real>(
        "factor", 1.0, "factor >= 0.0", "Scaling factor to be applied"););

TauMaterial::TauMaterial(const InputParameters & parameters)
  : ADMaterial(parameters),
    _N(_mesh.dimension() + 2),
    _factor(getParam<Real>("factor")),
    _viscous_stress(getParam<bool>("viscous_stress")),
    _rho(getADMaterialProperty<Real>(nms::density)),
    _speed(getADMaterialProperty<Real>(nms::speed)),
    _velocity(getADMaterialProperty<RealVectorValue>(nms::velocity)),

    _method(getParam<MooseEnum>("method").getEnum<TauMethodEnum>()),
    _temporal_limit(getParam<MooseEnum>("temporal_limit").getEnum<settings::OnOffEnum>()),
    _advective_limit(getParam<MooseEnum>("advective_limit").getEnum<AdvectiveLimitEnum>()),
    _diffusive_limit(getParam<MooseEnum>("diffusive_limit").getEnum<settings::OnOffEnum>()),
    _multipliers(getParam<RealVectorValue>("multipliers")),

    // only needed if computing a compressible advection limit
    _fluid(isParamValid(nms::fluid) ?
               &getUserObject<SinglePhaseFluidProperties>(nms::fluid)
               : nullptr),
    _v(_advective_limit == combined || _advective_limit == compressible
           ? &getADMaterialProperty<Real>(nms::v)
           : nullptr),
    _e(_advective_limit == combined || _advective_limit == compressible
           ? &getADMaterialProperty<Real>(nms::e)
           : nullptr),

    // these material properties are only needed if the diffusive component exists
    _cp(_diffusive_limit == settings::standard ? &getADMaterialProperty<Real>(nms::cp)
                                               : nullptr),
    _kappa(_diffusive_limit == settings::standard ? &getADMaterialProperty<Real>(nms::kappa)
                                              : nullptr),
    _mu_eff((_diffusive_limit == settings::standard) && _viscous_stress ?
      &getADMaterialProperty<Real>(nms::mu_eff) : nullptr),

    // class variables to be set later
    _mass_tau(0.0),
    _momentum_tau(0.0),
    _energy_tau(0.0)
{
  if (_advective_limit == incompressible)
    checkUnusedInputParameter(parameters, nms::fluid, "With an incompressible advective limit");
  else
    if (!_fluid)
      paramError(nms::fluid, "With a compressible or combined advective limit, this user object is required");

  // we use the opposite convention of the theory manual - here, 'tau' actually
  // represents 1 / tau in the theory manual notation. The multiplier correspond
  // to multipliers on the forms shown in the theory manual, so we'll actually
  // apply their inverse - check that they're all nonzero. We use the opposite
  // convention because we don't need to check in computeTauComponent() for
  // all of the 'limits' to be nonzero.
  for (unsigned int i = 0; i < 3; ++i)
    if (_multipliers(i) < NS_DEFAULT_VALUES::epsilon)
      mooseError("Entry provided for 'multipliers' is smaller than " +
                 std::to_string(NS_DEFAULT_VALUES::epsilon) + " in 'TauMaterial'!");
}

void
TauMaterial::computeQpProperties()
{
  // tau is never used on boundaries because the SUPG kernels are not integrated by parts,
  // so return if on a boundary
  if (_bnd)
    return;

  // compute element size
  _h = elementSize(_current_elem);

  ADReal temporal = computeTransientLimit();
  ADReal incompressible = computeIncompressibleLimit();
  ADReal compressible = computeCompressibleLimit();
  ADReal diffusive_energy = computeEnergyDiffusiveLimit();
  ADReal diffusive_momentum = _viscous_stress ? computeMomentumDiffusiveLimit() : 0.0;

  std::vector<ADReal> inviscid_limits = {temporal, incompressible, compressible};
  std::vector<ADReal> energy_limits = {temporal, incompressible, compressible, diffusive_energy};
  std::vector<ADReal> momentum_limits = {temporal, incompressible, compressible, diffusive_momentum};

  // mass equation
  _mass_tau = computeTauComponent(inviscid_limits);

  // momentum equation(s)
  _momentum_tau = computeTauComponent(momentum_limits);

  // energy equation
  _energy_tau = computeTauComponent(energy_limits);

  // assemble tau as some combination of the mass, momentum, and energy tau's
  computeTau();
}

ADReal
TauMaterial::computeIncompressibleLimit() const
{
  switch (_advective_limit)
  {
    case incompressible:
    case combined:
      return (1.0 / _multipliers(1)) * 2.0 * _speed[_qp] / _h;
    case compressible:
      return 0.0;
    default:
      mooseError("Unhandled 'AdvectiveLimitEnum' in 'TauMaterial'!");
  }
}

ADReal
TauMaterial::computeCompressibleLimit() const
{
  switch (_advective_limit)
  {
    case compressible:
    case combined:
    {
      ADReal c = _fluid->c_from_v_e((*_v)[_qp], (*_e)[_qp]);
      return (1.0 / _multipliers(1)) * 2.0 * (_speed[_qp] + c) / _h;
    }
    case incompressible:
      return 0.0;
    default:
      mooseError("Unhandled 'AdvectiveLimitEnum' in 'TauMaterial'!");
  }
}

ADReal
TauMaterial::computeTransientLimit() const
{
  switch (_temporal_limit)
  {
    case settings::standard:
    {
      if (_fe_problem.isTransient())
        return (1.0 / _multipliers(0)) * 2.0 / _fe_problem.dt();
      else
        return 0.0;
    }
    case settings::none:
      return 0.0;
    default:
      mooseError("Unhandled 'LimitEnum' in 'TauMaterial'!");
  }
}

ADReal
TauMaterial::computeMomentumDiffusiveLimit() const
{
  switch (_diffusive_limit)
  {
    case settings::standard:
    {
      ADReal diffusivity = (*_mu_eff)[_qp] / _rho[_qp];
      return (1.0 / _multipliers(2)) * 4.0 * diffusivity / (_h * _h);
    }
    case settings::none:
      return 0.0;
    default:
      mooseError("Unhandled 'LimitEnum' in 'TauMaterial'!");
  }
}

ADReal
TauMaterial::computeEnergyDiffusiveLimit() const
{
  switch (_diffusive_limit)
  {
    case settings::standard:
    {
      ADReal diffusivity =
          (*_kappa)[_qp] / std::max(_rho[_qp] * (*_cp)[_qp], NS_DEFAULT_VALUES::epsilon);
      return (1.0 / _multipliers(2)) * 4.0 * diffusivity / (_h * _h);
    }
    case settings::none:
      return 0.0;
    default:
      mooseError("Unhandled 'LimitEnum' in 'TauMaterial'!");
  }
}

ADReal
TauMaterial::computeTauComponent(const std::vector<ADReal> limits) const
{
  switch (_method)
  {
    case switching:
    {
      ADReal term = 0.0;
      for (const auto & l : limits)
        term += l * l;

      // TODO: figure out how to AD-ify this expression
      return term > NS_DEFAULT_VALUES::epsilon ? _factor / std::sqrt(term) : 0.0;
    }
    case minimum:
      return _factor * *std::max_element(limits.begin(), limits.end());
    default:
      mooseError("Unhandled MooseEnum 'TauMethodEnum'!");
  }
}
