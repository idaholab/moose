//* This file is part of the MOOSE framework
//* https://www.mooseframework.org
//*
//* All rights reserved, see COPYRIGHT for full restrictions
//* https://github.com/idaholab/moose/blob/master/COPYRIGHT
//*
//* Licensed under LGPL 2.1, please see LICENSE for details
//* https://www.gnu.org/licenses/lgpl-2.1.html

#include "TriInterWrapper1PhaseProblem.h"
#include "AuxiliarySystem.h"
#include "TriInterWrapperMesh.h"

registerMooseObject("SubChannelApp", TriInterWrapper1PhaseProblem);

InputParameters
TriInterWrapper1PhaseProblem::validParams()
{
  InputParameters params = InterWrapper1PhaseProblem::validParams();
  params.addClassDescription(
      "Solver class for interwrapper of assemblies in a triangular-lattice arrangement");
  return params;
}

TriInterWrapper1PhaseProblem::TriInterWrapper1PhaseProblem(const InputParameters & params)
  : InterWrapper1PhaseProblem(params),
    _tri_sch_mesh(dynamic_cast<TriInterWrapperMesh &>(_subchannel_mesh))
{
}

double
TriInterWrapper1PhaseProblem::computeFrictionFactor(Real Re)
{
  Real a, b;
  if (Re < 1)
  {
    return 64.0;
  }
  else if (Re >= 1 and Re < 5000)
  {
    a = 64.0;
    b = -1.0;
  }
  else if (Re >= 5000 and Re < 30000)
  {
    a = 0.316;
    b = -0.25;
  }
  else
  {
    a = 0.184;
    b = -0.20;
  }
  return a * std::pow(Re, b);
}

void
TriInterWrapper1PhaseProblem::computeDP(int iblock)
{
  unsigned int last_node = (iblock + 1) * _block_size;
  unsigned int first_node = iblock * _block_size + 1;

  for (unsigned int iz = first_node; iz < last_node + 1; iz++)
  {
    auto z_grid = _subchannel_mesh.getZGrid();
    auto k_grid = _subchannel_mesh.getKGrid();
    auto dz = z_grid[iz] - z_grid[iz - 1];
    for (unsigned int i_ch = 0; i_ch < _n_channels; i_ch++)
    {
      auto * node_in = _subchannel_mesh.getChannelNode(i_ch, iz - 1);
      auto * node_out = _subchannel_mesh.getChannelNode(i_ch, iz);
      auto rho_in = (*_rho_soln)(node_in);
      auto rho_out = (*_rho_soln)(node_out);
      auto mu_in = (*_mu_soln)(node_in);
      auto S = (*_S_flow_soln)(node_in);
      auto w_perim = (*_w_perim_soln)(node_in);
      // hydraulic diameter in the i direction
      auto Dh_i = 4.0 * S / w_perim;
      auto time_term =
          _TR * ((*_mdot_soln)(node_out)-_mdot_soln->old(node_out)) * dz / _dt -
          dz * 2.0 * (*_mdot_soln)(node_out) * (rho_out - _rho_soln->old(node_out)) / rho_in / _dt;

      auto mass_term1 =
          std::pow((*_mdot_soln)(node_out), 2.0) * (1.0 / S / rho_out - 1.0 / S / rho_in);
      auto mass_term2 = -2.0 * (*_mdot_soln)(node_out) * (*_SumWij_soln)(node_out) / S / rho_in;

      auto crossflow_term = 0.0;
      auto turbulent_term = 0.0;

      unsigned int counter = 0;
      for (auto i_gap : _subchannel_mesh.getChannelGaps(i_ch))
      {
        auto chans = _subchannel_mesh.getGapChannels(i_gap);
        unsigned int ii_ch = chans.first;
        unsigned int jj_ch = chans.second;
        auto * node_in_i = _subchannel_mesh.getChannelNode(ii_ch, iz - 1);
        auto * node_in_j = _subchannel_mesh.getChannelNode(jj_ch, iz - 1);
        auto * node_out_i = _subchannel_mesh.getChannelNode(ii_ch, iz);
        auto * node_out_j = _subchannel_mesh.getChannelNode(jj_ch, iz);
        auto rho_i = (*_rho_soln)(node_in_i);
        auto rho_j = (*_rho_soln)(node_in_j);
        auto Si = (*_S_flow_soln)(node_in_i);
        auto Sj = (*_S_flow_soln)(node_in_j);
        auto u_star = 0.0;
        // figure out donor axial velocity
        if (_Wij(i_gap, iz) > 0.0)
          u_star = (*_mdot_soln)(node_out_i) / Si / rho_i;
        else
          u_star = (*_mdot_soln)(node_out_j) / Sj / rho_j;

        crossflow_term +=
            _subchannel_mesh.getCrossflowSign(i_ch, counter) * _Wij(i_gap, iz) * u_star;

        turbulent_term += _WijPrime(i_gap, iz) * (2 * (*_mdot_soln)(node_out) / rho_in / S -
                                                  (*_mdot_soln)(node_out_j) / Sj / rho_j -
                                                  (*_mdot_soln)(node_out_i) / Si / rho_i);
        counter++;
      }
      turbulent_term *= _CT;

      auto Re = (((*_mdot_soln)(node_in) / S) * Dh_i / mu_in);
      auto fi = computeFrictionFactor(Re);
      auto ki = k_grid[i_ch][iz - 1];
      auto friction_term = (fi * dz / Dh_i + ki) * 0.5 * (std::pow((*_mdot_soln)(node_out), 2.0)) /
                           (S * (*_rho_soln)(node_out));
      auto gravity_term = _g_grav * (*_rho_soln)(node_out)*dz * S;
      auto DP = std::pow(S, -1.0) * (time_term + mass_term1 + mass_term2 + crossflow_term +
                                     turbulent_term + friction_term + gravity_term); // Pa
      _DP_soln->set(node_out, DP);
    }
  }
}

