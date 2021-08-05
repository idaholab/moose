#include "LiquidMetalSubChannel1PhaseProblem.h"
#include "AuxiliarySystem.h"
#include "TriSubChannelMesh.h"

registerMooseObject("SubChannelApp", LiquidMetalSubChannel1PhaseProblem);

InputParameters
LiquidMetalSubChannel1PhaseProblem::validParams()
{
  InputParameters params = SubChannel1PhaseProblemBase::validParams();
  return params;
}

LiquidMetalSubChannel1PhaseProblem::LiquidMetalSubChannel1PhaseProblem(
    const InputParameters & params)
  : SubChannel1PhaseProblemBase(params),
    _tri_sch_mesh(dynamic_cast<TriSubChannelMesh &>(_subchannel_mesh))
{
}

double
LiquidMetalSubChannel1PhaseProblem::computeFrictionFactor(double Re)
{
  _console << "Re number= " << Re << std::endl;
  mooseError(name(),
             ": This option for the friction factor is not currently used for sodium coolant");
  return 0;
}

void
LiquidMetalSubChannel1PhaseProblem::computeH(int iz)
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
    const Real & pitch = _subchannel_mesh.getPitch();
    const Real & rod_diameter = _subchannel_mesh.getRodDiameter();

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
    double sum_wijh = 0.0;
    double sum_wijprime_dhij = 0.0;
    // Calculate sum of crossflow into channel i from channels j around i
    unsigned int counter = 0;
    Real e_cond = 0.0;
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
      else if (Wij(i_gap, iz) < 0.0)
      {
        h_star = (*h_soln)(node_in_j);
      }
      // take care of the sign by applying the map, use donor cell
      sum_wijh += _subchannel_mesh.getCrossflowSign(i_ch, counter) * Wij(i_gap, iz) * h_star;
      sum_wijprime_dhij += WijPrime(i_gap, iz) *
                           (2 * (*h_soln)(node_in) - (*h_soln)(node_in_j) - (*h_soln)(node_in_i));
      counter++;

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
      auto thcon_i = _fp->k_from_p_T((*P_soln)(node_in_i), (*T_soln)(node_in_i));
      auto thcon_j = _fp->k_from_p_T((*P_soln)(node_in_j), (*T_soln)(node_in_j));
      auto shape_factor = 0.66 * (pitch / rod_diameter) *
                          std::pow((_subchannel_mesh.getGapWidth(i_gap) / rod_diameter), -0.3);
      if (ii_ch == i_ch)
      {
        e_cond += 0.5 * (thcon_i + thcon_j) * Sij * shape_factor *
                  ((*T_soln)(node_in_j) - (*T_soln)(node_in_i)) / dist_ij;
      }
      else
      {
        e_cond += -0.5 * (thcon_i + thcon_j) * Sij * shape_factor *
                  ((*T_soln)(node_in_j) - (*T_soln)(node_in_i)) / dist_ij;
      }
    }

    // compute the axial heat conduction between current and lower axial node
    auto * node_in_i = _subchannel_mesh.getChannelNode(i_ch, iz);
    auto * node_in_j = _subchannel_mesh.getChannelNode(i_ch, iz - 1);
    auto thcon_i = _fp->k_from_p_T((*P_soln)(node_in_i), (*T_soln)(node_in_i));
    auto thcon_j = _fp->k_from_p_T((*P_soln)(node_in_j), (*T_soln)(node_in_j));
    auto Si = (*S_flow_soln)(node_in_i);
    auto dist_ij = z_grid[iz] - z_grid[iz - 1];

    e_cond +=
        0.5 * (thcon_i + thcon_j) * Si * ((*T_soln)(node_in_j) - (*T_soln)(node_in_i)) / dist_ij;

    int nz = _subchannel_mesh.getNumOfAxialNodes();
    // compute the axial heat conduction between current and upper axial node
    if (iz < nz)
    {

      auto * node_in_i = _subchannel_mesh.getChannelNode(i_ch, iz);
      auto * node_in_j = _subchannel_mesh.getChannelNode(i_ch, iz + 1);
      auto thcon_i = _fp->k_from_p_T((*P_soln)(node_in_i), (*T_soln)(node_in_i));
      auto thcon_j = _fp->k_from_p_T((*P_soln)(node_in_j), (*T_soln)(node_in_j));
      auto Si = (*S_flow_soln)(node_in_i);
      auto dist_ij = z_grid[iz + 1] - z_grid[iz];
      e_cond +=
          0.5 * (thcon_i + thcon_j) * Si * ((*T_soln)(node_in_j) - (*T_soln)(node_in_i)) / dist_ij;
    }

    h_out = (mdot_in * h_in - sum_wijh - sum_wijprime_dhij + e_cond +
             ((*q_prime_soln)(node_out) + (*q_prime_soln)(node_in)) * dz / 2.0 +
             _TR * rho_soln->old(node_out) * h_soln->old(node_out) * volume / _dt) /
            (mdot_out + _TR * (*rho_soln)(node_out)*volume / _dt);

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
LiquidMetalSubChannel1PhaseProblem::computeWijPrime(int iz)
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
    const Real & pitch = _subchannel_mesh.getPitch();
    const Real & rod_diameter = _subchannel_mesh.getRodDiameter();
    const Real & wire_lead_length = _tri_sch_mesh.getWireLeadLength();
    const Real & wire_diameter = _tri_sch_mesh.getWireDiameter();
    auto theta = std::acos(wire_lead_length /
                           std::sqrt(std::pow(wire_lead_length, 2) +
                                     std::pow(libMesh::pi * (rod_diameter + wire_diameter), 2)));
    // Calculation of Turbulent Crossflow
    for (unsigned int i_gap = 0; i_gap < n_gaps; i_gap++)
    {
      auto dum1 = 0.14 * std::pow((pitch - rod_diameter) / rod_diameter, -0.5);
      auto dum2 = libMesh::pi / 6 * std::pow((pitch - rod_diameter / 2), 2);
      auto dum3 = libMesh::pi * std::pow(rod_diameter, 2) / 24;
      auto dum4 = std::sqrt(3) / 4 * std::pow(pitch, 2);
      auto dum5 = libMesh::pi * std::pow(rod_diameter, 2) / 8;
      auto eps = dum1 * pow((dum2 - dum3) / (dum4 - dum5), 0.5) * std::tan(theta);
      WijPrime(i_gap, iz) = eps * 0.5 *
                            (((*mdot_soln)(node_in_i) + (*mdot_soln)(node_out_i) +
                              (*mdot_soln)(node_in_j) + (*mdot_soln)(node_out_j)) /
                             (Si + Sj)) *
                            Sij; // Kg/sec
    }
  }
}

