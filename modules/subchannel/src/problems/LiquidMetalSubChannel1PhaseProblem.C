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

double
LiquidMetalSubChannel1PhaseProblem::computeFrictionFactor(
    double Re, int i_ch, Real S, Real w_perim, Real Dh_i)
{
  // define and initialize variables used in the correlation
  const Real & pitch = _subchannel_mesh.getPitch();
  const Real & rod_diameter = _subchannel_mesh.getRodDiameter();
  const Real & wire_lead_length = _tri_sch_mesh.getWireLeadLength();
  const Real & wire_diameter = _tri_sch_mesh.getWireDiameter();
  auto p_to_d = pitch / rod_diameter;
  auto subch_type = _subchannel_mesh.getSubchannelType(i_ch);
  auto gap = _tri_sch_mesh.getDuctToRodGap();
  auto w_to_d = (rod_diameter + gap) / rod_diameter;
  auto Reb_l = pow(10, (p_to_d - 1)) * 320.0;
  auto Reb_t = pow(10, 0.7 * (p_to_d - 1)) * 1.0E+4;
  const Real lamda = 7.0;
  auto fib = log(Re / Reb_l) / log(Reb_t / Reb_l);
  Real a_l = 0.0;
  Real b1_l = 0.0;
  Real b2_l = 0.0;
  Real a_t = 0.0;
  Real b1_t = 0.0;
  Real b2_t = 0.0;
  Real cfl_p = 0.0;
  Real cft_p = 0.0;
  auto teta = acos(wire_lead_length / sqrt(pow(wire_lead_length, 2) +
                                           pow(libMesh::pi * (rod_diameter + wire_diameter), 2)));
  auto wd_t = (19.56 + 98.71 * (wire_diameter / rod_diameter) +
               303.47 * pow((wire_diameter / rod_diameter), 2)) *
              pow((wire_lead_length / rod_diameter), -0.541);
  auto wd_l = 1.4 * wd_t;
  auto ws_t = -11.0 * log(wire_lead_length / rod_diameter) / log(10.0) + 19.0;
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
    cfl_p = a_l + b1_l * (p_to_d - 1) + b2_l * pow((p_to_d - 1), 2);
    // turbulent flow friction factor for bare rod bundle - Center subchannel
    cft_p = a_t + b1_t * (p_to_d - 1) + b2_t * pow((p_to_d - 1), 2);
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
    cfl_p = a_l + b1_l * (w_to_d - 1) + b2_l * pow((w_to_d - 1), 2);
    // turbulent flow friction factor for bare rod bundle - Edge subchannel
    cft_p = a_t + b1_t * (w_to_d - 1) + b2_t * pow((w_to_d - 1), 2);
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
    cfl_p = a_l + b1_l * (w_to_d - 1) + b2_l * pow((w_to_d - 1), 2);
    // turbulent flow friction factor for bare rod bundle - Corner subchannel
    cft_p = a_t + b1_t * (w_to_d - 1) + b2_t * pow((w_to_d - 1), 2);
  }

  if (subch_type == EChannelType::CENTER)
  {
    // wetted perimeter for center subchannel and bare rod bundle
    pw_p = libMesh::pi * rod_diameter / 2.0;
    // wire projected area - center subchannel wire-wrapped bundle
    ar = libMesh::pi * (rod_diameter + wire_diameter) * wire_diameter / 6.0;
    // bare rod bundle center subchannel flow area
    a_p = sqrt(3.0) / 4.0 * pow(pitch, 2.0) - libMesh::pi * pow(rod_diameter, 2) / 8.0;
    // turbulent friction factor equation constant - Center subchannel
    cf_t = cft_p * (pw_p / w_perim) +
           wd_t * (3 * ar / a_p) * (Dh_i / wire_lead_length) * pow((Dh_i / wire_diameter), 0.18);
    // laminar friction factor equation constant - Center subchannel
    cf_l = cfl_p * (pw_p / w_perim) +
           wd_l * (3 * ar / a_p) * (Dh_i / wire_lead_length) * (Dh_i / wire_diameter);
  }
  else if (subch_type == EChannelType::EDGE)
  {
    // wire projected area - edge subchannel wire-wrapped bundle
    ar = libMesh::pi * (rod_diameter + wire_diameter) * wire_diameter / 4.0;
    // bare rod bundle edge subchannel flow area
    a_p = S + 0.5 * libMesh::pi * pow(wire_diameter, 2) / 4.0 / cos(teta);
    // turbulent friction factor equation constant - Edge subchannel
    cf_t = cft_p * pow((1 + ws_t * (ar / a_p) * pow(tan(teta), 2.0)), 1.41);
    // laminar friction factor equation constant - Edge subchannel
    cf_l = cfl_p * (1 + ws_l * (ar / a_p) * pow(tan(teta), 2.0));
  }
  else
  {
    // wire projected area - corner subchannel wire-wrapped bundle
    ar = libMesh::pi * (rod_diameter + wire_diameter) * wire_diameter / 6.0;
    // bare rod bundle corner subchannel flow area
    a_p = S + 1.0 / 6.0 * libMesh::pi / 4.0 * pow(wire_diameter, 2) / cos(teta);
    // turbulent friction factor equation constant - Corner subchannel
    cf_t = cft_p * pow((1 + ws_t * (ar / a_p) * pow(tan(teta), 2.0)), 1.41);
    // laminar friction factor equation constant - Corner subchannel
    cf_l = cfl_p * (1 + ws_l * (ar / a_p) * pow(tan(teta), 2.0));
  }
  // laminar friction factor
  auto f_l = cf_l / Re;
  // turbulent friction factor
  auto f_t = cf_t / pow(Re, 0.18);

  if (Re < Reb_l)
  {
    return f_l; // laminar flow
  }
  else if (Re > Reb_t)
  {
    return f_t; // turbulent flow
  }
  else
  {
    return f_l * pow((1 - fib), 1.0 / 3.0) * (1 - pow(fib, lamda)) +
           f_t * pow(fib, 1.0 / 3.0); // transition flow
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

    auto Mass_Term1 =
        std::pow((*mdot_soln)(node_out), 2.0) * (1.0 / S / rho_out - 1.0 / S / rho_in);
    auto Mass_Term2 = -2.0 * (*mdot_soln)(node_out) * (*SumWij_soln)(node_out) / S / rho_in;

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
    auto fi = computeFrictionFactor(Re, i_ch, S, w_perim, Dh_i);
    // auto fi = computeFrictionFactor(Re);
    auto Friction_Term = (fi * dz / Dh_i) * 0.5 * (std::pow((*mdot_soln)(node_out), 2.0)) /
                         (S * (*rho_soln)(node_out));
    auto Gravity_Term = _g_grav * (*rho_soln)(node_out)*dz * S;
    auto DP = std::pow(S, -1.0) * (time_term + Mass_Term1 + Mass_Term2 + CrossFlow_Term +
                                   Turbulent_Term + Friction_Term + Gravity_Term); // Pa
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
    auto fi = computeFrictionFactor(Rei, i_ch, Si, w_perim, Dhi);
    massflow = sqrt(2.0 * Dhi * dpdz * rho * std::pow(Si, 2.0) / fi);
    Error = std::abs((massflow - massflow_old) / massflow_old);
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
    auto fi = computeFrictionFactor(Rei, i_ch, Si, w_perim, Dhi);
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
    g_new(i_ch) = pow(dpz_ave / dpz(i_ch), 0.3) * mass_flux_i(i_ch);
    mass_flow_new_tot = mass_flow_new_tot + g_new(i_ch) * Si(i_ch);
    _dpz_error = _dpz_error + pow((dpz(i_ch) - dpz_ave), 2.0);
  }
  _dpz_error = pow(_dpz_error, 0.5) / dpz_ave / tot_chan;
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
      int max_level_cycles = 200;
      int bottom_limiter;
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