double
TriInterWrapper1PhaseProblem::computeMassFlowForDPDZ(Real dpdz, int i_ch)
{
  auto * node = _subchannel_mesh.getChannelNode(i_ch, 0);
  // initialize massflow
  auto massflow = (*_mdot_soln)(node);
  auto rho = (*_rho_soln)(node);
  auto T = (*_T_soln)(node);
  auto mu = _fp->mu_from_rho_T(rho, T);
  auto Si = (*_S_flow_soln)(node);
  auto w_perim = (*_w_perim_soln)(node);
  auto Dhi = 4.0 * Si / w_perim;
  auto max_iter = 10;
  auto TOL = 1e-6;
  // Iterate until we find massflow that matches the given dp/dz.
  auto iter = 0;
  auto error = 1.0;
  while (error > TOL)
  {
    iter += 1;
    if (iter > max_iter)
      mooseError(name(), ": exceeded maximum number of iterations");

    auto massflow_old = massflow;
    auto Rei = ((massflow / Si) * Dhi / mu);
    auto fi = computeFrictionFactor(Rei);
    massflow = std::sqrt(2.0 * Dhi * dpdz * rho * std::pow(Si, 2.0) / fi);
    error = std::abs((massflow - massflow_old) / massflow_old);
  }
  return massflow;
}

