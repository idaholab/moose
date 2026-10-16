//* This file is part of the MOOSE framework
//* https://mooseframework.inl.gov
//*
//* All rights reserved, see COPYRIGHT for full restrictions
//* https://github.com/idaholab/moose/blob/master/COPYRIGHT
//*
//* Licensed under LGPL 2.1, please see LICENSE for details
//* https://www.gnu.org/licenses/lgpl-2.1.html

#include "FVPorousFlowPeacemanBorehole.h"
#include "PorousFlowDictator.h"
#include "PorousFlowSumQuantity.h"
#include "PorousFlowPeacemanBorehole.h"
#include "Function.h"
#include "MooseVariableScalar.h"
#include "NonlinearSystemBase.h"
#include "Assembly.h"
#include "RotationMatrix.h"

registerMooseObject("PorousFlowApp", FVPorousFlowPeacemanBorehole);

InputParameters
FVPorousFlowPeacemanBorehole::validParams()
{
  InputParameters params = PorousFlowLineGeometry::validParams();

  // As PorousFlowLineSink, without the heat-transfer options
  params.addParam<UserObjectName>(
      "SumQuantityUO",
      "User Object of type=PorousFlowSumQuantity in which to place the total outflow from the "
      "borehole for each time step");
  params.addRequiredParam<UserObjectName>(
      "PorousFlowDictator", "The UserObject that holds the list of PorousFlow variable names");
  params.addParam<unsigned int>(
      "fluid_phase",
      0,
      "The fluid phase whose pressure (and potentially mobility, etc) controls the flux to the "
      "borehole");
  params.addParam<unsigned int>("mass_fraction_component",
                                "The index corresponding to a fluid component.  If supplied, the "
                                "flux will be multiplied by the mass fraction for the component");
  params.addParam<bool>(
      "use_relative_permeability", false, "Multiply the flux by the fluid relative permeability");
  params.addParam<bool>("use_mobility", false, "Multiply the flux by the fluid mobility");
  params.addCoupledVar("multiplying_var", 1.0, "Fluxes will be multiplied by this variable");

  // As PorousFlowPeacemanBorehole
  params.addRequiredParam<FunctionName>(
      "character",
      "If zero then borehole does nothing.  Unless allow_crossflow = true: if positive the "
      "borehole acts as a sink (production well) for porepressure > borehole pressure, and does "
      "nothing otherwise; if negative the borehole acts as a source (injection well) for "
      "porepressure < borehole pressure, and does nothing otherwise.  The flow rate to/from the "
      "borehole is multiplied by |character|, so usually character = +/- 1");
  params.addRequiredParam<RealVectorValue>(
      "unit_weight",
      "(fluid_density*gravitational_acceleration) as a vector pointing downwards.  Note that the "
      "borehole pressure at a given z position is bottom pressure + unit_weight*(q - q_bottom), "
      "where q=(x,y,z) and q_bottom=(x,y,z) of the bottom point of the borehole.  If you don't "
      "want bottomhole pressure to vary in the borehole just set unit_weight=0.  Typical value is "
      "= (0,0,-1E4), for water");
  params.addParam<Real>("re_constant",
                        0.28,
                        "The dimensionless constant used in evaluating the borehole effective "
                        "radius.  Peaceman finite-difference (and cell-centred finite-volume) "
                        "calculations give 0.28");
  params.addParam<Real>("well_constant",
                        -1.0,
                        "Usually this is calculated internally from the element geometry, the "
                        "local borehole direction and segment length, and the permeability.  "
                        "However, if this parameter is given as a positive number then this "
                        "number is used instead of the internal calculation.  re_constant "
                        "becomes irrelevant");

  // Control
  MooseEnum control("bhp rate", "bhp");
  params.addParam<MooseEnum>(
      "control",
      control,
      "bhp: the bottomhole pressure is prescribed by bottom_p_or_t.  rate: the total mass rate is "
      "prescribed by rate, and the bottomhole pressure is the scalar variable bottom_p");
  params.addParam<FunctionName>("bottom_p_or_t",
                                "The pressure at the bottom of the borehole (control = bhp)");
  params.addParam<FunctionName>(
      "rate",
      "Total mass rate of the borehole (kg/s) for control = rate: the sum of the fluxes of all "
      "the DiracKernels of this borehole (those sharing bottom_p).  Positive for production "
      "(fluid leaving the porous medium), negative for injection");
  params.addCoupledVar(
      "bottom_p",
      "Scalar variable for the bottomhole pressure (control = rate).  Its equation (rate - sum of "
      "the borehole fluxes = 0) is assembled by the DiracKernels of this borehole, so it "
      "only needs a NullScalarKernel.  Its initial condition is the starting guess");

  // Flow direction and injection
  params.addParam<bool>(
      "allow_crossflow",
      false,
      "Allow fluid to flow in either direction at every borehole point.  The sign of character is "
      "then ignored");
  params.addParam<unsigned int>(
      "injection_phase",
      "If given, the flow direction at a point is "
      "decided by the total volumetric flow of all phases, and at injecting points only this "
      "phase enters, with the total mobility (or total relative permeability) of the cell");

  params.addClassDescription(
      "Approximates a borehole in a finite-volume mesh using the Peaceman approach, with either a "
      "prescribed bottomhole pressure or a prescribed total mass rate.  Only fluid flow is "
      "modelled: heat transfer is not implemented");
  return params;
}

