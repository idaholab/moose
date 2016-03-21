/****************************************************************/
/* MOOSE - Multiphysics Object Oriented Simulation Environment  */
/*                                                              */
/*          All contents are licensed under LGPL V2.1           */
/*             See LICENSE for full restrictions                */
/****************************************************************/
#include "TensorMechanicsPlasticDruckerPrager.h"

template<>
InputParameters validParams<TensorMechanicsPlasticDruckerPrager>()
{
  InputParameters params = validParams<TensorMechanicsPlasticModel>();
  MooseEnum mc_interpolation_scheme("outer_tip=0 inner_tip=1 lode_zero=2 inner_edge=3 native=4", "lode_zero");
  params.addParam<MooseEnum>("mc_interpolation_scheme", mc_interpolation_scheme, "Scheme by which the Drucker-Prager cohesion, friction angle and dilation angle are set from the Mohr-Coulomb parameters mc_cohesion, mc_friction_angle and mc_dilation_angle.  Consider the DP and MC yield surfaces on the devatoric (octahedral) plane.  Outer_tip: the DP circle touches the outer tips of the MC hex.  Inner_tip: the DP circle touches the inner tips of the MC hex.  Lode_zero: the DP circle intersects the MC hex at lode angle=0.  Inner_edge: the DP circle is the largest circle that wholey fits inside the MC hex.  Native: The DP cohesion, friction angle and dilation angle are set equal to the mc_ parameters entered.");
  params.addRequiredParam<UserObjectName>("mc_cohesion", "A TensorMechanicsHardening UserObject that defines hardening of the Mohr-Coulomb cohesion.  Physically this should not be negative.");
  params.addRequiredParam<UserObjectName>("mc_friction_angle", "A TensorMechanicsHardening UserObject that defines hardening of the Mohr-Coulomb friction angle (in radians).  Physically this should be between 0 and Pi/2.");
  params.addRequiredParam<UserObjectName>("mc_dilation_angle", "A TensorMechanicsHardening UserObject that defines hardening of the Mohr-Coulomb dilation angle (in radians).  Usually the dilation angle is not greater than the friction angle, and it is between 0 and Pi/2.");
  params.addClassDescription("Non-associative Drucker Prager plasticity with no smoothing of the cone tip.");
  return params;
}

TensorMechanicsPlasticDruckerPrager::TensorMechanicsPlasticDruckerPrager(const InputParameters & parameters) :
    TensorMechanicsPlasticModel(parameters),
    _mc_cohesion(getUserObject<TensorMechanicsHardeningModel>("mc_cohesion")),
    _mc_phi(getUserObject<TensorMechanicsHardeningModel>("mc_friction_angle")),
    _mc_psi(getUserObject<TensorMechanicsHardeningModel>("mc_dilation_angle")),
    _mc_interpolation_scheme(getParam<MooseEnum>("mc_interpolation_scheme")),
    _zero_cohesion_hardening(_mc_cohesion.modelName().compare("Constant") == 0),
    _zero_phi_hardening(_mc_phi.modelName().compare("Constant") == 0),
    _zero_psi_hardening(_mc_psi.modelName().compare("Constant") == 0)
{
  if (_mc_phi.value(0) < 0 || _mc_psi.value(0) < 0)
    mooseError("TensorMechanicsPlasticDruckerPrager: MC friction and dilation angles must lie in [0, Pi/2]");
  if (_mc_phi.value(0) < _mc_psi.value(0))
    mooseError("TensorMechanicsPlasticDruckerPrager: MC friction angle must not be less than MC dilation angle");
  if (_mc_cohesion.value(0) < 0)
    mooseError("TensorMechanicsPlasticDruckerPrager: MC cohesion should not be negative");

  initializeAandB(0.0, _aaa, _bbb);
  initializeB(0.0, false, _bbb_flow);
}


Real
TensorMechanicsPlasticDruckerPrager::yieldFunction(const RankTwoTensor & stress, Real intnl) const
{
  Real aaa;
  Real bbb;
  AandB(intnl, aaa, bbb);
  return std::sqrt(stress.secondInvariant()) + stress.trace()*bbb - aaa;
}

