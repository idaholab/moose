//* This file is part of the MOOSE framework
//* https://www.mooseframework.org
//*
//* All rights reserved, see COPYRIGHT for full restrictions
//* https://github.com/idaholab/moose/blob/master/COPYRIGHT
//*
//* Licensed under LGPL 2.1, please see LICENSE for details
//* https://www.gnu.org/licenses/lgpl-2.1.html

#include "TensorMechanicsPlasticDruckerPrager.h"
#include "RankFourTensor.h"
#include "libmesh/utility.h"

registerMooseObject("TensorMechanicsApp", TensorMechanicsPlasticDruckerPrager);

InputParameters
TensorMechanicsPlasticDruckerPrager::validParams()
{
  InputParameters params = TensorMechanicsPlasticModel::validParams();
  MooseEnum mc_interpolation_scheme("outer_tip=0 inner_tip=1 lode_zero=2 inner_edge=3 native=4",
                                    "lode_zero");
  params.addParam<MooseEnum>(
      "mc_interpolation_scheme",
      mc_interpolation_scheme,
      "Scheme by which the Drucker-Prager cohesion, friction angle and dilation angle are set from "
      "the Mohr-Coulomb parameters mc_cohesion, mc_friction_angle and mc_dilation_angle.  Consider "
      "the DP and MC yield surfaces on the deviatoric (octahedral) plane.  Outer_tip: the DP "
      "circle touches the outer tips of the MC hex.  Inner_tip: the DP circle touches the inner "
      "tips of the MC hex.  Lode_zero: the DP circle intersects the MC hex at lode angle=0.  "
      "Inner_edge: the DP circle is the largest circle that wholly fits inside the MC hex.  "
      "Native: The DP cohesion, friction angle and dilation angle are set equal to the mc_ "
      "parameters entered.");
  params.addRequiredParam<UserObjectName>(
      "mc_cohesion",
      "A TensorMechanicsHardening UserObject that defines hardening of the "
      "Mohr-Coulomb cohesion.  Physically this should not be negative.");
  params.addRequiredParam<UserObjectName>(
      "mc_friction_angle",
      "A TensorMechanicsHardening UserObject that defines hardening of the "
      "Mohr-Coulomb friction angle (in radians).  Physically this should be "
      "between 0 and Pi/2.");
  params.addRequiredParam<UserObjectName>(
      "mc_dilation_angle",
      "A TensorMechanicsHardening UserObject that defines hardening of the "
      "Mohr-Coulomb dilation angle (in radians).  Usually the dilation angle "
      "is not greater than the friction angle, and it is between 0 and Pi/2.");
  params.addClassDescription(
      "Non-associative Drucker Prager plasticity with no smoothing of the cone tip.");
  return params;
}

TensorMechanicsPlasticDruckerPrager::TensorMechanicsPlasticDruckerPrager(
    const InputParameters & parameters)
  : TensorMechanicsPlasticModel(parameters),
    _mc_cohesion(getUserObject<TensorMechanicsHardeningModel>("mc_cohesion")),
    _mc_phi(getUserObject<TensorMechanicsHardeningModel>("mc_friction_angle")),
    _mc_psi(getUserObject<TensorMechanicsHardeningModel>("mc_dilation_angle")),
    _mc_interpolation_scheme(getParam<MooseEnum>("mc_interpolation_scheme")),
    _zero_cohesion_hardening(_mc_cohesion.modelName().compare("Constant") == 0),
    _zero_phi_hardening(_mc_phi.modelName().compare("Constant") == 0),
    _zero_psi_hardening(_mc_psi.modelName().compare("Constant") == 0)
{
  if (_mc_phi.value(0.0) < 0.0 || _mc_psi.value(0.0) < 0.0 ||
      _mc_phi.value(0.0) > libMesh::pi / 2.0 || _mc_psi.value(0.0) > libMesh::pi / 2.0)
    mooseError("TensorMechanicsPlasticDruckerPrager: MC friction and dilation angles must lie in "
               "[0, Pi/2]");
  if (_mc_phi.value(0) < _mc_psi.value(0.0))
    mooseError("TensorMechanicsPlasticDruckerPrager: MC friction angle must not be less than MC "
               "dilation angle");
  if (_mc_cohesion.value(0.0) < 0)
    mooseError("TensorMechanicsPlasticDruckerPrager: MC cohesion should not be negative");

  initializeAandB(0.0, _aaa, _bbb);
  initializeB(0.0, dilation, _bbb_flow);
}

Real
TensorMechanicsPlasticDruckerPrager::yieldFunction(const RankTwoTensor & stress, Real intnl) const
{
  Real aaa;
  Real bbb;
  bothAB(intnl, aaa, bbb);
  return std::sqrt(stress.secondInvariant()) + stress.trace() * bbb - aaa;
}

RankTwoTensor
TensorMechanicsPlasticDruckerPrager::df_dsig(const RankTwoTensor & stress, Real bbb) const
{
  return 0.5 * stress.dsecondInvariant() / std::sqrt(stress.secondInvariant()) +
         stress.dtrace() * bbb;
}