void
TriInterWrapper1PhaseProblem::enforceUniformDPDZAtInlet()
{
  _console
      << "Edit mass flow boundary condition in order to have uniform Pressure drop at the inlet\n";
  auto total_mass_flow = 0.0;
  for (unsigned int i_ch = 0; i_ch < _subchannel_mesh.getNumOfChannels(); i_ch++)
  {
    auto * node_in = _subchannel_mesh.getChannelNode(i_ch, 0);
    total_mass_flow += (*_mdot_soln)(node_in);
  }
  _console << "Total mass flow :" << total_mass_flow << " [kg/sec] \n";
  // Define vectors of pressure drop and massflow
  Eigen::VectorXd dPdZ_i(_subchannel_mesh.getNumOfChannels());
  Eigen::VectorXd mass_flow_i(_subchannel_mesh.getNumOfChannels());
  // Calculate Pressure drop derivative for current mass flow BC
  for (unsigned int i_ch = 0; i_ch < _subchannel_mesh.getNumOfChannels(); i_ch++)
  {
    auto * node_in = _subchannel_mesh.getChannelNode(i_ch, 0);
    auto rho_in = (*_rho_soln)(node_in);
    auto T_in = (*_T_soln)(node_in);
    auto Si = (*_S_flow_soln)(node_in);
    auto w_perim = (*_w_perim_soln)(node_in);
    auto Dhi = 4.0 * Si / w_perim;
    auto mu = _fp->mu_from_rho_T(rho_in, T_in);
    auto Rei = (((*_mdot_soln)(node_in) / Si) * Dhi / mu);
    auto fi = computeFrictionFactor(Rei);
    dPdZ_i(i_ch) =
        (fi / Dhi) * 0.5 * (std::pow((*_mdot_soln)(node_in), 2.0)) / (std::pow(Si, 2.0) * rho_in);
  }

  // Initialize an average pressure drop for uniform pressure inlet condition
  auto dPdZ = dPdZ_i.mean();
  auto error = 1.0;
  auto tol = 1e-6;
  auto iter = 0;
  auto max_iter = 10;
  while (error > tol)
  {
    iter += 1;
    if (iter > max_iter)
      mooseError(name(), ": exceeded maximum number of iterations");
    auto dPdZ_old = dPdZ;
    for (unsigned int i_ch = 0; i_ch < _subchannel_mesh.getNumOfChannels(); i_ch++)
    {
      // update inlet mass flow to achieve DPDZ
      mass_flow_i(i_ch) = computeMassFlowForDPDZ(dPdZ, i_ch);
    }
    // Calculate total massflow at the inlet
    auto mass_flow_sum = mass_flow_i.sum();
    // Update the DP/DZ to correct the mass flow rate.
    dPdZ *= std::pow(total_mass_flow / mass_flow_sum, 2.0);
    error = std::abs((dPdZ - dPdZ_old) / dPdZ_old);
  }

  // Populate solution vector with corrected boundary conditions
  for (unsigned int i_ch = 0; i_ch < _subchannel_mesh.getNumOfChannels(); i_ch++)
  {
    auto * node = _subchannel_mesh.getChannelNode(i_ch, 0);
    _mdot_soln->set(node, mass_flow_i(i_ch));
  }
  _console << "Done applying mass flow boundary condition\n";
}

void
TriInterWrapper1PhaseProblem::computeInletMassFlowDist()
{
  auto mass_flow = 0.0;
  // inlet pressure
  Eigen::VectorXd p_in(_subchannel_mesh.getNumOfChannels());
  // outlet pressure
  Eigen::VectorXd p_out(_subchannel_mesh.getNumOfChannels());
  // pressure drop
  Eigen::VectorXd dpz(_subchannel_mesh.getNumOfChannels());
  // inlet mass flow rate
  Eigen::VectorXd mass_flow_i(_subchannel_mesh.getNumOfChannels());
  // inlet mass flow flux
  Eigen::VectorXd mass_flux_i(_subchannel_mesh.getNumOfChannels());
  // new/corrected mass flux
  Eigen::VectorXd g_new(_subchannel_mesh.getNumOfChannels());
  // flow area
  Eigen::VectorXd Si(_subchannel_mesh.getNumOfChannels());
  // total number of subchannels
  auto tot_chan = _subchannel_mesh.getNumOfChannels();
  // average pressure
  auto dpz_ave = 0.0;
  // summmation for the pressure drop over all subchannels
  auto dpzsum = 0.0;
  // new total mass flow rate - used for correction
  auto mass_flow_new_tot = 0.0;
  // number of axial nodes
  unsigned int nz = _subchannel_mesh.getNumOfAxialCells();

  for (unsigned int i_ch = 0; i_ch < _subchannel_mesh.getNumOfChannels(); i_ch++)
  {
    auto * node = _subchannel_mesh.getChannelNode(i_ch, 0);
    auto * node_out = _subchannel_mesh.getChannelNode(i_ch, nz);
    Si(i_ch) = (*_S_flow_soln)(node);
    mass_flow_i(i_ch) = (*_mdot_soln)(node);
    mass_flux_i(i_ch) = mass_flow_i(i_ch) / Si(i_ch);
    p_in(i_ch) = (*_P_soln)(node);
    p_out(i_ch) = (*_P_soln)(node_out);
    dpz(i_ch) = p_in(i_ch) - p_out(i_ch);
    if (dpz(i_ch) <= 0.0)
    {
      mooseError(
          name(), " Computed presurre drop at the following subchannel is less than zero. ", i_ch);
    }
    dpzsum = dpzsum + dpz(i_ch);
    mass_flow = mass_flow + mass_flow_i(i_ch);
  }
  dpz_ave = dpzsum / tot_chan;
  _dpz_error = 0.0;
  for (unsigned int i_ch = 0; i_ch < _subchannel_mesh.getNumOfChannels(); i_ch++)
  {
    g_new(i_ch) = std::pow(dpz_ave / dpz(i_ch), 0.3) * mass_flux_i(i_ch);
    mass_flow_new_tot = mass_flow_new_tot + g_new(i_ch) * Si(i_ch);
    _dpz_error = _dpz_error + std::pow((dpz(i_ch) - dpz_ave), 2.0);
  }
  _dpz_error = std::pow(_dpz_error, 0.5) / dpz_ave / tot_chan;
  for (unsigned int i_ch = 0; i_ch < _subchannel_mesh.getNumOfChannels(); i_ch++)
  {
    auto * node = _subchannel_mesh.getChannelNode(i_ch, 0);
    g_new(i_ch) = g_new(i_ch) * mass_flow / mass_flow_new_tot;
    _mdot_soln->set(node, g_new(i_ch) * Si(i_ch));
  }
}

