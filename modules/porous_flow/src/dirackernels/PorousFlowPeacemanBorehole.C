//* This file is part of the MOOSE framework
//* https://mooseframework.inl.gov
//*
//* All rights reserved, see COPYRIGHT for full restrictions
//* https://github.com/idaholab/moose/blob/master/COPYRIGHT
//*
//* Licensed under LGPL 2.1, please see LICENSE for details
//* https://www.gnu.org/licenses/lgpl-2.1.html

#include "PorousFlowPeacemanBorehole.h"
#include "RotationMatrix.h"
#include "Function.h"
#include "SinglePhaseFluidProperties.h"
#include "MooseVariableFieldBase.h"
#include "SystemBase.h"
#include "libmesh/system.h"

registerMooseObject("PorousFlowApp", PorousFlowPeacemanBorehole);

InputParameters
PorousFlowPeacemanBorehole::validParams()
{
  InputParameters params = PorousFlowLineSink::validParams();
  params.addRequiredParam<FunctionName>(
      "character",
      "If zero then borehole does nothing.  If positive the borehole acts as a sink "
      "(production well) for porepressure > borehole pressure, and does nothing "
      "otherwise.  If negative the borehole acts as a source (injection well) for "
      "porepressure < borehole pressure, and does nothing otherwise.  The flow rate "
      "to/from the borehole is multiplied by |character|, so usually character = +/- "
      "1, but you can specify other quantities to provide an overall scaling to the "
      "flow if you like.");
  params.addRequiredParam<FunctionName>("bottom_p_or_t",
                                        "For function_of=pressure, this function is the "
                                        "pressure at the bottom of the borehole, "
                                        "otherwise it is the temperature at the bottom of "
                                        "the borehole.");
  params.addParam<RealVectorValue>(
      "unit_weight",
      "(fluid_density*gravitational_acceleration) as a vector pointing downwards.  "
      "Note that the borehole pressure at a given z position is bottom_p_or_t + "
      "unit_weight*(q - q_bottom), where q=(x,y,z) and q_bottom=(x,y,z) of the "
      "bottom point of the borehole.  The analogous formula holds for "
      "function_of=temperature.  If you don't want bottomhole pressure (or "
      "temperature) to vary in the borehole just set unit_weight=0.  Typical value "
      "is = (0,0,-1E4), for water.  Exactly one of 'unit_weight' or 'unit_weight_fp' must be "
      "given.  Use 'unit_weight_fp' instead of 'unit_weight' if you want the fluid unit weight "
      "to vary along the borehole according to a temperature-dependent fluid "
      "density, rather than being a single constant value.");
  params.addParam<UserObjectName>(
      "unit_weight_fp",
      "SinglePhaseFluidProperties UserObject used to evaluate the in-well fluid density from "
      "'unit_weight_temperature' at each borehole point, in order to build a wellbore pressure "
      "profile that accounts for a thermal gradient along the borehole.  Providing this "
      "parameter activates this temperature-dependent unit-weight mode instead of the constant "
      "'unit_weight'.  Not compatible with function_of=temperature, since in that mode "
      "bottom_p_or_t is a temperature, not a pressure, so there is no pressure profile to build.  "
      "If given, 'unit_weight_temperature' and 'unit_weight_gravity' are also required.  "
      "(Deliberately not named 'fp'/'gravity'/'temperature_variable', even though that mirrors "
      "convention elsewhere in PorousFlow, because those are common GlobalParams names: an "
      "input file that sets 'gravity' or 'fp' in [GlobalParams] for unrelated Darcy kernels or "
      "fluid materials would otherwise silently activate, or fail to validate, this mode on "
      "every PorousFlowPeacemanBorehole in the input.)");
  params.addCoupledVar(
      "unit_weight_temperature",
      "The (nonlinear or auxiliary) variable holding temperature, sampled at each borehole point "
      "to compute the in-well fluid density used to build the wellbore pressure profile.  Must "
      "be a variable, not a constant value.  This is unrelated to function_of=temperature "
      "(which instead selects whether the *outflow* driving this DiracKernel is a function of "
      "porepressure or temperature).  Only used, and required, if 'unit_weight_fp' is given.");
  params.addParam<RealVectorValue>(
      "unit_weight_gravity",
      "Gravitational acceleration, pointing downwards, in the units used elsewhere in this "
      "input file (eg (0,0,-9.81) for SI units and lengths in metres).  Only used, and "
      "required, if 'unit_weight_fp' is given.  Densities computed from 'unit_weight_fp' are "
      "always in kg/m^3, so if lengths in this input file are not metres, scale "
      "'unit_weight_gravity' accordingly (eg (0,0,-9.81E-6) if pressures are in MPa and lengths "
      "in metres, matching the 'gravity' convention used by PorousFlow Darcy kernels).");
  MooseEnum temperature_unit_choice("Kelvin=0 Celsius=1", "Kelvin");
  params.addParam<MooseEnum>(
      "unit_weight_temperature_unit",
      temperature_unit_choice,
      "The unit of 'unit_weight_temperature'.  Only used if 'unit_weight_fp' is given.");
  MooseEnum pressure_unit_choice("Pa MPa", "Pa");
  params.addParam<MooseEnum>(
      "unit_weight_pressure_unit",
      pressure_unit_choice,
      "The unit of 'unit_weight_reference_pressure'.  Only used if 'unit_weight_fp' is given.");
  params.addRangeCheckedParam<Real>(
      "unit_weight_reference_pressure",
      101325.0, // standard atmosphere: liquid-water density is only weakly pressure-dependent
                // (~0.9% per 20MPa), so a fixed reference pressure is used instead of the local
                // (coupled, and hence more expensive to sample) porepressure
      "unit_weight_reference_pressure > 0",
      "The fixed pressure (in the units given by 'unit_weight_pressure_unit') at which the "
      "in-well fluid density is evaluated by 'unit_weight_fp'.  Only used if 'unit_weight_fp' "
      "is given.  Choose a value close to the expected wellbore pressure for the most accurate "
      "density.");
  params.addParam<Real>("re_constant",
                        0.28,
                        "The dimensionless constant used in evaluating the borehole effective "
                        "radius.  This depends on the meshing scheme.  Peacemann "
                        "finite-difference calculations give 0.28, while for rectangular finite "
                        "elements the result is closer to 0.1594.  (See  Eqn(4.13) of Z Chen, Y "
                        "Zhang, Well flow models for various numerical methods, Int J Num "
                        "Analysis and Modeling, 3 (2008) 375-388.)");
  params.addParam<Real>("well_constant",
                        -1.0,
                        "Usually this is calculated internally from the element geometry, the "
                        "local borehole direction and segment length, and the permeability.  "
                        "However, if this parameter is given as a positive number then this "
                        "number is used instead of the internal calculation.  This speeds up "
                        "computation marginally.  re_constant becomes irrelevant");
  params.addClassDescription(
      "Approximates a borehole in the mesh using the Peaceman approach, ie "
      "using a number of point sinks with given radii whose positions are "
      "read from a file.  NOTE: if you are using PorousFlowPorosity that depends on volumetric "
      "strain, you should set strain_at_nearest_qp=true in your GlobalParams, to ensure the nodal "
      "Porosity Material uses the volumetric strain at the Dirac quadpoints, and can therefore be "
      "computed.  The wellbore pressure profile is built either from a constant fluid unit "
      "weight ('unit_weight') or, if a thermal gradient along the borehole makes a single "
      "constant unit weight a poor approximation, from a fluid density computed at each "
      "borehole point from a temperature-dependent fluid-properties UserObject "
      "('unit_weight_fp')");
  return params;
}