double
LiquidMetalSubChannel1PhaseProblem::computeFrictionFactor(
    double Re, int i_ch, Real S, Real w_perim, Real Dh_i)
{
  const Real & pitch = _subchannel_mesh.getPitch();
  const Real & rod_diameter = _subchannel_mesh.getRodDiameter();
  const Real & wire_lead_length = _tri_sch_mesh.getWireLeadLength();
  const Real & wire_diameter = _tri_sch_mesh.getWireDiameter();
  auto p_to_d = pitch / rod_diameter;
  auto subch_type = _subchannel_mesh.getSubchannelType(i_ch);
  auto gap = _tri_sch_mesh.getDuctToRodGap();
  auto w_to_d = (rod_diameter + gap) / rod_diameter;
  auto Reb_l = std::pow(10, (p_to_d - 1)) * 320.0;
  auto Reb_t = std::pow(10, 0.7 * (p_to_d - 1)) * 1.0E+4;
  const Real lambda = 7.0;
  auto fib = std::log(Re / Reb_l) / std::log(Reb_t / Reb_l);
  Real a_l = 0.0;
  Real b1_l = 0.0;
  Real b2_l = 0.0;
  Real a_t = 0.0;
  Real b1_t = 0.0;
  Real b2_t = 0.0;
  Real cfl_p = 0.0;
  Real cft_p = 0.0;
  auto theta = std::acos(wire_lead_length /
                         std::sqrt(std::pow(wire_lead_length, 2) +
                                   std::pow(libMesh::pi * (rod_diameter + wire_diameter), 2)));
  auto wd_t = (19.56 + 98.71 * (wire_diameter / rod_diameter) +
               303.47 * std::pow((wire_diameter / rod_diameter), 2)) *
              std::pow((wire_lead_length / rod_diameter), -0.541);
  auto wd_l = 1.4 * wd_t;
  auto ws_t = -11.0 * std::log(wire_lead_length / rod_diameter) / std::log(10.0) + 19.0;
  auto ws_l = ws_t;
  Real pw_p = 0.0;
  Real ar = 0.0;
  Real a_p = 0.0;
  Real cf_t = 0.0;
  Real cf_l = 0.0;

  // Find the coefficients of bare rod bundle friction factor
  // correlations for turbulent and laminar flow regimes.
  if (subch_type == EChannelType::CENTER)
  {
    if (p_to_d < 1.1)
    {
      a_l = 26.0;
      b1_l = 888.2;
      b2_l = -3334.0;
      a_t = 0.09378;
      b1_t = 1.398;
      b2_t = -8.664;
    }
    else
    {
      a_l = 62.97;
      b1_l = 216.9;
      b2_l = -190.2;
      a_t = 0.1458;
      b1_t = 0.03632;
      b2_t = -0.03333;
    }
    // laminar flow friction factor for bare rod bundle - Center subchannel
    cfl_p = a_l + b1_l * (p_to_d - 1) + b2_l * std::pow((p_to_d - 1), 2);
    // turbulent flow friction factor for bare rod bundle - Center subchannel
    cft_p = a_t + b1_t * (p_to_d - 1) + b2_t * std::pow((p_to_d - 1), 2);
  }
  else if (subch_type == EChannelType::EDGE)
  {
    if (p_to_d < 1.1)
    {
      a_l = 26.18;
      b1_l = 554.5;
      b2_l = -1480.0;
      a_t = 0.09377;
      b1_t = 0.8732;
      b2_t = -3.341;
    }
    else
    {
      a_l = 44.4;
      b1_l = 256.7;
      b2_l = -267.6;
      a_t = 0.1430;
      b1_t = 0.04199;
      b2_t = -0.04428;
    }
    // laminar flow friction factor for bare rod bundle - Edge subchannel
    cfl_p = a_l + b1_l * (w_to_d - 1) + b2_l * std::pow((w_to_d - 1), 2);
    // turbulent flow friction factor for bare rod bundle - Edge subchannel
    cft_p = a_t + b1_t * (w_to_d - 1) + b2_t * std::pow((w_to_d - 1), 2);
  }
  else
  {
    if (p_to_d < 1.1)
    {
      a_l = 26.98;
      b1_l = 1636.0;
      b2_l = -10050.0;
      a_t = 0.1004;
      b1_t = 1.625;
      b2_t = -11.85;
    }
    else
    {
      a_l = 87.26;
      b1_l = 38.59;
      b2_l = -55.12;
      a_t = 0.1499;
      b1_t = 0.006706;
      b2_t = -0.0009567;
    }
    // laminar flow friction factor for bare rod bundle - Corner subchannel
    cfl_p = a_l + b1_l * (w_to_d - 1) + b2_l * std::pow((w_to_d - 1), 2);
    // turbulent flow friction factor for bare rod bundle - Corner subchannel
    cft_p = a_t + b1_t * (w_to_d - 1) + b2_t * std::pow((w_to_d - 1), 2);
  }

  if (subch_type == EChannelType::CENTER)
  {
    // wetted perimeter for center subchannel and bare rod bundle
    pw_p = libMesh::pi * rod_diameter / 2.0;
    // wire projected area - center subchannel wire-wrapped bundle
    ar = libMesh::pi * (rod_diameter + wire_diameter) * wire_diameter / 6.0;
    // bare rod bundle center subchannel flow area
    a_p =
        std::sqrt(3.0) / 4.0 * std::pow(pitch, 2.0) - libMesh::pi * std::pow(rod_diameter, 2) / 8.0;
    // turbulent friction factor equation constant - Center subchannel
    cf_t = cft_p * (pw_p / w_perim) + wd_t * (3 * ar / a_p) * (Dh_i / wire_lead_length) *
                                          std::pow((Dh_i / wire_diameter), 0.18);
    // laminar friction factor equation constant - Center subchannel
    cf_l = cfl_p * (pw_p / w_perim) +
           wd_l * (3 * ar / a_p) * (Dh_i / wire_lead_length) * (Dh_i / wire_diameter);
  }
  else if (subch_type == EChannelType::EDGE)
  {
    // wire projected area - edge subchannel wire-wrapped bundle
    ar = libMesh::pi * (rod_diameter + wire_diameter) * wire_diameter / 4.0;
    // bare rod bundle edge subchannel flow area
    a_p = S + 0.5 * libMesh::pi * std::pow(wire_diameter, 2) / 4.0 / std::cos(theta);
    // turbulent friction factor equation constant - Edge subchannel
    cf_t = cft_p * std::pow((1 + ws_t * (ar / a_p) * std::pow(std::tan(theta), 2.0)), 1.41);
    // laminar friction factor equation constant - Edge subchannel
    cf_l = cfl_p * (1 + ws_l * (ar / a_p) * std::pow(std::tan(theta), 2.0));
  }
  else
  {
    // wire projected area - corner subchannel wire-wrapped bundle
    ar = libMesh::pi * (rod_diameter + wire_diameter) * wire_diameter / 6.0;
    // bare rod bundle corner subchannel flow area
    a_p = S + 1.0 / 6.0 * libMesh::pi / 4.0 * std::pow(wire_diameter, 2) / std::cos(theta);
    // turbulent friction factor equation constant - Corner subchannel
    cf_t = cft_p * std::pow((1 + ws_t * (ar / a_p) * std::pow(std::tan(theta), 2.0)), 1.41);
    // laminar friction factor equation constant - Corner subchannel
    cf_l = cfl_p * (1 + ws_l * (ar / a_p) * std::pow(std::tan(theta), 2.0));
  }
  // laminar friction factor
  auto f_l = cf_l / Re;
  // turbulent friction factor
  auto f_t = cf_t / std::pow(Re, 0.18);

  if (Re < Reb_l)
  {
    // laminar flow
    return f_l;
  }
  else if (Re > Reb_t)
  {
    // turbulent flow
    return f_t;
  }
  else
  {
    // transition flow
    return f_l * std::pow((1 - fib), 1.0 / 3.0) * (1 - std::pow(fib, lambda)) +
           f_t * std::pow(fib, 1.0 / 3.0);
  }
}

