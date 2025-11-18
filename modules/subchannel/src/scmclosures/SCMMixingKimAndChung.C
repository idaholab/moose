//* This file is part of the MOOSE framework
//* https://mooseframework.inl.gov
//*
//* All rights reserved, see COPYRIGHT for full restrictions
//* https://github.com/idaholab/moose/blob/master/COPYRIGHT
//*
//* Licensed under LGPL 2.1, please see LICENSE for details
//* https://www.gnu.org/licenses/lgpl-2.1.html

#include "SCMMixingKimAndChung.h"
#include "SinglePhaseFluidProperties.h"

registerMooseObject("SubChannelApp", SCMMixingKimAndChung);

InputParameters
SCMMixingKimAndChung::validParams()
{
  InputParameters params = SCMMixingClosureBase::validParams();
  params.addClassDescription(
      "Class that models the mixing parameter beta using the Kim and Chung correlations.");
  return params;
}

SCMMixingKimAndChung::SCMMixingKimAndChung(const InputParameters & parameters)
  : SCMMixingClosureBase(parameters),
    _is_tri_lattice(dynamic_cast<const TriSubChannelMesh *>(&_subchannel_mesh) != nullptr),
    _tri_sch_mesh(dynamic_cast<const TriSubChannelMesh *>(&_subchannel_mesh)),
    _quad_sch_mesh(dynamic_cast<const QuadSubChannelMesh *>(&_subchannel_mesh))
{
}

Real
SCMMixingKimAndChung::computeMixingParameter(const unsigned int & i_gap,
                                             const unsigned int & iz) const
{
  if (_is_tri_lattice)
    return computeTriLatticeMixingParameter(i_gap, iz);
  else
    return computeQuadLatticeMixingParameter(i_gap, iz);
}