PorousFlowPeacemanBorehole::PorousFlowPeacemanBorehole(const InputParameters & parameters)
  : PorousFlowLineSink(parameters),
    _character(getFunction("character")),
    _p_bot(getFunction("bottom_p_or_t")),
    _unit_weight(isParamValid("unit_weight") ? getParam<RealVectorValue>("unit_weight")
                                             : RealVectorValue()),
    _use_density_from_temperature(isParamValid("unit_weight_fp")),
    _fp(_use_density_from_temperature ? &getUserObject<SinglePhaseFluidProperties>("unit_weight_fp")
                                      : nullptr),
    _temperature_var(_use_density_from_temperature && isCoupled("unit_weight_temperature")
                         ? getFieldVar("unit_weight_temperature", 0)
                         : nullptr),
    _temperature_system(_temperature_var ? &_temperature_var->sys().system() : nullptr),
    _temperature_var_number(_temperature_var ? _temperature_var->number() : libMesh::invalid_uint),
    _gravity(isParamValid("unit_weight_gravity") ? getParam<RealVectorValue>("unit_weight_gravity")
                                                 : RealVectorValue()),
    _density_reference_pressure(
        getParam<Real>("unit_weight_reference_pressure") *
        (getParam<MooseEnum>("unit_weight_pressure_unit") == 0 ? 1.0 : 1.0E6)),
    _t_c2k(getParam<MooseEnum>("unit_weight_temperature_unit") == 0 ? 0.0 : 273.15),
    _re_constant(getParam<Real>("re_constant")),
    _well_constant(getParam<Real>("well_constant")),
    _has_permeability(
        hasMaterialProperty<RealTensorValue>("PorousFlow_permeability_qp") &&
        hasMaterialProperty<std::vector<RealTensorValue>>("dPorousFlow_permeability_qp_dvar")),
    _has_thermal_conductivity(
        hasMaterialProperty<RealTensorValue>("PorousFlow_thermal_conductivity_qp") &&
        hasMaterialProperty<std::vector<RealTensorValue>>(
            "dPorousFlow_thermal_conductivity_qp_dvar")),
    _perm_or_cond(_p_or_t == PorTchoice::pressure
                      ? getMaterialProperty<RealTensorValue>("PorousFlow_permeability_qp")
                      : getMaterialProperty<RealTensorValue>("PorousFlow_thermal_conductivity_qp")),
    _dperm_or_cond_dvar(
        _p_or_t == PorTchoice::pressure
            ? getMaterialProperty<std::vector<RealTensorValue>>("dPorousFlow_permeability_qp_dvar")
            : getMaterialProperty<std::vector<RealTensorValue>>(
                  "dPorousFlow_thermal_conductivity_qp_dvar"))
{
  if (_p_or_t == PorTchoice::pressure && !_has_permeability)
    mooseError("PorousFlowPeacemanBorehole: You have specified function_of=porepressure, but you "
               "do not have a quadpoint permeability material");
  if (_p_or_t == PorTchoice::temperature && !_has_thermal_conductivity)
    mooseError("PorousFlowPeacemanBorehole: You have specified function_of=temperature, but you do "
               "not have a quadpoint thermal_conductivity material");

  // The wellbore pressure profile is built from either a single constant unit_weight, or a
  // fluid density computed from temperature at each point (via 'unit_weight_fp') - never both,
  // and never neither.  Note this deliberately does not error if 'unit_weight_temperature' or
  // 'unit_weight_gravity' are valid while 'unit_weight_fp' is not: unlike 'unit_weight' and
  // 'unit_weight_fp' themselves, those two are not required to detect the user's intended mode,
  // so treating their unexpected presence as an error would only serve to reject input files
  // that harmlessly set an identically-named GlobalParam for some unrelated object.
  const int checkWellborePressureFormat =
      int(isParamValid("unit_weight")) + int(isParamValid("unit_weight_fp"));
  if (checkWellborePressureFormat > 1)
    paramError("unit_weight",
               "PorousFlowPeacemanBorehole: must specify only one of 'unit_weight' (a constant "
               "fluid unit weight) or 'unit_weight_fp' (a fluid-properties UserObject, so that "
               "the fluid unit weight is instead computed from the temperature at each borehole "
               "point)");
  else if (checkWellborePressureFormat == 0)
    paramError("unit_weight",
               "PorousFlowPeacemanBorehole: must specify at least one of 'unit_weight' or "
               "'unit_weight_fp'");

  if (_use_density_from_temperature)
  {
    if (_p_or_t == PorTchoice::temperature)
      paramError("unit_weight_fp",
                 "PorousFlowPeacemanBorehole: 'unit_weight_fp' computes a fluid density to "
                 "build a hydrostatic *pressure* profile along the borehole, which is "
                 "meaningless when function_of=temperature (bottom_p_or_t is then a "
                 "temperature, not a pressure)");
    if (!isParamValid("unit_weight_temperature"))
      paramError("unit_weight_temperature",
                 "PorousFlowPeacemanBorehole: 'unit_weight_temperature' must be supplied when "
                 "'unit_weight_fp' is supplied");
    if (!isCoupled("unit_weight_temperature"))
      paramError("unit_weight_temperature",
                 "PorousFlowPeacemanBorehole: 'unit_weight_temperature' must be a nonlinear or "
                 "auxiliary variable, not a constant value.  The wellbore pressure profile is "
                 "built by sampling this variable at each borehole point, so a spatially "
                 "constant temperature has no profile to sample: use 'unit_weight' instead if "
                 "the in-well fluid density really is constant");
    if (!isParamValid("unit_weight_gravity"))
      paramError("unit_weight_gravity",
                 "PorousFlowPeacemanBorehole: 'unit_weight_gravity' must be supplied when "
                 "'unit_weight_fp' is supplied");
  }
}