RankTwoTensor
TensorMechanicsPlasticDruckerPrager::df_dsig(const RankTwoTensor & stress, Real bbb) const
{
  return 0.5*stress.dsecondInvariant()/std::sqrt(stress.secondInvariant()) + stress.dtrace()*bbb;
}

RankTwoTensor
TensorMechanicsPlasticDruckerPrager::dyieldFunction_dstress(const RankTwoTensor & stress, Real intnl) const
{
  Real bbb;
  Bonly(intnl, true, bbb);
  return df_dsig(stress, bbb);
}


Real
TensorMechanicsPlasticDruckerPrager::dyieldFunction_dintnl(const RankTwoTensor & stress, Real intnl) const
{
  Real daaa;
  Real dbbb;
  dAandB(intnl, daaa, dbbb);
  return stress.trace()*dbbb - daaa;
}

RankTwoTensor
TensorMechanicsPlasticDruckerPrager::flowPotential(const RankTwoTensor & stress, Real intnl) const
{
  Real bbb_flow;
  Bonly(intnl, false, bbb_flow);
  return df_dsig(stress, bbb_flow);
}

RankFourTensor
TensorMechanicsPlasticDruckerPrager::dflowPotential_dstress(const RankTwoTensor & stress, Real /*intnl*/) const
{
  RankFourTensor dr_dstress;
  dr_dstress = 0.5*stress.d2secondInvariant()/std::sqrt(stress.secondInvariant());
  dr_dstress += -0.5*0.5*stress.dsecondInvariant().outerProduct(stress.dsecondInvariant())/std::pow(stress.secondInvariant(), 1.5);
  return dr_dstress;
}

RankTwoTensor
TensorMechanicsPlasticDruckerPrager::dflowPotential_dintnl(const RankTwoTensor & stress, Real intnl) const
{
  Real dbbb;
  dBonly(intnl, false, dbbb);
  return stress.dtrace()*dbbb;
}


std::string
TensorMechanicsPlasticDruckerPrager::modelName() const
{
  return "DruckerPrager";
}


void
TensorMechanicsPlasticDruckerPrager::AandB(Real intnl, Real & aaa, Real & bbb) const
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
TensorMechanicsPlasticDruckerPrager::Bonly(Real intnl, bool friction, Real & bbb) const
{
  if (_zero_phi_hardening && friction)
  {
    bbb = _bbb;
    return;
  }
  if (_zero_psi_hardening && !friction)
  {
    bbb = _bbb_flow;
    return;
  }
  initializeB(intnl, friction, bbb);
}


void
TensorMechanicsPlasticDruckerPrager::dBonly(Real intnl, bool friction, Real & dbbb) const
{
  if (_zero_phi_hardening && friction)
  {
    dbbb = 0;
    return;
  }
  if (_zero_psi_hardening && !friction)
  {
    dbbb = 0;
    return;
  }
  const Real sin = (friction) ? std::sin(_mc_phi.value(intnl)) : std::sin(_mc_psi.value(intnl));
  const Real dsin = (friction) ? std::cos(_mc_phi.value(intnl))*_mc_phi.derivative(intnl) : std::cos(_mc_psi.value(intnl))*_mc_psi.derivative(intnl);
  switch (_mc_interpolation_scheme)
  {
  case 0: // outer_tip
    dbbb = 2.0/std::sqrt(3)*(dsin/(3 - sin) + sin*dsin/std::pow(3 - sin, 2));
    break;
  case 1: // inner_tip
    dbbb = 2.0/std::sqrt(3)*(dsin/(3 + sin) - sin*dsin/std::pow(3 + sin, 2));
    break;
  case 2: // lode_zero
    dbbb = dsin/3.0;
    break;
  case 3: // inner_edge
    dbbb = dsin/std::sqrt(9 + 3*std::pow(sin, 2)) - 3*sin*sin*dsin/std::pow(9 + 3*std::pow(sin, 2), 1.5);
    break;
  case 4: // native
    const Real cos = (friction) ? std::cos(_mc_phi.value(intnl)) : std::cos(_mc_psi.value(intnl));
    const Real dcos = (friction) ? -std::sin(_mc_phi.value(intnl))*_mc_phi.derivative(intnl) : -std::sin(_mc_psi.value(intnl))*_mc_psi.derivative(intnl);
    dbbb = dsin/cos - sin*dcos/std::pow(cos, 2);
    break;
  }
}


