#include "SubChannel1PhaseProblemBase.h"
#include "SystemBase.h"
#include "libmesh/petsc_vector.h"
#include <petscdm.h>
#include <petscdmda.h>
#include <petscksp.h>
#include <petscsys.h>
#include <petscvec.h>
#include <iostream>
#include <Eigen/Dense>
#include <cmath>
#include "SinglePhaseFluidProperties.h"
#include "SolutionHandle.h"
#include "AuxiliarySystem.h"

InputParameters
SubChannel1PhaseProblemBase::validParams()
{
  InputParameters params = ExternalProblem::validParams();
  params.addRequiredParam<Real>("abeta",
                                "Thermal diffusion coefficient used in turbulent crossflow");
  params.addRequiredParam<Real>("CT", "Turbulent modeling parameter");
  params.addRequiredParam<bool>("enforce_uniform_pressure",
                                "Flag that enables uniform inlet pressure");
  params.addRequiredParam<UserObjectName>("fp", "Fluid properties user object name");
  return params;
}

SubChannel1PhaseProblemBase::SubChannel1PhaseProblemBase(const InputParameters & params)
  : ExternalProblem(params),
    _g_grav(9.87),
    _TR(0.0),
    _dt(1.0),
    _subchannel_mesh(dynamic_cast<SubChannelMeshBase &>(_mesh)),
    _abeta(getParam<Real>("abeta")),
    _CT(getParam<Real>("CT")),
    _enforce_uniform_pressure(getParam<bool>("enforce_uniform_pressure")),
    _fp(nullptr)
{
  unsigned int nz = _subchannel_mesh.getNumOfAxialNodes();
  unsigned int n_gaps = _subchannel_mesh.getNumOfGapsPerLayer();
  // Turbulent crossflow (stuff that live on the gaps)
  Wij.resize(n_gaps, nz + 1);
  Wij_old.resize(n_gaps, nz + 1);
  WijPrime.resize(n_gaps, nz + 1);
  Wij.setZero();
  Wij_old.setZero();
  WijPrime.setZero();
}

void
SubChannel1PhaseProblemBase::initialSetup()
{
  ExternalProblem::initialSetup();
  // Read in Wij_global for null transient only at the first run of externalSolve
  if (isTransient())
  {
    _TR = 1.0;
    _dt = dt();
    std::ifstream file("Wij_SS");
    std::string line_str;
    int row_index = 0;

    while (std::getline(file, line_str))
    {
      int column_index = 0;
      std::stringstream str_strm(line_str);
      std::string tmp;
      char delim = ' '; // Define the delimiter to split by
      while (std::getline(str_strm, tmp, delim))
      {
        // Provide proper checks here for tmp like if empty
        // Also strip down symbols like !, ., ?, etc.
        // Finally push it.
        if (tmp.size() != 0)
        {
          Wij(row_index, column_index) = std::stof(tmp);
          Wij_old(row_index, column_index) = std::stof(tmp);
          column_index += 1;
        }
      }
      row_index += 1;
    }
  }

  _fp = &getUserObject<SinglePhaseFluidProperties>(getParam<UserObjectName>("fp"));
  mdot_soln = new SolutionHandle(getVariable(0, "mdot"));
  SumWij_soln = new SolutionHandle(getVariable(0, "SumWij"));
  P_soln = new SolutionHandle(getVariable(0, "P"));
  DP_soln = new SolutionHandle(getVariable(0, "DP"));
  h_soln = new SolutionHandle(getVariable(0, "h"));
  T_soln = new SolutionHandle(getVariable(0, "T"));
  rho_soln = new SolutionHandle(getVariable(0, "rho"));
  S_flow_soln = new SolutionHandle(getVariable(0, "S"));
  w_perim_soln = new SolutionHandle(getVariable(0, "w_perim"));
  q_prime_soln = new SolutionHandle(getVariable(0, "q_prime"));
}

SubChannel1PhaseProblemBase::~SubChannel1PhaseProblemBase()
{
  delete mdot_soln;
  delete SumWij_soln;
  delete P_soln;
  delete DP_soln;
  delete h_soln;
  delete T_soln;
  delete rho_soln;
  delete S_flow_soln;
  delete w_perim_soln;
  delete q_prime_soln;
}