void
TriInterWrapper1PhaseProblem::computeh(int iblock)
{
  unsigned int last_node = (iblock + 1) * _block_size;
  unsigned int first_node = iblock * _block_size + 1;

  if (iblock == 0)
  {
    for (unsigned int i_ch = 0; i_ch < _n_channels; i_ch++)
    {
      auto * node = _subchannel_mesh.getChannelNode(i_ch, 0);
      _h_soln->set(node, _fp->h_from_p_T((*_P_soln)(node) + _P_out, (*_T_soln)(node)));
    }
  }

  for (unsigned int iz = first_node; iz < last_node + 1; iz++)
  {
    auto z_grid = _subchannel_mesh.getZGrid();
    auto dz = z_grid[iz] - z_grid[iz - 1];
    auto heated_length = _subchannel_mesh.getHeatedLength();
    auto unheated_length_entry = _subchannel_mesh.getHeatedLengthEntry();

    Real gedge_ave = 0.0;
    Real mdot_sum = 0.0;
    Real si_sum = 0.0;
    for (unsigned int i_ch = 0; i_ch < _n_channels; i_ch++)
    {
      auto subch_type = _subchannel_mesh.getSubchannelType(i_ch);
      if (subch_type == EChannelType::EDGE || subch_type == EChannelType::CORNER)
      {
        auto * node_in = _subchannel_mesh.getChannelNode(i_ch, iz - 1);
        auto Si = (*_S_flow_soln)(node_in);
        auto mdot_in = (*_mdot_soln)(node_in);
        mdot_sum = mdot_sum + mdot_in;
        si_sum = si_sum + Si;
      }
    }
    gedge_ave = mdot_sum / si_sum;

    for (unsigned int i_ch = 0; i_ch < _n_channels; i_ch++)
    {
      const Real & pitch = _subchannel_mesh.getPitch();
      const Real & assembly_diameter = _subchannel_mesh.getSideX();
      auto * node_in = _subchannel_mesh.getChannelNode(i_ch, iz - 1);
      auto * node_out = _subchannel_mesh.getChannelNode(i_ch, iz);
      auto mdot_in = (*_mdot_soln)(node_in);
      auto h_in = (*_h_soln)(node_in); // J/kg
      auto volume = dz * (*_S_flow_soln)(node_in);
      auto mdot_out = (*_mdot_soln)(node_out);
      auto h_out = 0.0;
      Real sumWijh = 0.0;
      Real sumWijPrimeDhij = 0.0;
      Real e_cond = 0.0;

      Real added_enthalpy;
      if (z_grid[iz] > unheated_length_entry && z_grid[iz] <= unheated_length_entry + heated_length)
        added_enthalpy = ((*_q_prime_soln)(node_out) + (*_q_prime_soln)(node_in)) * dz / 2.0;
      else
        added_enthalpy = 0.0;

      // compute the sweep flow enthalpy change
      auto subch_type = _subchannel_mesh.getSubchannelType(i_ch);
      Real sweep_enthalpy = 0.0;

      if (subch_type == EChannelType::EDGE || subch_type == EChannelType::CORNER)
      {
        const Real & pitch = _subchannel_mesh.getPitch();
        const Real & assembly_diameter = _subchannel_mesh.getSideX();
        const Real & wire_lead_length = 0.0;
        const Real & wire_diameter = 0.0;
        auto gap = _tri_sch_mesh.getDuctToRodGap();
        auto w = assembly_diameter + gap;
        auto theta =
            std::acos(wire_lead_length /
                      std::sqrt(std::pow(wire_lead_length, 2) +
                                std::pow(libMesh::pi * (assembly_diameter + wire_diameter), 2)));
        // in/out channels for i_ch
        auto sweep_in = _tri_sch_mesh.getSweepFlowChans(i_ch).first;
        auto * node_sin = _subchannel_mesh.getChannelNode(sweep_in, iz - 1);
        auto cs_t = 0.75 * std::pow(wire_lead_length / assembly_diameter, 0.3);
        auto ar2 = libMesh::pi * (assembly_diameter + wire_diameter) * wire_diameter / 4.0;
        auto a2p = pitch * (w - assembly_diameter / 2.0) -
                   libMesh::pi * std::pow(assembly_diameter, 2) / 8.0;
        auto Sij_in = dz * gap;
        auto Sij_out = dz * gap;
        auto wsweep_in = gedge_ave * cs_t * std::pow((ar2 / a2p), 0.5) * std::tan(theta) * Sij_in;
        auto wsweep_out = gedge_ave * cs_t * std::pow((ar2 / a2p), 0.5) * std::tan(theta) * Sij_out;
        auto sweep_hin = (*_h_soln)(node_sin);
        auto sweep_hout = (*_h_soln)(node_in);
        sweep_enthalpy = (wsweep_in * sweep_hin - wsweep_out * sweep_hout);
      }

      // Calculate sum of crossflow into channel i from channels j around i
      unsigned int counter = 0;
      for (auto i_gap : _subchannel_mesh.getChannelGaps(i_ch))
      {
        auto chans = _subchannel_mesh.getGapChannels(i_gap);
        unsigned int ii_ch = chans.first;
        // i is always the smallest and first index in the mapping
        unsigned int jj_ch = chans.second;
        auto * node_in_i = _subchannel_mesh.getChannelNode(ii_ch, iz - 1);
        auto * node_in_j = _subchannel_mesh.getChannelNode(jj_ch, iz - 1);
        // Define donor enthalpy
        auto h_star = 0.0;
        if (_Wij(i_gap, iz) > 0.0)
          h_star = (*_h_soln)(node_in_i);
        else if (_Wij(i_gap, iz) < 0.0)
          h_star = (*_h_soln)(node_in_j);
        // take care of the sign by applying the map, use donor cell
        sumWijh += _subchannel_mesh.getCrossflowSign(i_ch, counter) * _Wij(i_gap, iz) * h_star;
        sumWijPrimeDhij += _WijPrime(i_gap, iz) * (2 * (*_h_soln)(node_in) - (*_h_soln)(node_in_j) -
                                                   (*_h_soln)(node_in_i));
        counter++;

        // compute the radial heat conduction through the gaps

        // compute the radial heat conduction through gaps
        auto subch_type_i = _subchannel_mesh.getSubchannelType(ii_ch);
        auto subch_type_j = _subchannel_mesh.getSubchannelType(jj_ch);
        Real dist_ij = pitch;

        if (subch_type_i == EChannelType::EDGE && subch_type_j == EChannelType::EDGE)
        {
          dist_ij = pitch;
        }
        else if ((subch_type_i == EChannelType::CORNER && subch_type_j == EChannelType::EDGE) ||
                 (subch_type_i == EChannelType::EDGE && subch_type_j == EChannelType::CORNER))
        {
          dist_ij = pitch;
        }
        else
        {
          dist_ij = pitch / std::sqrt(3);
        }

        auto Sij = dz * _subchannel_mesh.getGapWidth(i_gap);
        auto thcon_i = _fp->k_from_p_T((*_P_soln)(node_in_i), (*_T_soln)(node_in_i));
        auto thcon_j = _fp->k_from_p_T((*_P_soln)(node_in_j), (*_T_soln)(node_in_j));
        auto shape_factor =
            0.66 * (pitch / assembly_diameter) *
            std::pow((_subchannel_mesh.getGapWidth(i_gap) / assembly_diameter), -0.3);
        if (ii_ch == i_ch)
        {
          e_cond += 0.5 * (thcon_i + thcon_j) * Sij * shape_factor *
                    ((*_T_soln)(node_in_j) - (*_T_soln)(node_in_i)) / dist_ij;
        }
        else
        {
          e_cond += -0.5 * (thcon_i + thcon_j) * Sij * shape_factor *
                    ((*_T_soln)(node_in_j) - (*_T_soln)(node_in_i)) / dist_ij;
        }
      }

      // compute the axial heat conduction between current and lower axial node
      auto * node_in_i = _subchannel_mesh.getChannelNode(i_ch, iz);
      auto * node_in_j = _subchannel_mesh.getChannelNode(i_ch, iz - 1);
      auto thcon_i = _fp->k_from_p_T((*_P_soln)(node_in_i), (*_T_soln)(node_in_i));
      auto thcon_j = _fp->k_from_p_T((*_P_soln)(node_in_j), (*_T_soln)(node_in_j));
      auto Si = (*_S_flow_soln)(node_in_i);
      auto dist_ij = z_grid[iz] - z_grid[iz - 1];

      e_cond += 0.5 * (thcon_i + thcon_j) * Si * ((*_T_soln)(node_in_j) - (*_T_soln)(node_in_i)) /
                dist_ij;

      unsigned int nz = _subchannel_mesh.getNumOfAxialCells();
      // compute the axial heat conduction between current and upper axial node
      if (iz < nz)
      {

        auto * node_in_i = _subchannel_mesh.getChannelNode(i_ch, iz);
        auto * node_in_j = _subchannel_mesh.getChannelNode(i_ch, iz + 1);
        auto thcon_i = _fp->k_from_p_T((*_P_soln)(node_in_i), (*_T_soln)(node_in_i));
        auto thcon_j = _fp->k_from_p_T((*_P_soln)(node_in_j), (*_T_soln)(node_in_j));
        auto Si = (*_S_flow_soln)(node_in_i);
        auto dist_ij = z_grid[iz + 1] - z_grid[iz];
        e_cond += 0.5 * (thcon_i + thcon_j) * Si * ((*_T_soln)(node_in_j) - (*_T_soln)(node_in_i)) /
                  dist_ij;
      }

      // end of radial heat conduction calc.

      h_out =
          (mdot_in * h_in - sumWijh - sumWijPrimeDhij + added_enthalpy + e_cond + sweep_enthalpy +
           _TR * _rho_soln->old(node_out) * _h_soln->old(node_out) * volume / _dt) /
          (mdot_out + _TR * (*_rho_soln)(node_out)*volume / _dt);

      if (h_out < 0)
      {
        mooseWarning(name(),
                     " : Calculation of negative Enthalpy h_out = : ",
                     h_out,
                     " Axial Level= : ",
                     iz);
      }
      _h_soln->set(node_out, h_out); // J/kg
    }
  }
}