FVPorousFlowPeacemanBorehole::FVPorousFlowPeacemanBorehole(const InputParameters & parameters)
  : PorousFlowLineGeometry(parameters),
    _dictator(getUserObject<PorousFlowDictator>("PorousFlowDictator")),
    _total_outflow_mass(isParamValid("SumQuantityUO")
                            ? &const_cast<PorousFlowSumQuantity &>(
                                  getUserObject<PorousFlowSumQuantity>("SumQuantityUO"))
                            : nullptr),
    _ph(getParam<unsigned int>("fluid_phase")),
    _use_mass_fraction(isParamValid("mass_fraction_component")),
    _sp(_use_mass_fraction ? getParam<unsigned int>("mass_fraction_component") : 0),
    _use_relative_permeability(getParam<bool>("use_relative_permeability")),
    _use_mobility(getParam<bool>("use_mobility")),
    _use_injection_phase(isParamValid("injection_phase")),
    _injection_phase(_use_injection_phase ? getParam<unsigned int>("injection_phase") : 0),
    _allow_crossflow(getParam<bool>("allow_crossflow")),
    _character(getFunction("character")),
    _unit_weight(getParam<RealVectorValue>("unit_weight")),
    _re_constant(getParam<Real>("re_constant")),
    _well_constant(getParam<Real>("well_constant")),
    _control(getParam<MooseEnum>("control").getEnum<Control>()),
    _bottom_p_function(isParamValid("bottom_p_or_t") ? &getFunction("bottom_p_or_t") : nullptr),
    _rate(isParamValid("rate") ? &getFunction("rate") : nullptr),
    _bottom_p_var(isCoupledScalar("bottom_p") ? getScalarVar("bottom_p", 0) : nullptr),
    _bottom_p_value(isCoupledScalar("bottom_p") ? &adCoupledScalarValue("bottom_p") : nullptr),
    _adds_rate(false),
    _multiplying_var(adCoupledValue("multiplying_var")),
    _pressure(getADMaterialProperty<std::vector<Real>>("PorousFlow_porepressure_qp")),
    _mass_fractions(_use_mass_fraction ? &getADMaterialProperty<std::vector<std::vector<Real>>>(
                                             "PorousFlow_mass_frac_qp")
                                       : nullptr),
    _density(getADMaterialProperty<std::vector<Real>>("PorousFlow_fluid_phase_density_qp")),
    _viscosity(getADMaterialProperty<std::vector<Real>>("PorousFlow_viscosity_qp")),
    _relperm(getADMaterialProperty<std::vector<Real>>("PorousFlow_relative_permeability_qp")),
    _permeability(getADMaterialProperty<RealTensorValue>("PorousFlow_permeability_qp"))
{
  if (!_var.isFV())
    paramError("variable",
               "FVPorousFlowPeacemanBorehole is for finite-volume variables.  For finite elements "
               "use PorousFlowPeacemanBorehole");
  if (_ph >= _dictator.numPhases())
    paramError("fluid_phase",
               "The Dictator proclaims that the maximum phase index in this simulation is ",
               _dictator.numPhases() - 1);
  if (_use_mass_fraction && _sp >= _dictator.numComponents())
    paramError("mass_fraction_component",
               "The Dictator proclaims that the maximum fluid component index in this simulation "
               "is ",
               _dictator.numComponents() - 1);
  if (_use_injection_phase && _injection_phase >= _dictator.numPhases())
    paramError("injection_phase",
               "The Dictator proclaims that the maximum phase index in this simulation is ",
               _dictator.numPhases() - 1);

  if (_control == Control::bhp)
  {
    if (!_bottom_p_function)
      paramError("bottom_p_or_t", "bottom_p_or_t must be given for control = bhp");
    if (_rate || _bottom_p_var)
      paramError("control", "rate and bottom_p are only used with control = rate");
  }
  else
  {
    if (!_rate || !_bottom_p_var)
      paramError("control", "rate and bottom_p must be given for control = rate");
    if (_bottom_p_function)
      paramError("bottom_p_or_t",
                 "bottom_p_or_t is not used with control = rate: the bottomhole pressure is the "
                 "bottom_p variable");
    if (_bottom_p_var->order() != 1)
      paramError("bottom_p", "The bottomhole pressure variable must be first order");
    if (!_use_mobility)
      paramError("use_mobility",
                 "With control = rate the borehole fluxes must be mass fluxes, so use_mobility = "
                 "true is required");
  }

  // The fluxes depend on all the PorousFlow variables (through the material properties)
  for (auto * var : _dictator.getCoupledMooseVars())
    addMooseVariableDependency(var);
}