bool
SubChannel1PhaseProblemBase::converged()
{
  return true;
}

void
SubChannel1PhaseProblemBase::computeWij(int iz)
{
  if (iz == 0)
  {
    mooseError(name(),
               ": Cannot compute crossflow quantities at the inlet of the assembly. Boundary "
               "conditions are applied here");
  }
  auto z_grid = _subchannel_mesh.getZGrid();
  unsigned int n_gaps = _subchannel_mesh.getNumOfGapsPerLayer();
  const Real & pitch = _subchannel_mesh.getPitch();
  auto dz = z_grid[iz] - z_grid[iz - 1];
  for (unsigned int i_gap = 0; i_gap < n_gaps; i_gap++)
  {
    auto chans = _subchannel_mesh.getGapNeighborChannels(i_gap);
    unsigned int i_ch = chans.first;
    unsigned int j_ch = chans.second;
    auto * node_in_i = _subchannel_mesh.getChannelNode(i_ch, iz - 1);
    auto * node_out_i = _subchannel_mesh.getChannelNode(i_ch, iz);
    auto * node_in_j = _subchannel_mesh.getChannelNode(j_ch, iz - 1);
    auto * node_out_j = _subchannel_mesh.getChannelNode(j_ch, iz);
    auto rho_i = (*rho_soln)(node_in_i);
    auto rho_j = (*rho_soln)(node_in_j);
    // area of channel i
    auto Si = (*S_flow_soln)(node_in_i);
    // area of channel j
    auto Sj = (*S_flow_soln)(node_in_j);
    // crossflow area between channels i,j dz*gap_width
    auto Sij = dz * _subchannel_mesh.getGapWidth(i_gap);
    // hydraulic diameter in the ij direction
    auto Lij = pitch;
    // total local form loss in the ij direction
    auto Kij = 481.759; // this will change
    //        2.0 *
    //        std::pow((1.0 - std::pow(Lij, 2.0) / std::pow(Lij -
    //        _subchannel_mesh._rod_diameter, 2.0)),
    //                 2.0);
    // apply lateral pressure difference damping
    auto asp = 1.0; // means no damping
    auto DPi = (*DP_soln)(node_out_i);
    auto DPj = (*DP_soln)(node_out_j);
    auto DPij_out = (*P_soln)(node_out_i) - (*P_soln)(node_out_j);
    auto DPij_in = (*P_soln)(node_in_i) - (*P_soln)(node_in_j);
    auto DPij = (1 - asp) * (DPij_out + DPi - DPj) + asp * DPij_in;

    // Figure out donor cell density
    auto rho_star = 0.0;
    if (Wij(i_gap, iz) > 0.0)
    {
      rho_star = rho_i;
    }
    else if (Wij(i_gap, iz) < 0.0)
    {
      rho_star = rho_j;
    }
    else
    {
      rho_star = (rho_i + rho_j) / 2.0;
    }

    auto Mass_Term_out =
        (*mdot_soln)(node_out_i) / Si / rho_i + (*mdot_soln)(node_out_j) / Sj / rho_j;
    auto Mass_Term_in = (*mdot_soln)(node_in_i) / Si / rho_i + (*mdot_soln)(node_in_j) / Sj / rho_j;
    auto Term_out = Sij * rho_star * (Lij / dz) * Mass_Term_out;
    auto Term_in = Sij * rho_star * (Lij / dz) * Mass_Term_in * Wij(i_gap, iz - 1);
    auto Pressure_Term = 2 * std::pow(Sij, 2.0) * DPij * rho_star;

    // Set inertia terms to zero (need to go away in the future)
    Term_out = 0.0;
    Term_in = 0.0;

    // INITIAL GUESS (eventually the continue statement will be removed)
    auto Wijguess = 0.0;
    if (Wij(i_gap, iz) == 0.0)
    {
      if (isTransient())
        Wijguess = Wij(i_gap, iz);
      else
        Wijguess = Wij(i_gap, iz - 1);
    }
    else
      continue;

    auto newton_error = 1.0;
    auto newton_tolerance = 1e-10;
    int newton_cycles = 0;
    int max_newton_cycles = 100;
    while (newton_error > newton_tolerance && newton_cycles <= max_newton_cycles)
    {
      newton_cycles++;
      if (newton_cycles == max_newton_cycles)
      {
        mooseError(
            name(), " CrossFlow Calculation didn't converge, newton_cycles: ", newton_cycles);
      }
      auto TimeTerm = _TR * 2.0 * (Wijguess - Wij_old(i_gap, iz)) * Lij * Sij * rho_star / _dt;
      auto Residual = TimeTerm + Kij * Wijguess * std::abs(Wijguess) + Term_out * Wijguess -
                      Term_in - Pressure_Term;
      auto derivative =
          _TR * 2.0 * Lij * Sij * rho_star / _dt + 2.0 * Kij * std::abs(Wijguess) + Term_out;
      Wijguess = Wijguess - Residual / (derivative + 1e-10);
      newton_error = std::abs(Residual);
    }
    // Wij has a global sign at this point (Positive is from the lower index subchannel to the
    // higher index)
    Wij(i_gap, iz) = Wijguess;
  }
}