void
PorousFlowPeacemanBorehole::initialSetup()
{
  PorousFlowLineGeometry::initialSetup();

  if (!_point_file.empty() && _zs[0] < _zs.back())
    mooseError("PorousFlowPeacemanBorehole: The last entry in the point_file needs to be at the "
               "bottom of the well_bore because this is the point where the function bottom_p_or_t "
               "is evaluated.  The depth of the first point is z=",
               _zs[0],
               " and the last point is z=",
               _zs.back());

  // construct the rotation matrix needed to rotate the permeability
  const unsigned int num_pts = _zs.size();
  _rot_matrix.resize(std::max(num_pts - 1, (unsigned)1));
  for (unsigned int i = 0; i + 1 < num_pts; ++i)
  {
    const RealVectorValue v2(_xs[i + 1] - _xs[i], _ys[i + 1] - _ys[i], _zs[i + 1] - _zs[i]);
    _rot_matrix[i] = RotationMatrix::rotVecToZ(v2);
  }
  if (num_pts == (unsigned)1)
    _rot_matrix[0] = RotationMatrix::rotVecToZ(_line_direction);
}

void
PorousFlowPeacemanBorehole::residualSetup()
{
  PorousFlowLineSink::residualSetup();
  computeWellborePressures();
}

void
PorousFlowPeacemanBorehole::jacobianSetup()
{
  PorousFlowLineSink::jacobianSetup();
  computeWellborePressures();
}

