//* This file is part of the MOOSE framework
//* https://mooseframework.inl.gov
//*
//* All rights reserved, see COPYRIGHT for full restrictions
//* https://github.com/idaholab/moose/blob/master/COPYRIGHT
//*
//* Licensed under LGPL 2.1, please see LICENSE for details
//* https://www.gnu.org/licenses/lgpl-2.1.html

#include "SCMMixingKimAndChung.h"
#include "SCMFrictionClosureBase.h"
#include "SinglePhaseFluidProperties.h"
#include "TriSubChannelMesh.h"
#include "QuadSubChannelMesh.h"

registerMooseObject("SubChannelApp", SCMMixingKimAndChung);

InputParameters
SCMMixingKimAndChung::validParams()
{
  InputParameters params = SCMMixingClosureBase::validParams();
  params.addClassDescription(
      "Class that models the turbulent mixing coefficient using the Kim and Chung correlations.");
  return params;
}

SCMMixingKimAndChung::SCMMixingKimAndChung(const InputParameters & parameters)
  : SCMMixingClosureBase(parameters),
    _is_tri_lattice(dynamic_cast<const TriSubChannelMesh *>(&_subchannel_mesh) != nullptr),
    _sch_mesh(_subchannel_mesh),
    _S_soln(_subproblem.getVariable(0, "S")),
    _mdot_soln(_subproblem.getVariable(0, "mdot")),
    _w_perim_soln(_subproblem.getVariable(0, "w_perim")),
    _mu_soln(_subproblem.getVariable(0, "mu")),
    _P_soln(_subproblem.getVariable(0, "P")),
    _T_soln(_subproblem.getVariable(0, "T"))
{
}

Real
SCMMixingKimAndChung::computeMixingParameter(const unsigned int i_gap,
                                             const unsigned int iz,
                                             const bool sweep_flow) const
{
  if (_is_tri_lattice)
    return computeTriLatticeMixingParameter(i_gap, iz, sweep_flow);
  else
    return computeQuadLatticeMixingParameter(i_gap, iz, sweep_flow);
}

/**
 * Common implementation for both tri and quad lattices.
 * The only lattice-dependent quantities are delta and sf (shape factor).
 */