void
SubChannel1PhaseProblemBase::computeSumWij(int iz)
{
  if (iz == 0)
  {
    mooseError(
        name(),
        ": Cannot compute sum of crossflow quantities at the inlet of the assembly. Boundary "
        "conditions are applied here");
  }
  for (unsigned int i_ch = 0; i_ch < _subchannel_mesh.getNumOfChannels(); i_ch++)
  {
    auto * node_out = _subchannel_mesh.getChannelNode(i_ch, iz);
    double SumWij = 0.0;
    // Calculate sum of crossflow into channel i from channels j around i
    unsigned int counter = 0;
    for (auto i_gap : _subchannel_mesh.getChannelGaps(i_ch))
    {
      SumWij += _subchannel_mesh.getCrossflowSign(i_ch, counter) * Wij(i_gap, iz);
      counter++;
    }
    // The net crossflow coming out of cell i [kg/sec]
    SumWij_soln->set(node_out, SumWij);
  }
}

void
SubChannel1PhaseProblemBase::computeMdot(int iz)
{
  if (iz == 0)
  {
    mooseError(name(),
               ": Cannot compute massflow at the inlet of the assembly. Boundary conditions are "
               "applied here");
  }
  auto z_grid = _subchannel_mesh.getZGrid();
  auto dz = z_grid[iz] - z_grid[iz - 1];
  // go through the channels of the level.
  for (unsigned int i_ch = 0; i_ch < _subchannel_mesh.getNumOfChannels(); i_ch++)
  {
    // Start with applying mass-conservation equation & energy - conservation equation
    // Find the nodes for the top and bottom of this element.
    auto * node_in = _subchannel_mesh.getChannelNode(i_ch, iz - 1);
    auto * node_out = _subchannel_mesh.getChannelNode(i_ch, iz);
    auto volume = dz * (*S_flow_soln)(node_in);
    // mass damping
    double am = 1.0; // means no damping
    auto TimeTerm = _TR * ((*rho_soln)(node_out)-rho_soln->old(node_out)) * volume / _dt;
    // Wij positive out of i into j;
    auto mdot_out = am * ((*mdot_soln)(node_in) - (*SumWij_soln)(node_out)-TimeTerm) +
                    (1.0 - am) * (*mdot_soln)(node_out);
    if (mdot_out < 0)
    {
      _console << "Wij = : " << Wij << "\n";
      mooseError(name(),
                 " : Calculation of negative mass flow mdot_out = : ",
                 mdot_out,
                 " Axial Level= : ",
                 iz);
    }
    // Update solution vector
    mdot_soln->set(node_out, mdot_out); // kg/sec
  }
}