void
PorousFlowPeacemanBorehole::computeWellborePressures()
{
  if (!_use_density_from_temperature)
    return;

  const std::size_t num_pts = _z_coord->size();
  _bh_pressure.assign(num_pts, 0.0);
  if (num_pts == 0)
    return;

  // Sample the temperature, and hence the in-well fluid density, at every well point (not just
  // the points owned by this processor), because the wellbore pressure at any point is a
  // cumulative integral over all the points between it and the bottom point.
  // System::point_value performs the parallel point-location and broadcast internally, so this
  // is safe to call even for points this processor's mesh partition does not own.
  std::vector<Real> density(num_pts);
  for (const auto i : make_range(num_pts))
  {
    const Point p(_x_coord->at(i), _y_coord->at(i), _z_coord->at(i));
    const Real temperature = _temperature_system->point_value(_temperature_var_number, p, false);
    density[i] = _fp->rho_from_p_T(_density_reference_pressure, temperature + _t_c2k);
  }

  // Integrate the fluid unit weight (density*gravity) up the wellbore from the bottom point,
  // where the pressure is prescribed by bottom_p_or_t, using the trapezoidal rule on each
  // polyline segment.  This is exact for a density varying linearly along the well (the target
  // use case: a linear thermal gradient with a locally-linear density(temperature)), and it
  // degenerates exactly to the constant-unit_weight formula when the density is constant, since
  // the sum then telescopes to density*gravity.(x_i - x_bottom).
  _bh_pressure[num_pts - 1] = _p_bot.value(_t, _bottom_point);
  for (std::size_t i = num_pts - 1; i > 0; --i)
  {
    const RealVectorValue segment(_x_coord->at(i - 1) - _x_coord->at(i),
                                  _y_coord->at(i - 1) - _y_coord->at(i),
                                  _z_coord->at(i - 1) - _z_coord->at(i));
    _bh_pressure[i - 1] =
        _bh_pressure[i] + 0.5 * (density[i - 1] + density[i]) * (_gravity * segment);
  }
}

Real
PorousFlowPeacemanBorehole::wellborePressure(unsigned current_dirac_ptid) const
{
  if (!_use_density_from_temperature)
    return _p_bot.value(_t, _bottom_point) + _unit_weight * (_q_point[_qp] - _bottom_point);

  mooseAssert(current_dirac_ptid < _bh_pressure.size(),
              "PorousFlowPeacemanBorehole: the wellbore pressure profile has not been computed "
              "for this Dirac point");
  return _bh_pressure[current_dirac_ptid];
}

Real
PorousFlowPeacemanBorehole::wellConstant(const RealTensorValue & perm,
                                         const RealTensorValue & rot,
                                         const Real & half_len,
                                         const Elem * ele,
                                         const Real & rad) const