Real
SCMMixingKimAndChung::computeLatticeMixingParameter(const unsigned int i_gap,
                                                    const unsigned int iz,
                                                    const Real delta,
                                                    const Real sf,
                                                    const bool /*sweep_flow*/) const
{
  const Real P_out = _scm_problem.getOutletPressure();
  const auto fp = _scm_problem.getSinglePhaseFluidProperties();
  const Real pin_diameter = _sch_mesh.getPinDiameter();

  const auto chans = _sch_mesh.getGapChannels(i_gap);
  const unsigned int i_ch = chans.first;
  const unsigned int j_ch = chans.second;

  const Node * const node_in_i = _sch_mesh.getChannelNode(i_ch, iz - 1);
  const Node * const node_out_i = _sch_mesh.getChannelNode(i_ch, iz);
  const Node * const node_in_j = _sch_mesh.getChannelNode(j_ch, iz - 1);
  const Node * const node_out_j = _sch_mesh.getChannelNode(j_ch, iz);

  const Real Si_in = _S_soln(node_in_i);
  const Real Sj_in = _S_soln(node_in_j);
  const Real Si_out = _S_soln(node_out_i);
  const Real Sj_out = _S_soln(node_out_j);

  const Real Si = 0.5 * (Si_in + Si_out);
  const Real Sj = 0.5 * (Sj_in + Sj_out);
  const Real S_total = Si_in + Sj_in + Si_out + Sj_out;

  // crossflow area between channels i,j (dz*gap_width)
  const Real gap = _sch_mesh.getGapWidth(iz, i_gap);

  const Real massflux_i = 0.5 * (_mdot_soln(node_in_i) + _mdot_soln(node_out_i)) / Si;
  const Real massflux_j = 0.5 * (_mdot_soln(node_in_j) + _mdot_soln(node_out_j)) / Sj;

  // average massflux definition
  const Real avg_massflux =
      0.5 * ((_mdot_soln(node_in_i) + _mdot_soln(node_in_j)) / (Si_in + Sj_in) +
             (_mdot_soln(node_out_i) + _mdot_soln(node_out_j)) / (Si_out + Sj_out));

  const Real w_perim_i = 0.5 * (_w_perim_soln(node_in_i) + _w_perim_soln(node_out_i));
  const Real w_perim_j = 0.5 * (_w_perim_soln(node_in_j) + _w_perim_soln(node_out_j));

  const Real mu_i = 0.5 * (_mu_soln(node_in_i) + _mu_soln(node_out_i));
  const Real mu_j = 0.5 * (_mu_soln(node_in_j) + _mu_soln(node_out_j));
  const Real avg_mu = 0.5 * (mu_i + mu_j);

  const Real avg_hD = 4.0 * (Si + Sj) / (w_perim_i + w_perim_j);
  const Real hD_i = 4.0 * Si / w_perim_i;
  const Real hD_j = 4.0 * Sj / w_perim_j;

  const Real Re_i = massflux_i * hD_i / mu_i;
  const Real Re_j = massflux_j * hD_j / mu_j;
  const Real Re = avg_massflux * avg_hD / avg_mu;

  constexpr Real gamma = 20.0; // empirical constant
  constexpr Real a = 0.18;
  constexpr Real b = 0.2;

  const auto friction_args_i = FrictionStruct(i_ch, Re_i, Si, w_perim_i);
  const auto friction_args_j = FrictionStruct(j_ch, Re_j, Sj, w_perim_j);

  const Real fi = _scm_problem.getFrictionClosure()->computeFrictionFactor(friction_args_i);
  const Real fj = _scm_problem.getFrictionClosure()->computeFrictionFactor(friction_args_j);
  const Real f = 0.5 * (fi + fj);

  // average heat conduction
  const Real avg_k =
      (1.0 / S_total) * (fp->k_from_p_T(_P_soln(node_out_i) + P_out, _T_soln(node_out_i)) * Si_out +
                         fp->k_from_p_T(_P_soln(node_in_i) + P_out, _T_soln(node_in_i)) * Si_in +
                         fp->k_from_p_T(_P_soln(node_out_j) + P_out, _T_soln(node_out_j)) * Sj_out +
                         fp->k_from_p_T(_P_soln(node_in_j) + P_out, _T_soln(node_in_j)) * Sj_in);

  // average specific heat capacity
  const Real avg_cp = (1.0 / S_total) *
                      (fp->cp_from_p_T(_P_soln(node_out_i) + P_out, _T_soln(node_out_i)) * Si_out +
                       fp->cp_from_p_T(_P_soln(node_in_i) + P_out, _T_soln(node_in_i)) * Si_in +
                       fp->cp_from_p_T(_P_soln(node_out_j) + P_out, _T_soln(node_out_j)) * Sj_out +
                       fp->cp_from_p_T(_P_soln(node_in_j) + P_out, _T_soln(node_in_j)) * Sj_in);

  const Real Pr = avg_mu * avg_cp / avg_k;                  // Prandtl number
  const Real Pr_t = Pr * (Re / gamma) * std::sqrt(f / 8.0); // Turbulent Prandtl number

  const Real L_x = sf * delta;  // axial length scale (gap is the lateral length scale)
  const Real lamda = gap / L_x; // aspect ratio
  const Real a_x = 1.0 - 2.0 * lamda * lamda / libMesh::pi; // velocity coefficient

  const Real z_FP_over_D =
      (2.0 * L_x / pin_diameter) *
      (1.0 + (-0.5 * std::log(lamda) + 0.5 * std::log(4.0) - 0.25) * lamda * lamda);

  const Real Str =
      1.0 / (0.822 * (gap / pin_diameter) + 0.144); // Strouhal number (Wu & Trupp 1994)

  const Real freq_factor = 2.0 / Utility::pow<2>(gamma) * std::sqrt(a / 8.0) * (avg_hD / gap);

  const Real rod_mixing = (1.0 / Pr_t) * lamda;
  const Real axial_mixing = a_x * z_FP_over_D * Str;

  // Mixing Stanton number: Stg (eq 25,Kim and Chung (2001), eq 19 (Jeong et. al 2005)
  return freq_factor * (rod_mixing + axial_mixing) * std::pow(Re, -b / 2.0);
}

Real
SCMMixingKimAndChung::computeTriLatticeMixingParameter(const unsigned int i_gap,
                                                       const unsigned int iz,
                                                       const bool sweep_flow) const
{
  // main differences for tri lattice: delta and sf
  const Real pitch = _sch_mesh.getPitch();
  const Real delta = pitch / std::sqrt(3.0); // centroid to centroid distance
  constexpr Real sf = 2.0 / 3.0;             // shape factor
  return computeLatticeMixingParameter(i_gap, iz, delta, sf, sweep_flow);
}

Real
SCMMixingKimAndChung::computeQuadLatticeMixingParameter(const unsigned int i_gap,
                                                        const unsigned int iz,
                                                        const bool sweep_flow) const
{
  // main differences for quad lattice: delta and sf
  const Real pitch = _sch_mesh.getPitch();
  const Real delta = pitch; // centroid to centroid distance
  constexpr Real sf = 1.0;  // shape factor
  return computeLatticeMixingParameter(i_gap, iz, delta, sf, sweep_flow);
}
