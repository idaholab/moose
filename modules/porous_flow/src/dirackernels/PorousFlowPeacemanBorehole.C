//* This file is part of the MOOSE framework
//* https://www.mooseframework.org
//*
//* All rights reserved, see COPYRIGHT for full restrictions
//* https://github.com/idaholab/moose/blob/master/COPYRIGHT
//*
//* Licensed under LGPL 2.1, please see LICENSE for details
//* https://www.gnu.org/licenses/lgpl-2.1.html

#include "PorousFlowPeacemanBorehole.h"
#include "RotationMatrix.h"
#include "Function.h"

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
                                        "the borehole");
  params.addRequiredParam<RealVectorValue>(
      "unit_weight",
      "(fluid_density*gravitational_acceleration) as a vector pointing downwards.  "
      "Note that the borehole pressure at a given z position is bottom_p_or_t + "
      "unit_weight*(q - q_bottom), where q=(x,y,z) and q_bottom=(x,y,z) of the "
      "bottom point of the borehole.  The analogous formula holds for "
      "function_of=temperature.  If you don't want bottomhole pressure (or "
      "temperature) to vary in the borehole just set unit_weight=0.  Typical value "
      "is = (0,0,-1E4), for water");
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
      "computed");
  return params;
}

PorousFlowPeacemanBorehole::PorousFlowPeacemanBorehole(const InputParameters & parameters)
  : PorousFlowLineSink(parameters),
    _character(getFunction("character")),
    _p_bot(getFunction("bottom_p_or_t")),
    _unit_weight(getParam<RealVectorValue>("unit_weight")),
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
}

void
PorousFlowPeacemanBorehole::initialSetup()
{
  PorousFlowLineGeometry::initialSetup();
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

  const Real bh_pressure =
      _p_bot.value(_t, _q_point[_qp]) + _unit_weight * (_q_point[_qp] - _bottom_point);
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

  const Real bh_pressure =
      _p_bot.value(_t, _q_point[_qp]) + _unit_weight * (_q_point[_qp] - _bottom_point);
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