void
SubChannel1PhaseProblemBase::computeWijPrime(int iz)
{
  if (iz == 0)
  {
    mooseError(name(),
               ": Cannot compute crossflow quantities at the inlet of the assembly. Boundary "
               "conditions are applied here");
  }
  auto z_grid = _subchannel_mesh.getZGrid();
  unsigned int n_gaps = _subchannel_mesh.getNumOfGapsPerLayer();
  auto dz = z_grid[iz] - z_grid[iz - 1];
  for (unsigned int i_gap = 0; i_gap < n_gaps; i_gap++)
  {
    auto chans = _subchannel_mesh.getGapNeighborChannels(i_gap);
    unsigned int i_ch = chans.first;
    unsigned int j_ch = chans.second;
    auto * node_in_i = _subchannel_mesh.getChannelNode(i_ch, iz - 1);
    auto * node_out_i = _subchannel_mesh.getChannelNode(i_ch, iz);
    auto * node_in_j = _subchannel_mesh.getChannelNode(j_ch, iz - 1);
    auto * node_out_j = _subchannel_mesh.getChannelNode(j_ch, iz);
    // area of channel i
    auto Si = (*S_flow_soln)(node_in_i);
    // area of channel j
    auto Sj = (*S_flow_soln)(node_in_j);
    // crossflow area between channels i,j dz*gap_width
    auto Sij = dz * _subchannel_mesh.getGapWidth(i_gap);
    // Calculation of Turbulent Crossflow
    for (unsigned int i_gap = 0; i_gap < n_gaps; i_gap++)
    {
      WijPrime(i_gap, iz) = _abeta * 0.5 *
                            (((*mdot_soln)(node_in_i) + (*mdot_soln)(node_out_i) +
                              (*mdot_soln)(node_in_j) + (*mdot_soln)(node_out_j)) /
                             (Si + Sj)) *
                            Sij; // Kg/sec
    }
  }
}

void
SubChannel1PhaseProblemBase::computeDP(int iz)
{
  if (iz == 0)
  {
    mooseError(name(),
               ": Cannot compute pressure drop at the inlet of the assembly. Boundary conditions "
               "are applied here");
  }
  auto z_grid = _subchannel_mesh.getZGrid();
  auto dz = z_grid[iz] - z_grid[iz - 1];
  // Sweep through the channels of level
  for (unsigned int i_ch = 0; i_ch < _subchannel_mesh.getNumOfChannels(); i_ch++)
  {
    // Find the nodes for the top and bottom of this element.
    auto * node_in = _subchannel_mesh.getChannelNode(i_ch, iz - 1);
    auto * node_out = _subchannel_mesh.getChannelNode(i_ch, iz);
    auto rho_in = (*rho_soln)(node_in);
    auto rho_out = (*rho_soln)(node_out);
    auto T_in = (*T_soln)(node_in);
    auto S = (*S_flow_soln)(node_in);
    auto w_perim = (*w_perim_soln)(node_in);
    // hydraulic diameter in the i direction
    auto Dh_i = 4.0 * S / w_perim;
    auto Time_Term =
        ((*mdot_soln)(node_out)-mdot_soln->old(node_out)) * dz / dt() -
        dz * 2 * (*mdot_soln)(node_out) * (rho_out - rho_soln->old(node_out)) / rho_in / dt();

    auto Mass_Term1 = std::pow((*mdot_soln)(node_out), 2.0) * (1 / S / rho_out - 1 / S / rho_in);
    auto Mass_Term2 = -2 * (*mdot_soln)(node_out) * (*SumWij_soln)(node_out) / S / rho_in;

    auto CrossFlow_Term = 0.0;
    auto Turbulent_Term = 0.0;

    unsigned int counter = 0;
    for (auto i_gap : _subchannel_mesh.getChannelGaps(i_ch))
    {
      auto chans = _subchannel_mesh.getGapNeighborChannels(i_gap);
      unsigned int ii_ch = chans.first;
      unsigned int jj_ch = chans.second;
      auto * node_in_i = _subchannel_mesh.getChannelNode(ii_ch, iz - 1);
      auto * node_in_j = _subchannel_mesh.getChannelNode(jj_ch, iz - 1);
      auto * node_out_i = _subchannel_mesh.getChannelNode(ii_ch, iz);
      auto * node_out_j = _subchannel_mesh.getChannelNode(jj_ch, iz);
      auto rho_i = (*rho_soln)(node_in_i);
      auto rho_j = (*rho_soln)(node_in_j);
      auto Si = (*S_flow_soln)(node_in_i);
      auto Sj = (*S_flow_soln)(node_in_j);
      auto U_star = 0.0;
      // figure out donor axial velocity
      if (Wij(i_gap, iz) > 0.0)
      {
        U_star = (*mdot_soln)(node_out_i) / Si / rho_i;
      }
      else
      {
        U_star = (*mdot_soln)(node_out_j) / Sj / rho_j;
      }

      CrossFlow_Term += _subchannel_mesh.getCrossflowSign(i_ch, counter) * Wij(i_gap, iz) * U_star;

      Turbulent_Term += WijPrime(i_gap, iz) * (2 * (*mdot_soln)(node_out) / rho_in / S -
                                               (*mdot_soln)(node_out_j) / Sj / rho_j -
                                               (*mdot_soln)(node_out_i) / Si / rho_i);
      counter++;
    }
    Turbulent_Term *= _CT;

    auto mu = _fp->mu_from_rho_T(rho_in, T_in);
    auto Re = (((*mdot_soln)(node_in) / S) * Dh_i / mu);
    auto fi = computeFrictionFactor(Re);
    auto Friction_Term = (fi * dz / Dh_i) * 0.5 * (std::pow((*mdot_soln)(node_out), 2.0)) /
                         (S * (*rho_soln)(node_out));
    auto Gravity_Term = _g_grav * (*rho_soln)(node_out)*dz * S;
    auto DP = 0.0;
    if (isTransient())
    {
      DP = std::pow(S, -1.0) * (Time_Term + Mass_Term1 + Mass_Term2 + CrossFlow_Term +
                                Turbulent_Term + Friction_Term + Gravity_Term); // Pa
    }
    else
    {
      // enabling the turbulent term in the momentum equation produces a radial pressure gradient
      DP = std::pow(S, -1.0) * (Mass_Term1 + Mass_Term2 + CrossFlow_Term + Turbulent_Term +
                                Friction_Term + Gravity_Term); // Pa
    }
    // update solution
    DP_soln->set(node_out, DP);
  }
}