void
LiquidMetalSubChannel1PhaseProblem::computeDP(int iz)
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
    auto time_term =
        _TR * ((*mdot_soln)(node_out)-mdot_soln->old(node_out)) * dz / _dt -
        dz * 2.0 * (*mdot_soln)(node_out) * (rho_out - rho_soln->old(node_out)) / rho_in / _dt;

    auto mass_term1 =
        std::pow((*mdot_soln)(node_out), 2.0) * (1.0 / S / rho_out - 1.0 / S / rho_in);
    auto mass_term2 = -2.0 * (*mdot_soln)(node_out) * (*SumWij_soln)(node_out) / S / rho_in;

    auto crossflow_term = 0.0;
    auto turbulent_term = 0.0;

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

      crossflow_term += _subchannel_mesh.getCrossflowSign(i_ch, counter) * Wij(i_gap, iz) * U_star;

      turbulent_term += WijPrime(i_gap, iz) * (2 * (*mdot_soln)(node_out) / rho_in / S -
                                               (*mdot_soln)(node_out_j) / Sj / rho_j -
                                               (*mdot_soln)(node_out_i) / Si / rho_i);
      counter++;
    }
    turbulent_term *= _CT;

    auto mu = _fp->mu_from_rho_T(rho_in, T_in);
    auto Re = (((*mdot_soln)(node_in) / S) * Dh_i / mu);
    auto fi = computeFrictionFactor(Re, i_ch, S, w_perim, Dh_i);
    // auto fi = computeFrictionFactor(Re);
    auto friction_term = (fi * dz / Dh_i) * 0.5 * (std::pow((*mdot_soln)(node_out), 2.0)) /
                         (S * (*rho_soln)(node_out));
    auto gravity_term = _g_grav * (*rho_soln)(node_out)*dz * S;
    auto DP = std::pow(S, -1.0) * (time_term + mass_term1 + mass_term2 + crossflow_term +
                                   turbulent_term + friction_term + gravity_term); // Pa
    // update solution
    DP_soln->set(node_out, DP);
  }
}