void
TensorMechanicsPlasticDruckerPrager::dAandB(Real intnl, Real & daaa, Real & dbbb) const
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
  const Real dcosphi = -std::sin(_mc_phi.value(intnl))*_mc_phi.derivative(intnl);
  const Real sinphi = std::sin(_mc_phi.value(intnl));
  const Real dsinphi = std::cos(_mc_phi.value(intnl))*_mc_phi.derivative(intnl);
  switch (_mc_interpolation_scheme)
  {
  case 0: // outer_tip
    daaa = 2.0*std::sqrt(3)*(dC*cosphi/(3 - sinphi) + C*dcosphi/(3 - sinphi) + C*cosphi*dsinphi/std::pow(3 - sinphi, 2));
    dbbb = 2.0/std::sqrt(3)*(dsinphi/(3 - sinphi) + sinphi*dsinphi/std::pow(3 - sinphi, 2));
    break;
  case 1: // inner_tip
    daaa = 2.0*std::sqrt(3)*(dC*cosphi/(3 + sinphi) + C*dcosphi/(3 + sinphi) - C*cosphi*dsinphi/std::pow(3 + sinphi, 2));
    dbbb = 2.0/std::sqrt(3)*(dsinphi/(3 + sinphi) - sinphi*dsinphi/std::pow(3 + sinphi, 2));
    break;
  case 2: // lode_zero
    daaa = dC*cosphi + C*dcosphi;
    dbbb = dsinphi/3.0;
    break;
  case 3: // inner_edge
    daaa = 3*dC*cosphi/std::sqrt(9 + 3*std::pow(sinphi, 2)) + 3*C*dcosphi/std::sqrt(9 + 3*std::pow(sinphi, 2)) - 3*C*cosphi*3*sinphi*dsinphi/std::pow(9 + 3*std::pow(sinphi, 2), 1.5);
    dbbb = dsinphi/std::sqrt(9 + 3*std::pow(sinphi, 2)) - 3*sinphi*sinphi*dsinphi/std::pow(9 + 3*std::pow(sinphi, 2), 1.5);
    break;
  case 4: // native
    daaa = dC;
    dbbb = dsinphi/cosphi - sinphi*dcosphi/std::pow(cosphi, 2);
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
    aaa = 2*std::sqrt(3)*C*cosphi/(3 - sinphi);
    bbb = 2*sinphi/std::sqrt(3)/(3 - sinphi);
    break;
  case 1: // inner_tip
    aaa = 2*std::sqrt(3)*C*cosphi/(3 + sinphi);
    bbb = 2*sinphi/std::sqrt(3)/(3 + sinphi);
    break;
  case 2: // lode_zero
    aaa = C*cosphi;
    bbb = sinphi/3;
    break;
  case 3: // inner_edge
    aaa = 3*C*cosphi/std::sqrt(9 + 3*std::pow(sinphi, 2));
    bbb = sinphi/std::sqrt(9 + 3*std::pow(sinphi, 2));
    break;
  case 4: // native
    aaa = C;
    bbb = sinphi/cosphi;
    break;
  }
}

void
TensorMechanicsPlasticDruckerPrager::initializeB(Real intnl, bool friction, Real & bbb) const
{
  const Real sin = (friction) ? std::sin(_mc_phi.value(intnl)) : std::sin(_mc_psi.value(intnl));
  switch (_mc_interpolation_scheme)
  {
  case 0: // outer_tip
    bbb = 2*sin/std::sqrt(3)/(3 - sin);
    break;
  case 1: // inner_tip
    bbb = 2*sin/std::sqrt(3)/(3 + sin);
    break;
  case 2: // lode_zero
    bbb = sin/3;
    break;
  case 3: // inner_edge
    bbb = sin/std::sqrt(9 + 3*std::pow(sin, 2));
    break;
  case 4: // native
    const Real cos = (friction) ? std::cos(_mc_phi.value(intnl)) : std::cos(_mc_psi.value(intnl));
    bbb = sin/cos;
    break;
  }
}