unsigned int
FVPorousFlowPeacemanBorehole::bottomPressureVariableNumber() const
{
  return _bottom_p_var ? _bottom_p_var->number() : libMesh::invalid_uint;
}

void
FVPorousFlowPeacemanBorehole::initialSetup()
{
  PorousFlowLineGeometry::initialSetup();

  if (!_point_file.empty() && _zs[0] < _zs.back())
    paramError("point_file",
               "The last entry in the point_file needs to be at the bottom of the borehole, "
               "because this is the point where the bottomhole pressure is defined.  The depth of "
               "the first point is z=",
               _zs[0],
               " and the last point is z=",
               _zs.back());

  if (_control == Control::rate)
  {
    // The DiracKernels sharing bottom_p are one borehole.  Each adds its flux to the
    // bottomhole-pressure equation, and the first one also adds the prescribed rate
    const FVPorousFlowPeacemanBorehole * adder = nullptr;
    const auto & dirac_kernels = _fe_problem.getNonlinearSystemBase(_sys.number())
                                     .getDiracKernelWarehouse()
                                     .getActiveObjects();
    for (const auto & dk : dirac_kernels)
    {
      const auto * bh = dynamic_cast<const FVPorousFlowPeacemanBorehole *>(dk.get());
      if (!bh || !bh->rateControlled() ||
          bh->bottomPressureVariableNumber() != bottomPressureVariableNumber())
        continue;
      if (bh->getParam<FunctionName>("rate") != getParam<FunctionName>("rate"))
        paramError("rate",
                   "The DiracKernels ",
                   name(),
                   " and ",
                   bh->name(),
                   " share bottom_p, so they are one borehole and must use the same rate");
      if (!adder)
        adder = bh;
    }
    _adds_rate = (adder == this);
  }
}