RankTwoTensor
TensorMechanicsPlasticDruckerPrager::dyieldFunction_dstress(const RankTwoTensor & stress,
                                                            Real intnl) const
{
  Real bbb;
  onlyB(intnl, friction, bbb);
  return df_dsig(stress, bbb);
}

Real
TensorMechanicsPlasticDruckerPrager::dyieldFunction_dintnl(const RankTwoTensor & stress,
                                                           Real intnl) const
{
  Real daaa;
  Real dbbb;
  dbothAB(intnl, daaa, dbbb);
  return stress.trace() * dbbb - daaa;
}

RankTwoTensor
TensorMechanicsPlasticDruckerPrager::flowPotential(const RankTwoTensor & stress, Real intnl) const
{
  Real bbb_flow;
  onlyB(intnl, dilation, bbb_flow);
  return df_dsig(stress, bbb_flow);
}

RankFourTensor
TensorMechanicsPlasticDruckerPrager::dflowPotential_dstress(const RankTwoTensor & stress,
                                                            Real /*intnl*/) const
{
  RankFourTensor dr_dstress;
  dr_dstress = 0.5 * stress.d2secondInvariant() / std::sqrt(stress.secondInvariant());
  dr_dstress += -0.5 * 0.5 * stress.dsecondInvariant().outerProduct(stress.dsecondInvariant()) /
                std::pow(stress.secondInvariant(), 1.5);
  return dr_dstress;
}

RankTwoTensor
TensorMechanicsPlasticDruckerPrager::dflowPotential_dintnl(const RankTwoTensor & stress,
                                                           Real intnl) const
{
  Real dbbb;
  donlyB(intnl, dilation, dbbb);
  return stress.dtrace() * dbbb;
}

std::string
TensorMechanicsPlasticDruckerPrager::modelName() const
{
  return "DruckerPrager";
}

void
TensorMechanicsPlasticDruckerPrager::bothAB(Real intnl, Real & aaa, Real & bbb) const
{
  if (_zero_cohesion_hardening && _zero_phi_hardening)
  {
    aaa = _aaa;
    bbb = _bbb;
    return;
  }
  initializeAandB(intnl, aaa, bbb);
}

void
TensorMechanicsPlasticDruckerPrager::onlyB(Real intnl, int fd, Real & bbb) const
{
  if (_zero_phi_hardening && (fd == friction))
  {
    bbb = _bbb;
    return;
  }
  if (_zero_psi_hardening && (fd == dilation))
  {
    bbb = _bbb_flow;
    return;
  }
  initializeB(intnl, fd, bbb);
}

void
TensorMechanicsPlasticDruckerPrager::donlyB(Real intnl, int fd, Real & dbbb) const
{
  if (_zero_phi_hardening && (fd == friction))
  {
    dbbb = 0;
    return;
  }
  if (_zero_psi_hardening && (fd == dilation))
  {
    dbbb = 0;
    return;
  }
  const Real s = (fd == friction) ? std::sin(_mc_phi.value(intnl)) : std::sin(_mc_psi.value(intnl));
  const Real ds = (fd == friction) ? std::cos(_mc_phi.value(intnl)) * _mc_phi.derivative(intnl)
                                   : std::cos(_mc_psi.value(intnl)) * _mc_psi.derivative(intnl);
  switch (_mc_interpolation_scheme)
  {
    case 0: // outer_tip
      dbbb = 2.0 / std::sqrt(3.0) * (ds / (3.0 - s) + s * ds / Utility::pow<2>(3.0 - s));
      break;
    case 1: // inner_tip
      dbbb = 2.0 / std::sqrt(3.0) * (ds / (3.0 + s) - s * ds / Utility::pow<2>(3.0 + s));
      break;
    case 2: // lode_zero
      dbbb = ds / 3.0;
      break;
    case 3: // inner_edge
      dbbb = ds / std::sqrt(9.0 + 3.0 * Utility::pow<2>(s)) -
             3 * s * s * ds / std::pow(9.0 + 3.0 * Utility::pow<2>(s), 1.5);
      break;
    case 4: // native
      const Real c =
          (fd == friction) ? std::cos(_mc_phi.value(intnl)) : std::cos(_mc_psi.value(intnl));
      const Real dc = (fd == friction)
                          ? -std::sin(_mc_phi.value(intnl)) * _mc_phi.derivative(intnl)
                          : -std::sin(_mc_psi.value(intnl)) * _mc_psi.derivative(intnl);
      dbbb = ds / c - s * dc / Utility::pow<2>(c);
      break;
  }
}