Real
SCMMixingKimAndChung::computeTriLatticeMixingParameter(const unsigned int & i_gap,
                                                       const unsigned int & iz) const
{
  auto S_soln = SolutionHandle(_subproblem.getVariable(0, "S"));
  auto mdot_soln = SolutionHandle(_subproblem.getVariable(0, "mdot"));
  auto w_perim_soln = SolutionHandle(_subproblem.getVariable(0, "w_perim"));
  auto mu_soln = SolutionHandle(_subproblem.getVariable(0, "mu"));
  auto P_soln = SolutionHandle(_subproblem.getVariable(0, "P"));
  auto T_soln = SolutionHandle(_subproblem.getVariable(0, "T"));
  auto P_out = _scm_problem->_P_out;
  auto fp = _scm_problem->_fp;
  const Real & pitch = _tri_sch_mesh->getPitch();
  const Real & pin_diameter = _tri_sch_mesh->getPinDiameter();
  auto chans = _tri_sch_mesh->getGapChannels(i_gap);
  unsigned int i_ch = chans.first;
  unsigned int j_ch = chans.second;
  auto * node_in_i = _tri_sch_mesh->getChannelNode(i_ch, iz - 1);
  auto * node_out_i = _tri_sch_mesh->getChannelNode(i_ch, iz);
  auto * node_in_j = _tri_sch_mesh->getChannelNode(j_ch, iz - 1);
  auto * node_out_j = _tri_sch_mesh->getChannelNode(j_ch, iz);
  auto Si_in = (S_soln)(node_in_i);
  auto Sj_in = (S_soln)(node_in_j);
  auto Si_out = (S_soln)(node_out_i);
  auto Sj_out = (S_soln)(node_out_j);
  // crossflow area between channels i,j (dz*gap_width)
  auto gap = _tri_sch_mesh->getGapWidth(iz, i_gap);
  auto avg_massflux =
      0.5 * (((mdot_soln)(node_in_i) + (mdot_soln)(node_in_j)) / (Si_in + Sj_in) +
             ((mdot_soln)(node_out_i) + (mdot_soln)(node_out_j)) / (Si_out + Sj_out));
  auto S_total = Si_in + Sj_in + Si_out + Sj_out;
  auto Si = 0.5 * (Si_in + Si_out);
  auto Sj = 0.5 * (Sj_in + Sj_out);
  auto w_perim_i = 0.5 * ((w_perim_soln)(node_in_i) + (w_perim_soln)(node_out_i));
  auto w_perim_j = 0.5 * ((w_perim_soln)(node_in_j) + (w_perim_soln)(node_out_j));
  auto avg_mu = (1 / S_total) * ((mu_soln)(node_out_i)*Si_out + (mu_soln)(node_in_i)*Si_in +
                                 (mu_soln)(node_out_j)*Sj_out + (mu_soln)(node_in_j)*Sj_in);
  auto avg_hD = 4.0 * (Si + Sj) / (w_perim_i + w_perim_j);
  auto Re = avg_massflux * avg_hD / avg_mu;
  Real gamma = 20.0;   // empirical constant
  Real sf = 2.0 / 3.0; // shape factor
  Real a = 0.18;
  Real b = 0.2;
  // _friction_args = FrictionStruct(i_ch, Re, S, w_perim);
  // Real f_darcy = _scm_problem->_friction_closure->computeFrictionFactor(friction_args) / 8.0;
  auto f = a * std::pow(Re, -b); // Rehme 1992 circular tube friction factor
  auto k =
      (1 / S_total) * (fp->k_from_p_T((P_soln)(node_out_i) + P_out, (T_soln)(node_out_i)) * Si_out +
                       fp->k_from_p_T((P_soln)(node_in_i) + P_out, (T_soln)(node_in_i)) * Si_in +
                       fp->k_from_p_T((P_soln)(node_out_j) + P_out, (T_soln)(node_out_j)) * Sj_out +
                       fp->k_from_p_T((P_soln)(node_in_j) + P_out, (T_soln)(node_in_j)) * Sj_in);
  auto cp = (1 / S_total) *
            (fp->cp_from_p_T((P_soln)(node_out_i) + P_out, (T_soln)(node_out_i)) * Si_out +
             fp->cp_from_p_T((P_soln)(node_in_i) + P_out, (T_soln)(node_in_i)) * Si_in +
             fp->cp_from_p_T((P_soln)(node_out_j) + P_out, (T_soln)(node_out_j)) * Sj_out +
             fp->cp_from_p_T((P_soln)(node_in_j) + P_out, (T_soln)(node_in_j)) * Sj_in);
  auto Pr = avg_mu * cp / k;                          // Prandtl number
  auto Pr_t = Pr * (Re / gamma) * std::sqrt(f / 8.0); // Turbulent Prandtl number
  auto delta = pitch / sqrt(3.0);                     // centroid to centroid distance
  auto L_x = sf * delta;  // axial length scale (gap is the lateral length scale)
  auto lamda = gap / L_x; // aspect ratio
  auto a_x = 1.0 - 2.0 * lamda * lamda / libMesh::pi; // velocity coefficient
  auto z_FP_over_D = (2.0 * L_x / pin_diameter) *
                     (1 + (-0.5 * std::log(lamda) + 0.5 * std::log(4.0) - 0.25) * lamda * lamda);
  auto Str = 1.0 / (0.822 * (gap / pin_diameter) + 0.144); // Strouhal number (Wu & Trupp 1994)
  auto freq_factor = 2.0 / Utility::pow<2>(gamma) * std::sqrt(a / 8.0) * (avg_hD / gap);
  auto rod_mixing = (1 / Pr_t) * lamda;
  auto axial_mixing = a_x * z_FP_over_D * Str;
  // Mixing Stanton number: Stg (eq 25,Kim and Chung (2001), eq 19 (Jeong et. al 2005)
  return freq_factor * (rod_mixing + axial_mixing) * std::pow(Re, -b / 2.0);
}