void
SubChannel1PhaseProblemBase::computeP(int iz)
{
  if (iz == 0)
  {
    mooseError(name(),
               ": Cannot compute pressure at the outlet of the assembly. Boundary conditions are "
               "applied here");
  }
  // Calculate pressure in the inlet of the cell assuming known outlet
  for (unsigned int i_ch = 0; i_ch < _subchannel_mesh.getNumOfChannels(); i_ch++)
  {
    auto * node_out = _subchannel_mesh.getChannelNode(i_ch, iz);
    auto * node_in = _subchannel_mesh.getChannelNode(i_ch, iz - 1);
    // update Pressure solution
    P_soln->set(node_in, (*P_soln)(node_out) + (*DP_soln)(node_out));
  }
}

void
SubChannel1PhaseProblemBase::computeH(int iz)
{
  if (iz == 0)
  {
    for (unsigned int i_ch = 0; i_ch < _subchannel_mesh.getNumOfChannels(); i_ch++)
    {
      auto * node = _subchannel_mesh.getChannelNode(i_ch, iz);
      h_soln->set(node, _fp->h_from_p_T((*P_soln)(node), (*T_soln)(node)));
    }
    return;
  }
  auto z_grid = _subchannel_mesh.getZGrid();
  auto dz = z_grid[iz] - z_grid[iz - 1];
  // go through the channels of the level.
  for (unsigned int i_ch = 0; i_ch < _subchannel_mesh.getNumOfChannels(); i_ch++)
  {
    // Start with applying mass-conservation equation & energy - conservation equation
    // Find the nodes for the top and bottom of this element.
    auto * node_in = _subchannel_mesh.getChannelNode(i_ch, iz - 1);
    auto * node_out = _subchannel_mesh.getChannelNode(i_ch, iz);
    // Copy the variables at the inlet (bottom) of this element.
    auto mdot_in = (*mdot_soln)(node_in);
    auto h_in = (*h_soln)(node_in); // J/kg
    auto volume = dz * (*S_flow_soln)(node_in);
    auto mdot_out = (*mdot_soln)(node_out);
    auto h_out = 0.0;
    double SumWijh = 0.0;
    double SumWijPrimeDhij = 0.0;
    // Calculate sum of crossflow into channel i from channels j around i
    unsigned int counter = 0;
    for (auto i_gap : _subchannel_mesh.getChannelGaps(i_ch))
    {
      auto chans = _subchannel_mesh.getGapNeighborChannels(i_gap);
      unsigned int ii_ch = chans.first;
      // i is always the smallest and first index in the mapping
      unsigned int jj_ch = chans.second;
      auto * node_in_i = _subchannel_mesh.getChannelNode(ii_ch, iz - 1);
      auto * node_in_j = _subchannel_mesh.getChannelNode(jj_ch, iz - 1);
      // Define donor enthalpy
      auto h_star = 0.0;
      if (Wij(i_gap, iz) > 0.0)
      {
        h_star = (*h_soln)(node_in_i);
      }
      else
      {
        h_star = (*h_soln)(node_in_j);
      }
      // take care of the sign by applying the map, use donor cell
      SumWijh += _subchannel_mesh.getCrossflowSign(i_ch, counter) * Wij(i_gap, iz) * h_star;
      SumWijPrimeDhij += WijPrime(i_gap, iz) *
                         (2 * (*h_soln)(node_in) - (*h_soln)(node_in_j) - (*h_soln)(node_in_i));
      counter++;
    }

    if (isTransient())
    {
      h_out = std::pow(mdot_out, -1) *
              (mdot_in * h_in - SumWijh - SumWijPrimeDhij +
               ((*q_prime_soln)(node_out) + (*q_prime_soln)(node_in)) * dz / 2.0 -
               ((*rho_soln)(node_out) * (*h_soln)(node_out)-rho_soln->old(node_out) *
                h_soln->old(node_out)) *
                   volume / dt());
    }
    else
    {
      h_out = std::pow(mdot_in, -1) *
              (mdot_in * h_in - SumWijh - SumWijPrimeDhij +
               ((*q_prime_soln)(node_out) + (*q_prime_soln)(node_in)) * dz / 2.0);
    }
    if (h_out < 0)
    {
      _console << "Wij = : " << Wij << "\n";
      mooseError(
          name(), " : Calculation of negative Enthalpy h_out = : ", h_out, " Axial Level= : ", iz);
    }
    // Update the solution vectors at the outlet of the cell
    h_soln->set(node_out, h_out); // J/kg
  }
}