void
TensorMechanicsPlasticDruckerPrager::dbothAB(Real intnl, Real & daaa, Real & dbbb) const
{
  if (_zero_cohesion_hardening && _zero_phi_hardening)
  {
    daaa = 0;
    dbbb = 0;
    return;
  }

  const Real C = _mc_cohesion.value(intnl);
  const Real dC = _mc_cohesion.derivative(intnl);
  const Real cosphi = std::cos(_mc_phi.value(intnl));
  const Real dcosphi = -std::sin(_mc_phi.value(intnl)) * _mc_phi.derivative(intnl);
  const Real sinphi = std::sin(_mc_phi.value(intnl));
  const Real dsinphi = std::cos(_mc_phi.value(intnl)) * _mc_phi.derivative(intnl);
  switch (_mc_interpolation_scheme)
  {
    case 0: // outer_tip
      daaa = 2.0 * std::sqrt(3.0) *
             (dC * cosphi / (3.0 - sinphi) + C * dcosphi / (3.0 - sinphi) +
              C * cosphi * dsinphi / Utility::pow<2>(3.0 - sinphi));
      dbbb = 2.0 / std::sqrt(3.0) *
             (dsinphi / (3.0 - sinphi) + sinphi * dsinphi / Utility::pow<2>(3.0 - sinphi));
      break;
    case 1: // inner_tip
      daaa = 2.0 * std::sqrt(3.0) *
             (dC * cosphi / (3.0 + sinphi) + C * dcosphi / (3.0 + sinphi) -
              C * cosphi * dsinphi / Utility::pow<2>(3.0 + sinphi));
      dbbb = 2.0 / std::sqrt(3.0) *
             (dsinphi / (3.0 + sinphi) - sinphi * dsinphi / Utility::pow<2>(3.0 + sinphi));
      break;
    case 2: // lode_zero
      daaa = dC * cosphi + C * dcosphi;
      dbbb = dsinphi / 3.0;
      break;
    case 3: // inner_edge
      daaa = 3.0 * dC * cosphi / std::sqrt(9.0 + 3.0 * Utility::pow<2>(sinphi)) +
             3.0 * C * dcosphi / std::sqrt(9.0 + 3.0 * Utility::pow<2>(sinphi)) -
             3.0 * C * cosphi * 3.0 * sinphi * dsinphi /
                 std::pow(9.0 + 3.0 * Utility::pow<2>(sinphi), 1.5);
      dbbb = dsinphi / std::sqrt(9.0 + 3.0 * Utility::pow<2>(sinphi)) -
             3.0 * sinphi * sinphi * dsinphi / std::pow(9.0 + 3.0 * Utility::pow<2>(sinphi), 1.5);
      break;
    case 4: // native
      daaa = dC;
      dbbb = dsinphi / cosphi - sinphi * dcosphi / Utility::pow<2>(cosphi);
      break;
  }
}

void
TensorMechanicsPlasticDruckerPrager::initializeAandB(Real intnl, Real & aaa, Real & bbb) const
{
  const Real C = _mc_cohesion.value(intnl);
  const Real cosphi = std::cos(_mc_phi.value(intnl));
  const Real sinphi = std::sin(_mc_phi.value(intnl));
  switch (_mc_interpolation_scheme)
  {
    case 0: // outer_tip
      aaa = 2.0 * std::sqrt(3.0) * C * cosphi / (3.0 - sinphi);
      bbb = 2.0 * sinphi / std::sqrt(3.0) / (3.0 - sinphi);
      break;
    case 1: // inner_tip
      aaa = 2.0 * std::sqrt(3.0) * C * cosphi / (3.0 + sinphi);
      bbb = 2.0 * sinphi / std::sqrt(3.0) / (3.0 + sinphi);
      break;
    case 2: // lode_zero
      aaa = C * cosphi;
      bbb = sinphi / 3.0;
      break;
    case 3: // inner_edge
      aaa = 3.0 * C * cosphi / std::sqrt(9.0 + 3.0 * Utility::pow<2>(sinphi));
      bbb = sinphi / std::sqrt(9.0 + 3.0 * Utility::pow<2>(sinphi));
      break;
    case 4: // native
      aaa = C;
      bbb = sinphi / cosphi;
      break;
  }
}

void
TensorMechanicsPlasticDruckerPrager::initializeB(Real intnl, int fd, Real & bbb) const
{
  const Real s = (fd == friction) ? std::sin(_mc_phi.value(intnl)) : std::sin(_mc_psi.value(intnl));
  switch (_mc_interpolation_scheme)
  {
    case 0: // outer_tip
      bbb = 2.0 * s / std::sqrt(3.0) / (3.0 - s);
      break;
    case 1: // inner_tip
      bbb = 2.0 * s / std::sqrt(3.0) / (3.0 + s);
      break;
    case 2: // lode_zero
      bbb = s / 3.0;
      break;
    case 3: // inner_edge
      bbb = s / std::sqrt(9.0 + 3.0 * Utility::pow<2>(s));
      break;
    case 4: // native
      const Real c =
          (fd == friction) ? std::cos(_mc_phi.value(intnl)) : std::cos(_mc_psi.value(intnl));
      bbb = s / c;
      break;
  }
}