double
LiquidMetalSubChannel1PhaseProblem::computeMassFlowForDPDZ(double dpdz, int i_ch)
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
  auto error = 1.0;
  while (error > TOL)
  {
    iter += 1;
    if (iter > max_iter)
      mooseError(name(), ": exceeded maximum number of iterations");

    auto massflow_old = massflow;
    auto Rei = ((massflow / Si) * Dhi / mu);
    auto fi = computeFrictionFactor(Rei, i_ch, Si, w_perim, Dhi);
    massflow = std::sqrt(2.0 * Dhi * dpdz * rho * std::pow(Si, 2.0) / fi);
    error = std::abs((massflow - massflow_old) / massflow_old);
  }
  return massflow;
}

void
LiquidMetalSubChannel1PhaseProblem::enforceUniformDPDZAtInlet()
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
  Eigen::VectorXd dPdZ_i(_subchannel_mesh.getNumOfChannels());
  Eigen::VectorXd mass_flow_i(_subchannel_mesh.getNumOfChannels());
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
    auto fi = computeFrictionFactor(Rei, i_ch, Si, w_perim, Dhi);
    dPdZ_i(i_ch) =
        (fi / Dhi) * 0.5 * (std::pow((*mdot_soln)(node_in), 2.0)) / (std::pow(Si, 2.0) * rho_in);
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
    mdot_soln->set(node, mass_flow_i(i_ch));
  }
  _console << "Done applying mass flow boundary condition\n";
}