void
SubChannel1PhaseProblemBase::computeT(int iz)
{
  if (iz == 0)
  {
    mooseError(name(),
               ": Cannot compute temperature at the inlet of the assembly. Boundary conditions are "
               "applied here");
  }
  for (unsigned int i_ch = 0; i_ch < _subchannel_mesh.getNumOfChannels(); i_ch++)
  {
    auto * node = _subchannel_mesh.getChannelNode(i_ch, iz);
    auto T = _fp->T_from_p_h((*P_soln)(node), (*h_soln)(node));
    T_soln->set(node, T); // Kelvin
  }
}

void
SubChannel1PhaseProblemBase::computeRho(int iz)
{
  for (unsigned int i_ch = 0; i_ch < _subchannel_mesh.getNumOfChannels(); i_ch++)
  {
    // Find the node
    auto * node = _subchannel_mesh.getChannelNode(i_ch, iz);
    auto rho = _fp->rho_from_p_T((*P_soln)(node), (*T_soln)(node));
    rho_soln->set(node, rho); // Kg/m3
  }
}

double
SubChannel1PhaseProblemBase::computeMassFlowForDPDZ(double dpdz, int i_ch)
{
  auto * node = _subchannel_mesh.getChannelNode(i_ch, 0);
  // initialize massflow
  auto massflow = (*mdot_soln)(node);
  auto rho = (*rho_soln)(node);
  auto T = (*T_soln)(node);
  auto mu = _fp->mu_from_rho_T(rho, T);
  auto Si = (*S_flow_soln)(node);
  auto w_perim = (*w_perim_soln)(node);
  auto Dhi = 4.0 * Si / w_perim;
  auto max_iter = 10;
  auto TOL = 1e-6;
  // Iterate until we find massflow that matches the given dp/dz.
  auto iter = 0;
  auto Error = 1.0;
  while (Error > TOL)
  {
    iter += 1;
    if (iter > max_iter)
    {
      mooseError(name(), ": exceeded maximum number of iterations");
    }
    auto massflow_old = massflow;
    auto Rei = ((massflow / Si) * Dhi / mu);
    auto fi = computeFrictionFactor(Rei);
    massflow = sqrt(2.0 * Dhi * dpdz * rho * std::pow(Si, 2.0) / fi);
    Error = std::abs((massflow - massflow_old) / massflow_old);
  }
  return massflow;
}