Real
SCMMixingKimAndChung::computeQuadLatticeMixingParameter(const unsigned int & i_gap,
                                                        const unsigned int & iz) const
{
  auto S_soln = SolutionHandle(_subproblem.getVariable(0, "S"));
  auto mdot_soln = SolutionHandle(_subproblem.getVariable(0, "mdot"));
  auto w_perim_soln = SolutionHandle(_subproblem.getVariable(0, "w_perim"));
  auto mu_soln = SolutionHandle(_subproblem.getVariable(0, "mu"));
  auto P_soln = SolutionHandle(_subproblem.getVariable(0, "P"));
  auto T_soln = SolutionHandle(_subproblem.getVariable(0, "T"));
  auto P_out = _scm_problem->_P_out;
  auto fp = _scm_problem->_fp;
  const Real & pitch = _quad_sch_mesh->getPitch();
  const Real & pin_diameter = _quad_sch_mesh->getPinDiameter();
  auto chans = _quad_sch_mesh->getGapChannels(i_gap);
  unsigned int i_ch = chans.first;
  unsigned int j_ch = chans.second;
  auto * node_in_i = _quad_sch_mesh->getChannelNode(i_ch, iz - 1);
  auto * node_out_i = _quad_sch_mesh->getChannelNode(i_ch, iz);
  auto * node_in_j = _quad_sch_mesh->getChannelNode(j_ch, iz - 1);
  auto * node_out_j = _quad_sch_mesh->getChannelNode(j_ch, iz);
  auto Si_in = (S_soln)(node_in_i);
  auto Sj_in = (S_soln)(node_in_j);
  auto Si_out = (S_soln)(node_out_i);
  auto Sj_out = (S_soln)(node_out_j);
  // crossflow area between channels i,j (dz*gap_width)
  auto gap = _quad_sch_mesh->getGapWidth(iz, i_gap);
  auto avg_massflux =
      0.5 * (((mdot_soln)(node_in_i) + (mdot_soln)(node_in_j)) / (Si_in + Sj_in) +
             ((mdot_soln)(node_out_i) + (mdot_soln)(node_out_j)) / (Si_out + Sj_out));
  auto S_total = Si_in + Sj_in + Si_out + Sj_out;
  auto Si = 0.5 * (Si_in + Si_out);
  auto Sj = 0.5 * (Sj_in + Sj_out);
  auto w_perim_i = 0.5 * ((w_perim_soln)(node_in_i) + (w_perim_soln)(node_out_i));
  auto w_perim_j = 0.5 * ((w_perim_soln)(node_in_j) + (w_perim_soln)(node_out_j));
  auto avg_mu = (1 / S_total) * ((mu_soln)(node_out_i)*Si_out + (mu_soln)(node_in_i)*Si_in +
                                 (mu_soln)(node_out_j)*Sj_out + (mu_soln)(node_in_j)*Sj_in);
  auto avg_hD = 4.0 * (Si + Sj) / (w_perim_i + w_perim_j);
  auto Re = avg_massflux * avg_hD / avg_mu;
  Real gamma = 20.0; // empirical constant
  Real sf = 1.0;     // shape factor
  Real a = 0.18;
  Real b = 0.2;
  // _friction_args = FrictionStruct(i_ch, Re, S, w_perim);
  // Real f_darcy = _scm_problem->_friction_closure->computeFrictionFactor(friction_args) / 8.0;
  auto f = a * std::pow(Re, -b); // Rehme 1992 circular tube friction factor
  auto k =
      (1 / S_total) * (fp->k_from_p_T((P_soln)(node_out_i) + P_out, (T_soln)(node_out_i)) * Si_out +
                       fp->k_from_p_T((P_soln)(node_in_i) + P_out, (T_soln)(node_in_i)) * Si_in +
                       fp->k_from_p_T((P_soln)(node_out_j) + P_out, (T_soln)(node_out_j)) * Sj_out +
                       fp->k_from_p_T((P_soln)(node_in_j) + P_out, (T_soln)(node_in_j)) * Sj_in);
  auto cp = (1 / S_total) *
            (fp->cp_from_p_T((P_soln)(node_out_i) + P_out, (T_soln)(node_out_i)) * Si_out +
             fp->cp_from_p_T((P_soln)(node_in_i) + P_out, (T_soln)(node_in_i)) * Si_in +
             fp->cp_from_p_T((P_soln)(node_out_j) + P_out, (T_soln)(node_out_j)) * Sj_out +
             fp->cp_from_p_T((P_soln)(node_in_j) + P_out, (T_soln)(node_in_j)) * Sj_in);
  auto Pr = avg_mu * cp / k;                          // Prandtl number
  auto Pr_t = Pr * (Re / gamma) * std::sqrt(f / 8.0); // Turbulent Prandtl number
  auto delta = pitch;                                 // centroid to centroid distance
  auto L_x = sf * delta;  // axial length scale (gap is the lateral length scale)
  auto lamda = gap / L_x; // aspect ratio
  auto a_x = 1.0 - 2.0 * lamda * lamda / libMesh::pi; // velocity coefficient
  auto z_FP_over_D = (2.0 * L_x / pin_diameter) *
                     (1 + (-0.5 * std::log(lamda) + 0.5 * std::log(4.0) - 0.25) * lamda * lamda);
  auto Str = 1.0 / (0.822 * (gap / pin_diameter) + 0.144); // Strouhal number (Wu & Trupp 1994)
  auto freq_factor = 2.0 / Utility::pow<2>(gamma) * std::sqrt(a / 8.0) * (avg_hD / gap);
  auto rod_mixing = (1 / Pr_t) * lamda;
  auto axial_mixing = a_x * z_FP_over_D * Str;
  // Mixing Stanton number: Stg (eq 25,Kim and Chung (2001), eq 19 (Jeong et. al 2005)
  return freq_factor * (rod_mixing + axial_mixing) * std::pow(Re, -b / 2.0);
}