// Peaceman's form for the borehole well constant
{
  if (_well_constant > 0)
    return _well_constant;

  // rot_perm has its "2" component lying along the half segment.
  // We want to determine the eigenvectors of rot(0:1, 0:1), since, when
  // rotated back to the original frame we will determine the element
  // lengths along these directions
  const RealTensorValue rot_perm = (rot * perm) * rot.transpose();
  const Real trace2D = rot_perm(0, 0) + rot_perm(1, 1);
  const Real det2D = rot_perm(0, 0) * rot_perm(1, 1) - rot_perm(0, 1) * rot_perm(1, 0);
  const Real sq = std::sqrt(std::max(0.25 * trace2D * trace2D - det2D,
                                     0.0)); // the std::max accounts for wierdo precision loss
  const Real eig_val1 = 0.5 * trace2D + sq;
  const Real eig_val2 = 0.5 * trace2D - sq;
  RealVectorValue eig_vec1, eig_vec2;
  if (sq > std::abs(trace2D) * 1E-7) // matrix is not a multiple of the identity (1E-7 accounts for
                                     // precision in a crude way)
  {
    if (rot_perm(1, 0) != 0)
    {
      eig_vec1(0) = eig_val1 - rot_perm(1, 1);
      eig_vec1(1) = rot_perm(1, 0);
      eig_vec2(0) = eig_val2 - rot_perm(1, 1);
      eig_vec2(1) = rot_perm(1, 0);
    }
    else if (rot_perm(0, 1) != 0)
    {
      eig_vec1(0) = rot_perm(0, 1);
      eig_vec1(1) = eig_val1 - rot_perm(0, 0);
      eig_vec2(0) = rot_perm(0, 1);
      eig_vec2(1) = eig_val2 - rot_perm(0, 0);
    }
    else // off diagonal terms are both zero
    {
      eig_vec1(0) = 1.0;
      eig_vec2(1) = 1.0;
    }
  }
  else // matrix is basically a multiple of the identity
  {
    eig_vec1(0) = 1.0;
    eig_vec2(1) = 1.0;
  }

  // finally, rotate these to original frame and normalise
  eig_vec1 = rot.transpose() * eig_vec1;
  eig_vec1 /= std::sqrt(eig_vec1 * eig_vec1);
  eig_vec2 = rot.transpose() * eig_vec2;
  eig_vec2 /= std::sqrt(eig_vec2 * eig_vec2);

  // find the "length" of the element in these directions
  // TODO - maybe better to use variance than max&min
  Real max1 = eig_vec1 * ele->point(0);
  Real max2 = eig_vec2 * ele->point(0);
  Real min1 = max1;
  Real min2 = max2;
  Real proj;
  for (unsigned int i = 1; i < ele->n_nodes(); i++)
  {
    proj = eig_vec1 * ele->point(i);
    max1 = (max1 < proj) ? proj : max1;
    min1 = (min1 < proj) ? min1 : proj;

    proj = eig_vec2 * ele->point(i);
    max2 = (max2 < proj) ? proj : max2;
    min2 = (min2 < proj) ? min2 : proj;
  }
  const Real ll1 = max1 - min1;
  const Real ll2 = max2 - min2;

  Real r0;
  if (eig_val1 <= 0.0)
    r0 = _re_constant * ll1;
  else if (eig_val2 <= 0.0)
    r0 = _re_constant * ll2;
  else
    r0 = _re_constant *
         std::sqrt(std::sqrt(eig_val1 / eig_val2) * std::pow(ll2, 2) +
                   std::sqrt(eig_val2 / eig_val1) * std::pow(ll1, 2)) /
         (std::pow(eig_val1 / eig_val2, 0.25) + std::pow(eig_val2 / eig_val1, 0.25));

  const Real effective_perm = (det2D >= 0.0 ? std::sqrt(det2D) : 0.0);

  const Real halfPi = acos(0.0);

  if (r0 <= rad)
    mooseError("The effective element size (about 0.2-times-true-ele-size) for an element "
               "containing a Peaceman-type borehole must be (much) larger than the borehole radius "
               "for the Peaceman formulation to be correct.  Your element has effective size ",
               r0,
               " and the borehole radius is ",
               rad,
               "\n");

  return 4 * halfPi * effective_perm * half_len / std::log(r0 / rad);
}