void
SubChannel1PhaseProblemBase::enforceUniformDPDZAtInlet()
{
  _console
      << "Edit mass flow boundary condition in order to have uniform Pressure drop at the inlet\n";
  auto total_mass_flow = 0.0;
  for (unsigned int i_ch = 0; i_ch < _subchannel_mesh.getNumOfChannels(); i_ch++)
  {
    auto * node_in = _subchannel_mesh.getChannelNode(i_ch, 0);
    total_mass_flow += (*mdot_soln)(node_in);
  }
  _console << "Total mass flow :" << total_mass_flow << " [kg/sec] \n";
  // Define vectors of pressure drop and massflow
  Eigen::VectorXd DPDZi(_subchannel_mesh.getNumOfChannels());
  Eigen::VectorXd MassFlowi(_subchannel_mesh.getNumOfChannels());
  // Calculate Pressure drop derivative for current mass flow BC
  for (unsigned int i_ch = 0; i_ch < _subchannel_mesh.getNumOfChannels(); i_ch++)
  {
    auto * node_in = _subchannel_mesh.getChannelNode(i_ch, 0);
    auto rho_in = (*rho_soln)(node_in);
    auto T_in = (*T_soln)(node_in);
    auto Si = (*S_flow_soln)(node_in);
    auto w_perim = (*w_perim_soln)(node_in);
    auto Dhi = 4.0 * Si / w_perim;
    auto mu = _fp->mu_from_rho_T(rho_in, T_in);
    auto Rei = (((*mdot_soln)(node_in) / Si) * Dhi / mu);
    auto fi = computeFrictionFactor(Rei);
    DPDZi(i_ch) =
        (fi / Dhi) * 0.5 * (std::pow((*mdot_soln)(node_in), 2.0)) / (std::pow(Si, 2.0) * rho_in);
  }

  // Initialize an average pressure drop for uniform pressure inlet condition
  auto DPDZ = DPDZi.mean();
  auto Error = 1.0;
  auto tol = 1e-6;
  auto iter = 0;
  auto max_iter = 10;
  while (Error > tol)
  {
    iter += 1;
    if (iter > max_iter)
    {
      mooseError(name(), ": exceeded maximum number of iterations");
    }
    auto DPDZ_old = DPDZ;
    for (unsigned int i_ch = 0; i_ch < _subchannel_mesh.getNumOfChannels(); i_ch++)
    {
      // update inlet mass flow to achieve DPDZ
      MassFlowi(i_ch) = computeMassFlowForDPDZ(DPDZ, i_ch);
    }
    // Calculate total massflow at the inlet
    auto mass_flow_sum = MassFlowi.sum();
    // Update the DP/DZ to correct the mass flow rate.
    DPDZ *= std::pow(total_mass_flow / mass_flow_sum, 2.0);
    Error = std::abs((DPDZ - DPDZ_old) / DPDZ_old);
  }

  // Populate solution vector with corrected boundary conditions
  for (unsigned int i_ch = 0; i_ch < _subchannel_mesh.getNumOfChannels(); i_ch++)
  {
    auto * node = _subchannel_mesh.getChannelNode(i_ch, 0);
    mdot_soln->set(node, MassFlowi(i_ch));
  }
  _console << "Done Applying mass flow boundary condition\n";
}