void
FVPorousFlowPeacemanBorehole::addPoints()
{
  // This function gets called just before the DiracKernel is evaluated, so this is a handy place
  // to zero the outflow (as PorousFlowLineSink)
  if (_total_outflow_mass)
    _total_outflow_mass->zero();

  PorousFlowLineGeometry::addPoints();

  // The rotation matrices needed to rotate the permeability (as PorousFlowPeacemanBorehole).  The
  // points may change with the mesh when they are generated from line_base
  const auto num_pts = _zs.size();
  _rot_matrix.resize(std::max(num_pts, std::size_t(2)) - 1);
  for (std::size_t i = 0; i + 1 < num_pts; ++i)
    _rot_matrix[i] = RotationMatrix::rotVecToZ(
        RealVectorValue(_xs[i + 1] - _xs[i], _ys[i + 1] - _ys[i], _zs[i + 1] - _zs[i]));
  if (num_pts == 1)
    _rot_matrix[0] = RotationMatrix::rotVecToZ(_line_direction);
}

Real
FVPorousFlowPeacemanBorehole::computeQpResidual()
{
  mooseError("FVPorousFlowPeacemanBorehole::computeQpResidual should not be called");
}

Real
FVPorousFlowPeacemanBorehole::wellConstant(unsigned int i, const RealTensorValue & perm) const
{
  // Sum over the half segments either side of point i, as PorousFlowPeacemanBorehole
  const auto half_segment = [&](unsigned int s)
  {
    if (_well_constant > 0.0)
      return _well_constant;
    return PorousFlowPeacemanBorehole::peacemanWellConstant(
        perm, _rot_matrix[s], _half_seg_len[s], _current_elem, _weight->at(i), _re_constant);
  };

  Real wc = 0.0;
  if (i > 0)
    wc += half_segment(i - 1);
  if (i + 1 < _zs.size() || _zs.size() == 1)
    wc += half_segment(i);
  return wc;
}

ADReal
FVPorousFlowPeacemanBorehole::pointOutflow(unsigned int i)
{
  const Point pt(_xs[i], _ys[i], _zs[i]);
  const Real character = _character.value(_t, pt);
  if (character == 0.0)
    return 0.0;

  const ADReal bottom_p = _control == Control::rate
                              ? (*_bottom_p_value)[0]
                              : ADReal(_bottom_p_function->value(_t, _bottom_point));
  const ADReal p_conn = bottom_p + _unit_weight * (pt - _bottom_point);

  // Flow direction: the total volumetric flow (injection_phase given) or this phase's pressure
  // difference
  Real direction = 0.0;
  if (_use_injection_phase)
    for (const auto ph : make_range(_dictator.numPhases()))
      direction += MetaPhysicL::raw_value(_relperm[_qp][ph] / _viscosity[_qp][ph] *
                                          (_pressure[_qp][ph] - p_conn));
  else
    direction = MetaPhysicL::raw_value(_pressure[_qp][_ph] - p_conn);
  if (!_allow_crossflow)
  {
    // As PorousFlowPeacemanBorehole: fluid only leaves a producer and only enters an injector
    if (direction == 0.0 || (direction > 0.0) != (character > 0.0))
      return 0.0;
  }
  // With crossflow, zero drawdown counts as producing, so that the derivatives are kept
  const bool producing = direction >= 0.0;

  // With injection_phase given, only the injection phase enters at injecting points, with the
  // total mobility of the cell
  const bool phase_injection = !producing && _use_injection_phase;
  if (phase_injection && _ph != _injection_phase)
    return 0.0;

  ADReal multiplier = std::abs(character) *
                      wellConstant(i, MetaPhysicL::raw_value(_permeability[_qp])) *
                      _multiplying_var[_qp];
  if (_use_relative_permeability || _use_mobility)
  {
    ADReal relperm = _relperm[_qp][_ph];
    ADReal mobility = _relperm[_qp][_ph] / _viscosity[_qp][_ph];
    if (phase_injection)
    {
      relperm = 0.0;
      mobility = 0.0;
      for (const auto ph : make_range(_dictator.numPhases()))
      {
        relperm += _relperm[_qp][ph];
        mobility += _relperm[_qp][ph] / _viscosity[_qp][ph];
      }
    }
    if (_use_relative_permeability)
      multiplier *= relperm;
    if (_use_mobility)
      multiplier *= _density[_qp][_ph] * mobility;
  }
  if (_use_mass_fraction)
    multiplier *= (*_mass_fractions)[_qp][_ph][_sp];

  return multiplier * (_pressure[_qp][_ph] - p_conn);
}