Real
PorousFlowPeacemanBorehole::computeQpBaseOutflow(unsigned current_dirac_ptid) const
{
  const Real character = _character.value(_t, _q_point[_qp]);
  if (character == 0.0)
    return 0.0;

  const Real bh_pressure = wellborePressure(current_dirac_ptid);
  const Real pp = ptqp();

  Real outflow = 0.0; // this is the flow rate from porespace out of the system

  if (current_dirac_ptid > 0)
  // contribution from half-segment "behind" this point (must have >1 point for
  // current_dirac_ptid>0)
  {
    if ((character < 0.0 && pp < bh_pressure) || (character > 0.0 && pp > bh_pressure))
    {
      // injection, so outflow<0 || production, so outflow>0
      const Real wc = wellConstant(_perm_or_cond[_qp],
                                   _rot_matrix[current_dirac_ptid - 1],
                                   _half_seg_len[current_dirac_ptid - 1],
                                   _current_elem,
                                   _weight->at(current_dirac_ptid));
      outflow += wc * (pp - bh_pressure);
    }
  }

  if (current_dirac_ptid + 1 < _zs.size() || _zs.size() == 1)
  // contribution from half-segment "ahead of" this point, or we only have one point
  {
    if ((character < 0.0 && pp < bh_pressure) || (character > 0.0 && pp > bh_pressure))
    {
      // injection, so outflow<0 || // production, so outflow>0
      const Real wc = wellConstant(_perm_or_cond[_qp],
                                   _rot_matrix[current_dirac_ptid],
                                   _half_seg_len[current_dirac_ptid],
                                   _current_elem,
                                   _weight->at(current_dirac_ptid));
      outflow += wc * (pp - bh_pressure);
    }
  }

  return outflow * _test[_i][_qp] * std::abs(character);
}

void
PorousFlowPeacemanBorehole::computeQpBaseOutflowJacobian(unsigned jvar,
                                                         unsigned current_dirac_ptid,
                                                         Real & outflow,
                                                         Real & outflowp) const
{
  outflow = 0.0;
  outflowp = 0.0;

  const Real character = _character.value(_t, _q_point[_qp]);
  if (character == 0.0)
    return;

  if (_dictator.notPorousFlowVariable(jvar))
    return;
  const unsigned pvar = _dictator.porousFlowVariableNum(jvar);

  // When _use_density_from_temperature is true, bh_pressure also depends on the temperature at
  // every well point between here and the bottom point (via computeWellborePressures()), not
  // just on the porous flow variables at this quadpoint.  That dependence is deliberately not
  // differentiated here: a DiracKernel can only assemble into the (test, phi) dofs of the
  // element containing its own quadpoint, so the coupling to temperature dofs in other elements
  // along the well cannot be represented in this Jacobian.  residualSetup()/jacobianSetup()
  // still recompute bh_pressure from the current nonlinear iterate before every evaluation, so
  // the converged solution is unaffected; only the Newton convergence rate may be mildly slower.
  const Real bh_pressure = wellborePressure(current_dirac_ptid);
  const Real pp = ptqp();
  const Real pp_prime = dptqp(pvar) * _phi[_j][_qp];

  if (current_dirac_ptid > 0)
  // contribution from half-segment "behind" this point
  {
    if ((character < 0.0 && pp < bh_pressure) || (character > 0.0 && pp > bh_pressure))
    {
      // injection, so outflow<0 || // production, so outflow>0
      const Real wc = wellConstant(_perm_or_cond[_qp],
                                   _rot_matrix[current_dirac_ptid - 1],
                                   _half_seg_len[current_dirac_ptid - 1],
                                   _current_elem,
                                   _weight->at(current_dirac_ptid));
      outflowp += wc * pp_prime;
      outflow += wc * (pp - bh_pressure);
    }
  }

  if (current_dirac_ptid < _zs.size() - 1 || _zs.size() == 1)
  // contribution from half-segment "ahead of" this point
  {
    if ((character < 0.0 && pp < bh_pressure) || (character > 0.0 && pp > bh_pressure))
    {
      // injection, so outflow<0 || // production, so outflow>0
      const Real wc = wellConstant(_perm_or_cond[_qp],
                                   _rot_matrix[current_dirac_ptid],
                                   _half_seg_len[current_dirac_ptid],
                                   _current_elem,
                                   _weight->at(current_dirac_ptid));
      outflowp += wc * pp_prime;
      outflow += wc * (pp - bh_pressure);
    }
  }

  outflowp *= _test[_i][_qp] * std::abs(character);
  outflow *= _test[_i][_qp] * std::abs(character);
}