void
SubChannel1PhaseProblemBase::externalSolve()
{
  if (_enforce_uniform_pressure)
  {
    enforceUniformDPDZAtInlet();
  }
  _console << "Executing subchannel solver\n";
  unsigned int nz = _subchannel_mesh.getNumOfAxialNodes();
  Eigen::MatrixXd PCYCLES(nz, 2);
  // Initialize
  PCYCLES.setZero();
  auto Ptol = 1E-10, Mtol = 1E-10;
  // nz level calculations
  for (unsigned int axial_level = 1; axial_level < nz + 1; axial_level++)
  {
    _console << "AXIAL LEVEL: " << axial_level << std::endl;
    double PError = 1.0;
    unsigned int stencilSize = 5;
    int max_axial_cycles = 200;
    int axial_cycles = 0;
    int max_level_cycles = 200;
    int bottom_limiter;
    while (PError > Ptol && axial_cycles <= max_axial_cycles)
    {
      if (axial_level < stencilSize)
        bottom_limiter = 1;
      else
        bottom_limiter = axial_level - stencilSize + 1;
      axial_cycles++;
      PCYCLES(axial_level - 1, 0) = axial_cycles;
      if (axial_cycles == max_axial_cycles)
      {
        mooseError(name(), " Pressure loop didn't converge, axial_cycles: ", axial_cycles);
      }
      // L2 norm of old pressure solution vector
      auto P_L2norm_old = P_soln->L2norm();
      // Sweep upwards through the channels.
      for (unsigned int iz = bottom_limiter; iz < axial_level + 1; iz++)
      {
        double MError = 1.0;
        int level_cycles = 0;
        // Lateral Loop... crossflow calculation
        while (MError > Mtol && (level_cycles <= max_level_cycles))
        {
          level_cycles++;
          if (level_cycles == max_level_cycles)
          {
            mooseError(name(), " Level loop didn't converge, level_cycles: ", level_cycles);
          }
          // L2 norm of old mass flow solution vector
          auto mdot_L2norm_old = mdot_soln->L2norm();
          // Calculate crossflow between channel i-j using crossflow momentum equation
          computeWij(iz);
          // calculate Sum of crossflow per channel
          computeSumWij(iz);
          // Calculate mass flow
          computeMdot(iz);
          // Calculate turbulent crossflow
          computeWijPrime(iz);
          // Calculate enthalpy (Rho and H need to be updated at the inlet to (TODO))
          computeH(iz);
          // Update Temperature
          computeT(iz);
          // Update Density
          computeRho(iz);
          // Calculate Error per level
          auto mdot_L2norm_new = mdot_soln->L2norm();
          MError = std::abs((mdot_L2norm_new - mdot_L2norm_old) / (mdot_L2norm_old + 1E-14));
        }
        // Populate Pressure Drop vector
        computeDP(iz);
      }
      // At this point we reach the top of the oscillating stencil and now backsolve
      if (axial_level == nz)
        bottom_limiter = 1;
      // Calculate pressure everywhere (in the stencil) using the axial momentum equation
      // Sweep downwards through the channels level by level
      for (int iz = axial_level; iz > bottom_limiter - 1; iz--) // nz calculations
      {
        for (unsigned int i_ch = 0; i_ch < _subchannel_mesh.getNumOfChannels(); i_ch++)
        {
          auto * node_out = _subchannel_mesh.getChannelNode(i_ch, iz);
          auto * node_in = _subchannel_mesh.getChannelNode(i_ch, iz - 1);
          // update Pressure solution
          P_soln->set(node_in, (*P_soln)(node_out) + (*DP_soln)(node_out));
        }
      }
      //      // Calculate pressure Error
      auto P_L2norm_new = P_soln->L2norm();
      PError = std::abs((P_L2norm_new - P_L2norm_old) / (P_L2norm_old + 1E-14));
      _console << "- PError: " << PError << std::endl;
      PCYCLES(axial_level - 1, 1) = PError;
    }
  }
  // update old crossflow matrix
  Wij_old = Wij;
  // Save Wij_global
  std::ofstream myfile1;
  myfile1.open("Wij_global", std::ofstream::trunc);
  myfile1 << std::setprecision(12) << std::scientific << Wij << "\n";
  myfile1.close();
  _console << "Finished executing subchannel solver\n";
  _aux->solution().close();
}

void SubChannel1PhaseProblemBase::syncSolutions(Direction /*direction*/) {}