void
FVPorousFlowPeacemanBorehole::cellOutflow(ADReal & outflow, ADReal & rate_residual)
{
  outflow = 0.0;
  rate_residual = 0.0;
  for (_qp = 0; _qp < _qrule->n_points(); _qp++)
  {
    _current_point = _physical_point[_qp];
    if (!isActiveAtPoint(_current_elem, _current_point))
      continue;
    const unsigned int i = currentPointCachedID();
    mooseAssert(i < _zs.size(), "Borehole point not found in the Dirac point cache");

    const ADReal q = pointOutflow(i);
    outflow += q;
    if (_control == Control::rate)
    {
      // rate - sum of the fluxes, with the prescribed rate added once, at the bottom point.  This
      // sign gives d(residual)/d(bottom_p) > 0
      rate_residual -= q;
      if (_adds_rate && i + 1 == _zs.size())
        rate_residual += _rate->value(_t, _bottom_point);
    }
  }
}

void
FVPorousFlowPeacemanBorehole::computeResidual()
{
  ADReal outflow, rate_residual;
  cellOutflow(outflow, rate_residual);

  // A finite-volume variable has one dof per cell, whose test function is 1 in the cell
  addResiduals(_assembly,
               std::array<Real, 1>{{MetaPhysicL::raw_value(outflow)}},
               _var.dofIndices(),
               _var.scalingFactor());
  // There may be no ScalarKernels, so add the bottomhole-pressure residual directly
  if (_control == Control::rate)
    addResiduals(_assembly,
                 std::array<Real, 1>{{MetaPhysicL::raw_value(rate_residual)}},
                 _bottom_p_var->dofIndices(),
                 _bottom_p_var->scalingFactor());

  if (_total_outflow_mass)
    _total_outflow_mass->add(MetaPhysicL::raw_value(outflow) * _dt);
}

void
FVPorousFlowPeacemanBorehole::computeJacobian()
{
  // AD gives the derivatives with respect to all the variables at once (PorousFlow variables and
  // the bottomhole pressure)
  ADReal outflow, rate_residual;
  cellOutflow(outflow, rate_residual);

  addJacobian(_assembly, std::array<ADReal, 1>{{outflow}}, _var.dofIndices(), _var.scalingFactor());
  if (_control == Control::rate)
    addJacobian(_assembly,
                std::array<ADReal, 1>{{rate_residual}},
                _bottom_p_var->dofIndices(),
                _bottom_p_var->scalingFactor());
}

void
FVPorousFlowPeacemanBorehole::computeResidualAndJacobian()
{
  ADReal outflow, rate_residual;
  cellOutflow(outflow, rate_residual);

  addResidualsAndJacobian(
      _assembly, std::array<ADReal, 1>{{outflow}}, _var.dofIndices(), _var.scalingFactor());
  if (_control == Control::rate)
    addResidualsAndJacobian(_assembly,
                            std::array<ADReal, 1>{{rate_residual}},
                            _bottom_p_var->dofIndices(),
                            _bottom_p_var->scalingFactor());

  if (_total_outflow_mass)
    _total_outflow_mass->add(MetaPhysicL::raw_value(outflow) * _dt);
}

void
FVPorousFlowPeacemanBorehole::computeOffDiagJacobian(unsigned int jvar)
{
  // ComputeDiracThread calls this once per coupled variable.  The AD Jacobian already holds every
  // column, so assemble it only once, for the diagonal block
  if (jvar == _var.number())
    computeJacobian();
}