void
TriInterWrapper1PhaseProblem::externalSolve()
{
  initializeSolution();
  _console << "Executing subchannel solver\n";
  auto P_error = 1.0;
  unsigned int P_it = 0;
  unsigned int P_it_max = 2 * _n_blocks;
  if (_n_blocks == 1)
    P_it_max = 1;
  while (P_error > _P_tol && P_it < P_it_max)
  {
    P_it += 1;
    if (P_it == P_it_max and _n_blocks != 1)
    {
      _console << "Reached maximum number of axial pressure iterations" << std::endl;
      _converged = false;
    }
    _console << "Solving Outer Iteration : " << P_it << std::endl;
    auto P_L2norm_old_axial = _P_soln->L2norm();
    for (unsigned int iblock = 0; iblock < _n_blocks; iblock++)
    {
      int last_level = (iblock + 1) * _block_size;
      int first_level = iblock * _block_size + 1;
      auto T_block_error = 1.0;
      auto T_it = 0;
      _console << "Solving Block: " << iblock << " From first level: " << first_level
               << " to last level: " << last_level << std::endl;

      while (T_block_error > _T_tol && T_it < _T_maxit)
      {
        T_it += 1;
        if (T_it == _T_maxit)
        {
          _console << "Reached maximum number of temperature iterations for block: " << iblock
                   << std::endl;
          _converged = false;
        }
        auto T_L2norm_old_block = _T_soln->L2norm();

        // computeWij(iblock);
        computeWijFromSolve(iblock);

        if (_compute_power)
        {
          computeh(iblock);

          computeT(iblock);
        }

        if (_compute_density)
          computeRho(iblock);

        if (_compute_viscosity)
          computeMu(iblock);

        // We must do a global assembly to make sure data is parallel consistent before we do things
        // like compute L2 norms
        _aux->solution().close();

        auto T_L2norm_new = _T_soln->L2norm();
        T_block_error =
            std::abs((T_L2norm_new - T_L2norm_old_block) / (T_L2norm_old_block + 1E-14));
        _console << "T_block_error: " << T_block_error << std::endl;
      }
    }
    auto P_L2norm_new_axial = _P_soln->L2norm();
    P_error =
        std::abs((P_L2norm_new_axial - P_L2norm_old_axial) / (P_L2norm_old_axial + _P_out + 1E-14));
    _console << "P_error :" << P_error << std::endl;
  }
  // update old crossflow matrix
  _Wij_old = _Wij;
  _console << "Finished executing subchannel solver\n";
  _aux->solution().close();

  auto power_in = 0.0;
  auto power_out = 0.0;
  for (unsigned int i_ch = 0; i_ch < _n_channels; i_ch++)
  {
    auto * node_in = _subchannel_mesh.getChannelNode(i_ch, 0);
    auto * node_out = _subchannel_mesh.getChannelNode(i_ch, _n_cells);
    power_in += (*_mdot_soln)(node_in) * (*_h_soln)(node_in);
    power_out += (*_mdot_soln)(node_out) * (*_h_soln)(node_out);
  }

  /// TODO: add a verbose print flag
  auto Total_surface_area = 0.0;
  auto mass_flow_in = 0.0;
  auto mass_flow_out = 0.0;
  for (unsigned int i_ch = 0; i_ch < _n_channels; i_ch++)
  {
    auto * node_in = _subchannel_mesh.getChannelNode(i_ch, 0);
    auto * node_out = _subchannel_mesh.getChannelNode(i_ch, _n_cells);
    Total_surface_area += (*_S_flow_soln)(node_in);
    mass_flow_in += (*_mdot_soln)(node_in);
    mass_flow_out += (*_mdot_soln)(node_out);
  }
  auto h_bulk_out = power_out / mass_flow_out;
  auto T_bulk_out = _fp->T_from_p_h(_P_out, h_bulk_out);

  _console << " ======================================= " << std::endl;
  _console << " ======== Subchannel Print Outs ======== " << std::endl;
  _console << " ======================================= " << std::endl;
  _console << "Total Surface Area :" << Total_surface_area << " m^2" << std::endl;
  _console << "Bulk coolant temperature at outlet :" << T_bulk_out << " K" << std::endl;
  _console << "Power added to coolant is: " << power_out - power_in << " Watt" << std::endl;
  _console << "Mass in: " << mass_flow_in << " kg/sec" << std::endl;
  _console << "Mass out: " << mass_flow_out << " kg/sec" << std::endl;
  _console << " ======================================= " << std::endl;

  _console << "Power added to coolant is: " << power_out - power_in << " Watt" << std::endl;
}
