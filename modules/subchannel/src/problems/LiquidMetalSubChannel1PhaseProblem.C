#include "LiquidMetalSubChannel1PhaseProblem.h"
#include "AuxiliarySystem.h"
registerMooseObject("SubChannelApp", LiquidMetalSubChannel1PhaseProblem);

InputParameters
LiquidMetalSubChannel1PhaseProblem::validParams()
{
  InputParameters params = SubChannel1PhaseProblemBase::validParams();
  return params;
}

LiquidMetalSubChannel1PhaseProblem::LiquidMetalSubChannel1PhaseProblem(
    const InputParameters & params)
  : SubChannel1PhaseProblemBase(params)
{
}

double
LiquidMetalSubChannel1PhaseProblem::computeFrictionFactor(double Re)
{
  double a, b;
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
  dpz_error0 = 1.0E-4;
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