void
LiquidMetalSubChannel1PhaseProblem::computeInletMassFlowDist()
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
  unsigned int nz = _subchannel_mesh.getNumOfAxialNodes();

  for (unsigned int i_ch = 0; i_ch < _subchannel_mesh.getNumOfChannels(); i_ch++)
  {
    auto * node = _subchannel_mesh.getChannelNode(i_ch, 0);
    auto * node_out = _subchannel_mesh.getChannelNode(i_ch, nz);
    Si(i_ch) = (*S_flow_soln)(node);
    mass_flow_i(i_ch) = (*mdot_soln)(node);
    mass_flux_i(i_ch) = mass_flow_i(i_ch) / Si(i_ch);
    p_in(i_ch) = (*P_soln)(node);
    p_out(i_ch) = (*P_soln)(node_out);
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
    mdot_soln->set(node, g_new(i_ch) * Si(i_ch));
  }
}

void
LiquidMetalSubChannel1PhaseProblem::externalSolve()
{
  Real dpz_error0;
  // maximum allowed inlet pressure convergence cycles
  auto max_dpz_cycles = 1;
  // inlet pressure convergence cycles
  auto dpz_cycles = 0;
  // max relative error convergence criterion for the inlet pressure distribution
  dpz_error0 = 5.0E-3;
  // initial guess for the relative error
  _dpz_error = 1.0e-2;

  if (_enforce_uniform_pressure)
  {
    enforceUniformDPDZAtInlet();
    max_dpz_cycles = 20;
  }

  _console << "Executing subchannel solver\n";
  unsigned int nz = _subchannel_mesh.getNumOfAxialNodes();
  Eigen::MatrixXd PCYCLES(nz, 2);

  // the following while loop runs up to the point the error in inlet pressure
  // distribution is less than dpz_error0
  while (_dpz_error > dpz_error0 && dpz_cycles < max_dpz_cycles)
  {
    dpz_cycles = dpz_cycles + 1;
    if (dpz_cycles > 1)
    {
      computeInletMassFlowDist();
      _console << " Inlet pressure cycle = " << dpz_cycles << " Pressure error = " << _dpz_error
               << std::endl;
    }

    if (_enforce_uniform_pressure)
    {
      if (_dpz_error > dpz_error0 && dpz_cycles == max_dpz_cycles)
      {
        mooseError(name(),
                   " Inlet mass flow rate/pressure distribution didn't converge, dpz_cycles: ",
                   dpz_cycles);
      }
    }

    // Initialize
    PCYCLES.setZero();
    auto P_tol = 1E-10, M_tol = 1E-10;
    // nz level calculations
    for (unsigned int axial_level = 1; axial_level < nz + 1; axial_level++)
    {
      _console << "AXIAL LEVEL: " << axial_level << std::endl;
      double P_error = 1.0;
      unsigned int stencil_size = 5;
      int max_axial_cycles = 200;
      int axial_cycles = 0;
      unsigned int max_level_cycles = 200;
      unsigned int bottom_limiter;
      while (P_error > P_tol && axial_cycles <= max_axial_cycles)
      {
        if (axial_level < stencil_size)
          bottom_limiter = 1;
        else
          bottom_limiter = axial_level - stencil_size + 1;
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
          double M_error = 1.0;
          unsigned int level_cycles = 0;
          // Lateral Loop... crossflow calculation
          while (M_error > M_tol && (level_cycles <= max_level_cycles))
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
            M_error = std::abs((mdot_L2norm_new - mdot_L2norm_old) / (mdot_L2norm_old + 1E-14));
          }
          // Populate Pressure Drop vector
          computeDP(iz);
        }
        // At this point we reach the top of the oscillating stencil and now backsolve
        if (axial_level == nz)
          bottom_limiter = 1;
        // Calculate pressure everywhere (in the stencil) using the axial momentum equation
        // Sweep downwards through the channels level by level
        for (unsigned int iz = axial_level; iz > bottom_limiter - 1; iz--) // nz calculations
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
        P_error = std::abs((P_L2norm_new - P_L2norm_old) / (P_L2norm_old + 1E-14));
        _console << "- P_error: " << P_error << std::endl;
        PCYCLES(axial_level - 1, 1) = P_error;
      }
    }
  } // while
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
