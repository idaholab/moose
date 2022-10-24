#include "LiquidMetalSubChannel1PhaseProblem.h"
#include "AuxiliarySystem.h"
#include "TriSubChannelMesh.h"

registerMooseObject("SubChannelApp", LiquidMetalSubChannel1PhaseProblem);

InputParameters
LiquidMetalSubChannel1PhaseProblem::validParams()
{
  InputParameters params = SubChannel1PhaseProblem::validParams();
  return params;
}

LiquidMetalSubChannel1PhaseProblem::LiquidMetalSubChannel1PhaseProblem(
    const InputParameters & params)
  : SubChannel1PhaseProblem(params),
    _tri_sch_mesh(dynamic_cast<TriSubChannelMesh &>(_subchannel_mesh))
{
  _outer_channels = 0.0;
  for (unsigned int i_ch = 0; i_ch < _n_channels; i_ch++)
  {
    auto subch_type = _subchannel_mesh.getSubchannelType(i_ch);
    if (subch_type == EChannelType::EDGE || subch_type == EChannelType::CORNER)
      _outer_channels += 1.0;
  }

  // Initializing heat conduction system
  createPetscMatrix(
      _hc_axial_heat_conduction_mat, _block_size * _n_channels, _block_size * _n_channels);
  createPetscVector(_hc_axial_heat_conduction_rhs, _block_size * _n_channels);
  createPetscMatrix(
      _hc_radial_heat_conduction_mat, _block_size * _n_channels, _block_size * _n_channels);
  createPetscVector(_hc_radial_heat_conduction_rhs, _block_size * _n_channels);
  createPetscMatrix(_hc_sweep_enthalpy_mat, _block_size * _n_channels, _block_size * _n_channels);
  createPetscVector(_hc_sweep_enthalpy_rhs, _block_size * _n_channels);
}

double
LiquidMetalSubChannel1PhaseProblem::computeFrictionFactor(double Re)
{
  _console << "Re number= " << Re << std::endl;
  mooseError("This option for the friction factor is not currently used for sodium coolant");
  return 0;
}

double
LiquidMetalSubChannel1PhaseProblem::computeFrictionFactor(double Re, int /* i_ch */)
{
  _console << "Re number= " << Re << std::endl;
  mooseError("This option for the friction factor is not currently used for sodium coolant");
  return 0;
}

void
LiquidMetalSubChannel1PhaseProblem::computeWijPrime(int iblock)
{
  unsigned int last_node = (iblock + 1) * _block_size;
  unsigned int first_node = iblock * _block_size + 1;

  for (unsigned int iz = first_node; iz < last_node + 1; iz++)
  {
    auto z_grid = _subchannel_mesh.getZGrid();
    auto dz = z_grid[iz] - z_grid[iz - 1];
    for (unsigned int i_gap = 0; i_gap < _n_gaps; i_gap++)
    {
      auto chans = _subchannel_mesh.getGapNeighborChannels(i_gap);
      unsigned int i_ch = chans.first;
      unsigned int j_ch = chans.second;
      auto subch_type1 = _subchannel_mesh.getSubchannelType(i_ch);
      auto subch_type2 = _subchannel_mesh.getSubchannelType(j_ch);
      _WijPrime(i_gap, iz) = 0.0;
      if (subch_type1 == EChannelType::CENTER || subch_type2 == EChannelType::CENTER)
      {
        auto * node_in_i = _subchannel_mesh.getChannelNode(i_ch, iz - 1);
        auto * node_out_i = _subchannel_mesh.getChannelNode(i_ch, iz);
        auto * node_in_j = _subchannel_mesh.getChannelNode(j_ch, iz - 1);
        auto * node_out_j = _subchannel_mesh.getChannelNode(j_ch, iz);
        auto Si_in = (*_S_flow_soln)(node_in_i);
        auto Sj_in = (*_S_flow_soln)(node_in_j);
        auto Si_out = (*_S_flow_soln)(node_out_i);
        auto Sj_out = (*_S_flow_soln)(node_out_j);
        // crossflow area between channels i,j (dz*gap_width)
        auto Sij = dz * _subchannel_mesh.getGapWidth(i_gap);
        const Real & pitch = _subchannel_mesh.getPitch();
        const Real & rod_diameter = _subchannel_mesh.getRodDiameter();
        const Real & wire_lead_length = _tri_sch_mesh.getWireLeadLength();
        const Real & wire_diameter = _tri_sch_mesh.getWireDiameter();
        auto theta =
            std::acos(wire_lead_length /
                      std::sqrt(std::pow(wire_lead_length, 2) +
                                std::pow(libMesh::pi * (rod_diameter + wire_diameter), 2)));
        // Calculation of Turbulent Crossflow
        auto dum1 = 0.14 * std::pow((pitch - rod_diameter) / rod_diameter, -0.5);
        auto dum2 = libMesh::pi / 6 * std::pow((pitch - rod_diameter / 2), 2);
        auto dum3 = libMesh::pi * std::pow(rod_diameter, 2) / 24;
        auto dum4 = std::sqrt(3) / 4 * std::pow(pitch, 2);
        auto dum5 = libMesh::pi * std::pow(rod_diameter, 2) / 8;
        auto eps = dum1 * pow((dum2 - dum3) / (dum4 - dum5), 0.5) * std::tan(theta);
        _WijPrime(i_gap, iz) =
            eps * 0.5 *
            (((*_mdot_soln)(node_in_i) + (*_mdot_soln)(node_in_j)) / (Si_in + Sj_in) +
             ((*_mdot_soln)(node_out_i) + (*_mdot_soln)(node_out_j)) / (Si_out + Sj_out)) *
            Sij;
      }
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

Real
LiquidMetalSubChannel1PhaseProblem::computeAddedHeatDuct(unsigned int i_ch, unsigned int iz)
{
  if (_duct_mesh_exist)
  {
    auto subch_type = _subchannel_mesh.getSubchannelType(i_ch);
    if (subch_type == EChannelType::EDGE || subch_type == EChannelType::CORNER)
    {
      auto dz = _z_grid[iz] - _z_grid[iz - 1];
      auto * node_in_chan = _subchannel_mesh.getChannelNode(i_ch, iz - 1);
      auto * node_out_chan = _subchannel_mesh.getChannelNode(i_ch, iz);
      auto * node_in_duct = _subchannel_mesh.getDuctNodeFromChannel(node_in_chan);
      auto * node_out_duct = _subchannel_mesh.getDuctNodeFromChannel(node_out_chan);
      auto heat_rate_in = (*_q_prime_duct_soln)(node_in_duct);
      auto heat_rate_out = (*_q_prime_duct_soln)(node_out_duct);
      return 0.5 * (heat_rate_in + heat_rate_out) * dz / _outer_channels;
    }
    else
    {
      return 0.0;
    }
  }
  else
  {
    return 0;
  }
}

void
LiquidMetalSubChannel1PhaseProblem::computeDP(int iblock)
{
  unsigned int last_node = (iblock + 1) * _block_size;
  unsigned int first_node = iblock * _block_size + 1;

  if (!_implicit_bool)
  {
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
        auto time_term = _TR * ((*_mdot_soln)(node_out)-_mdot_soln->old(node_out)) * dz / _dt -
                         dz * 2.0 * (*_mdot_soln)(node_out) * (rho_out - _rho_soln->old(node_out)) /
                             rho_in / _dt;

        auto mass_term1 =
            std::pow((*_mdot_soln)(node_out), 2.0) * (1.0 / S / rho_out - 1.0 / S / rho_in);
        auto mass_term2 = -2.0 * (*_mdot_soln)(node_out) * (*_SumWij_soln)(node_out) / S / rho_in;

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
        auto fi = computeFrictionFactor(Re, i_ch, S, w_perim, Dh_i);
        auto ki = k_grid[i_ch][iz - 1];
        auto friction_term = (fi * dz / Dh_i + ki) * 0.5 *
                             (std::pow((*_mdot_soln)(node_out), 2.0)) /
                             (S * (*_rho_soln)(node_out));
        auto gravity_term = _g_grav * (*_rho_soln)(node_out)*dz * S;
        auto DP = std::pow(S, -1.0) * (time_term + mass_term1 + mass_term2 + crossflow_term +
                                       turbulent_term + friction_term + gravity_term); // Pa
        _DP_soln->set(node_out, DP);
      }
    }
  }
  else
  {
    MatZeroEntries(_amc_time_derivative_mat);
    MatZeroEntries(_amc_advective_derivative_mat);
    MatZeroEntries(_amc_cross_derivative_mat);
    MatZeroEntries(_amc_friction_force_mat);
    VecZeroEntries(_amc_time_derivative_rhs);
    VecZeroEntries(_amc_advective_derivative_rhs);
    VecZeroEntries(_amc_cross_derivative_rhs);
    VecZeroEntries(_amc_friction_force_rhs);
    VecZeroEntries(_amc_gravity_rhs);
    MatZeroEntries(_amc_sys_mdot_mat);
    VecZeroEntries(_amc_sys_mdot_rhs);

    for (unsigned int iz = first_node; iz < last_node + 1; iz++)
    {
      auto z_grid = _subchannel_mesh.getZGrid();
      auto k_grid = _subchannel_mesh.getKGrid();
      auto dz = z_grid[iz] - z_grid[iz - 1];
      auto iz_ind = iz - first_node;
      for (unsigned int i_ch = 0; i_ch < _n_channels; i_ch++)
      {
        // inlet and outlet nodes
        auto * node_in = _subchannel_mesh.getChannelNode(i_ch, iz - 1);
        auto * node_out = _subchannel_mesh.getChannelNode(i_ch, iz);

        // inlet, outlet, and interpolated density
        auto rho_in = (*_rho_soln)(node_in);
        auto rho_out = (*_rho_soln)(node_out);
        auto rho_interp =
            this->computeInterpolatedValue(rho_out, rho_in, "central_difference", 0.5);

        // inlet, outlet, and interpolated viscosity
        auto mu_in = (*_mu_soln)(node_in);
        auto mu_out = (*_mu_soln)(node_out);
        auto mu_interp = this->computeInterpolatedValue(mu_out, mu_in, "central_difference", 0.5);

        // inlet, outlet, and interpolated axial surface area
        auto S_in = (*_S_flow_soln)(node_in);
        auto S_out = (*_S_flow_soln)(node_out);
        auto S_interp = this->computeInterpolatedValue(S_out, S_in, "central_difference", 0.5);

        // inlet, outlet, and interpolated wetted perimeter
        auto w_perim_in = (*_w_perim_soln)(node_in);
        auto w_perim_out = (*_w_perim_soln)(node_out);
        auto w_perim_interp =
            this->computeInterpolatedValue(w_perim_out, w_perim_in, "central_difference", 0.5);

        // interpolation weight coefficient
        PetscScalar Pe = 0.5;
        auto alpha = computeInterpolationCoefficients("central_difference", Pe);

        // hydraulic diameter in the i direction
        auto Dh_i = 4.0 * S_interp / w_perim_interp;

        /// Time derivative term
        if (iz == first_node)
        {
          PetscScalar value_vec_tt = -1.0 * _TR * alpha * (*_mdot_soln)(node_in)*dz / _dt;
          PetscInt row_vec_tt = i_ch + _n_channels * iz_ind;
          VecSetValues(_amc_time_derivative_rhs, 1, &row_vec_tt, &value_vec_tt, ADD_VALUES);
        }
        else
        {
          PetscInt row_tt = i_ch + _n_channels * iz_ind;
          PetscInt col_tt = i_ch + _n_channels * (iz_ind - 1);
          PetscScalar value_tt = _TR * alpha * dz / _dt;
          MatSetValues(_amc_time_derivative_mat, 1, &row_tt, 1, &col_tt, &value_tt, INSERT_VALUES);
        }

        // Adding diagonal elements
        PetscInt row_tt = i_ch + _n_channels * iz_ind;
        PetscInt col_tt = i_ch + _n_channels * iz_ind;
        PetscScalar value_tt = _TR * (1.0 - alpha) * dz / _dt;
        MatSetValues(_amc_time_derivative_mat, 1, &row_tt, 1, &col_tt, &value_tt, INSERT_VALUES);

        // Adding RHS elements
        PetscScalar mdot_old_interp = computeInterpolatedValue(
            _mdot_soln->old(node_out), _mdot_soln->old(node_in), "central_difference", Pe);
        PetscScalar value_vec_tt = _TR * mdot_old_interp * dz / _dt;
        PetscInt row_vec_tt = i_ch + _n_channels * iz_ind;
        VecSetValues(_amc_time_derivative_rhs, 1, &row_vec_tt, &value_vec_tt, ADD_VALUES);

        /// Advective derivative term
        if (iz == first_node)
        {
          PetscScalar value_vec_at = std::pow((*_mdot_soln)(node_in), 2.0) / (S_in * rho_in);
          PetscInt row_vec_at = i_ch + _n_channels * iz_ind;
          VecSetValues(_amc_advective_derivative_rhs, 1, &row_vec_at, &value_vec_at, ADD_VALUES);
        }
        else
        {
          PetscInt row_at = i_ch + _n_channels * iz_ind;
          PetscInt col_at = i_ch + _n_channels * (iz_ind - 1);
          PetscScalar value_at = -1.0 * (*_mdot_soln)(node_in) / (S_in * rho_in);
          MatSetValues(
              _amc_advective_derivative_mat, 1, &row_at, 1, &col_at, &value_at, INSERT_VALUES);
        }

        // Adding diagonal elements
        PetscInt row_at = i_ch + _n_channels * iz_ind;
        PetscInt col_at = i_ch + _n_channels * iz_ind;
        PetscScalar value_at = (*_mdot_soln)(node_out) / (S_out * rho_out);
        MatSetValues(
            _amc_advective_derivative_mat, 1, &row_at, 1, &col_at, &value_at, INSERT_VALUES);

        /// Cross derivative term
        unsigned int counter = 0;
        unsigned int cross_index = iz; // iz-1;
        for (auto i_gap : _subchannel_mesh.getChannelGaps(i_ch))
        {
          auto chans = _subchannel_mesh.getGapNeighborChannels(i_gap);
          unsigned int ii_ch = chans.first;
          unsigned int jj_ch = chans.second;
          auto * node_in_i = _subchannel_mesh.getChannelNode(ii_ch, iz - 1);
          auto * node_in_j = _subchannel_mesh.getChannelNode(jj_ch, iz - 1);
          auto * node_out_i = _subchannel_mesh.getChannelNode(ii_ch, iz);
          auto * node_out_j = _subchannel_mesh.getChannelNode(jj_ch, iz);
          auto rho_i = computeInterpolatedValue(
              (*_rho_soln)(node_out_i), (*_rho_soln)(node_in_i), "central_difference", Pe);
          auto rho_j = computeInterpolatedValue(
              (*_rho_soln)(node_out_j), (*_rho_soln)(node_in_j), "central_difference", Pe);
          auto S_i = computeInterpolatedValue(
              (*_S_flow_soln)(node_out_i), (*_S_flow_soln)(node_in_i), "central_difference", Pe);
          auto S_j = computeInterpolatedValue(
              (*_S_flow_soln)(node_out_j), (*_S_flow_soln)(node_in_j), "central_difference", Pe);
          auto u_star = 0.0;
          // figure out donor axial velocity
          if (_Wij(i_gap, cross_index) > 0.0)
          {
            if (iz == first_node)
            {
              u_star = (*_mdot_soln)(node_in_i) / S_i / rho_i;
              PetscScalar value_vec_ct = -1.0 * alpha *
                                         _subchannel_mesh.getCrossflowSign(i_ch, counter) *
                                         _Wij(i_gap, cross_index) * u_star;
              PetscInt row_vec_ct = i_ch + _n_channels * iz_ind;
              VecSetValues(_amc_cross_derivative_rhs, 1, &row_vec_ct, &value_vec_ct, ADD_VALUES);
            }
            else
            {
              PetscScalar value_ct = alpha * _subchannel_mesh.getCrossflowSign(i_ch, counter) *
                                     _Wij(i_gap, cross_index) / S_i / rho_i;
              PetscInt row_ct = i_ch + _n_channels * iz_ind;
              PetscInt col_ct = ii_ch + _n_channels * (iz_ind - 1);
              MatSetValues(
                  _amc_cross_derivative_mat, 1, &row_ct, 1, &col_ct, &value_ct, ADD_VALUES);
            }
            PetscScalar value_ct = (1.0 - alpha) *
                                   _subchannel_mesh.getCrossflowSign(i_ch, counter) *
                                   _Wij(i_gap, cross_index) / S_i / rho_i;
            PetscInt row_ct = i_ch + _n_channels * iz_ind;
            PetscInt col_ct = ii_ch + _n_channels * iz_ind;
            MatSetValues(_amc_cross_derivative_mat, 1, &row_ct, 1, &col_ct, &value_ct, ADD_VALUES);
          }
          else if (_Wij(i_gap, cross_index) < 0.0) // _Wij=0 operations not necessary
          {
            if (iz == first_node)
            {
              u_star = (*_mdot_soln)(node_in_j) / S_j / rho_j;
              PetscScalar value_vec_ct = -1.0 * alpha *
                                         _subchannel_mesh.getCrossflowSign(i_ch, counter) *
                                         _Wij(i_gap, cross_index) * u_star;
              PetscInt row_vec_ct = i_ch + _n_channels * iz_ind;
              VecSetValues(_amc_cross_derivative_rhs, 1, &row_vec_ct, &value_vec_ct, ADD_VALUES);
            }
            else
            {
              PetscScalar value_ct = alpha * _subchannel_mesh.getCrossflowSign(i_ch, counter) *
                                     _Wij(i_gap, cross_index) / S_j / rho_j;
              PetscInt row_ct = i_ch + _n_channels * iz_ind;
              PetscInt col_ct = jj_ch + _n_channels * (iz_ind - 1);
              MatSetValues(
                  _amc_cross_derivative_mat, 1, &row_ct, 1, &col_ct, &value_ct, ADD_VALUES);
            }
            PetscScalar value_ct = (1.0 - alpha) *
                                   _subchannel_mesh.getCrossflowSign(i_ch, counter) *
                                   _Wij(i_gap, cross_index) / S_j / rho_j;
            PetscInt row_ct = i_ch + _n_channels * iz_ind;
            PetscInt col_ct = jj_ch + _n_channels * iz_ind;
            MatSetValues(_amc_cross_derivative_mat, 1, &row_ct, 1, &col_ct, &value_ct, ADD_VALUES);
          }

          if (iz == first_node)
          {
            PetscScalar value_vec_ct = -2.0 * alpha * (*_mdot_soln)(node_in)*_CT *
                                       _WijPrime(i_gap, cross_index) / (rho_interp * S_interp);
            value_vec_ct += alpha * (*_mdot_soln)(node_in_j)*_CT * _WijPrime(i_gap, cross_index) /
                            (rho_j * S_j);
            value_vec_ct += alpha * (*_mdot_soln)(node_in_i)*_CT * _WijPrime(i_gap, cross_index) /
                            (rho_i * S_i);
            PetscInt row_vec_ct = i_ch + _n_channels * iz_ind;
            VecSetValues(_amc_cross_derivative_rhs, 1, &row_vec_ct, &value_vec_ct, ADD_VALUES);
          }
          else
          {
            PetscScalar value_center_ct =
                2.0 * alpha * _CT * _WijPrime(i_gap, cross_index) / (rho_interp * S_interp);
            PetscInt row_ct = i_ch + _n_channels * iz_ind;
            PetscInt col_ct = i_ch + _n_channels * (iz_ind - 1);
            MatSetValues(
                _amc_cross_derivative_mat, 1, &row_ct, 1, &col_ct, &value_center_ct, ADD_VALUES);

            PetscScalar value_left_ct =
                -1.0 * alpha * _CT * _WijPrime(i_gap, cross_index) / (rho_j * S_j);
            row_ct = i_ch + _n_channels * iz_ind;
            col_ct = jj_ch + _n_channels * (iz_ind - 1);
            MatSetValues(
                _amc_cross_derivative_mat, 1, &row_ct, 1, &col_ct, &value_left_ct, ADD_VALUES);

            PetscScalar value_right_ct =
                -1.0 * alpha * _CT * _WijPrime(i_gap, cross_index) / (rho_i * S_i);
            row_ct = i_ch + _n_channels * iz_ind;
            col_ct = ii_ch + _n_channels * (iz_ind - 1);
            MatSetValues(
                _amc_cross_derivative_mat, 1, &row_ct, 1, &col_ct, &value_right_ct, ADD_VALUES);
          }

          PetscScalar value_center_ct =
              2.0 * (1.0 - alpha) * _CT * _WijPrime(i_gap, cross_index) / (rho_interp * S_interp);
          PetscInt row_ct = i_ch + _n_channels * iz_ind;
          PetscInt col_ct = i_ch + _n_channels * iz_ind;
          MatSetValues(
              _amc_cross_derivative_mat, 1, &row_ct, 1, &col_ct, &value_center_ct, ADD_VALUES);

          PetscScalar value_left_ct =
              -1.0 * (1.0 - alpha) * _CT * _WijPrime(i_gap, cross_index) / (rho_j * S_j);
          row_ct = i_ch + _n_channels * iz_ind;
          col_ct = jj_ch + _n_channels * iz_ind;
          MatSetValues(
              _amc_cross_derivative_mat, 1, &row_ct, 1, &col_ct, &value_left_ct, ADD_VALUES);

          PetscScalar value_right_ct =
              -1.0 * (1.0 - alpha) * _CT * _WijPrime(i_gap, cross_index) / (rho_i * S_i);
          row_ct = i_ch + _n_channels * iz_ind;
          col_ct = ii_ch + _n_channels * iz_ind;
          MatSetValues(
              _amc_cross_derivative_mat, 1, &row_ct, 1, &col_ct, &value_right_ct, ADD_VALUES);

          counter++;
        }

        /// Friction term
        PetscScalar mdot_interp = computeInterpolatedValue(
            (*_mdot_soln)(node_out), (*_mdot_soln)(node_in), "central_difference", Pe);
        auto Re = ((mdot_interp / S_interp) * Dh_i / mu_interp);
        auto fi = computeFrictionFactor(Re, i_ch, S_interp, w_perim_interp, Dh_i);
        auto ki = computeInterpolatedValue(
            k_grid[i_ch][iz], k_grid[i_ch][iz - 1], "central_difference", Pe);
        auto coef = (fi * dz / Dh_i + ki) * 0.5 * std::abs((*_mdot_soln)(node_out)) /
                    (S_interp * rho_interp);
        if (iz == first_node)
        {
          PetscScalar value_vec = -1.0 * alpha * coef * (*_mdot_soln)(node_in);
          PetscInt row_vec = i_ch + _n_channels * iz_ind;
          VecSetValues(_amc_friction_force_rhs, 1, &row_vec, &value_vec, ADD_VALUES);
        }
        else
        {
          PetscInt row = i_ch + _n_channels * iz_ind;
          PetscInt col = i_ch + _n_channels * (iz_ind - 1);
          PetscScalar value = alpha * coef;
          MatSetValues(_amc_friction_force_mat, 1, &row, 1, &col, &value, INSERT_VALUES);
        }

        // Adding diagonal elements
        PetscInt row = i_ch + _n_channels * iz_ind;
        PetscInt col = i_ch + _n_channels * iz_ind;
        PetscScalar value = (1.0 - alpha) * coef;
        MatSetValues(_amc_friction_force_mat, 1, &row, 1, &col, &value, INSERT_VALUES);

        /// Gravity force
        PetscScalar value_vec = -1.0 * _g_grav * rho_interp * dz * S_interp;
        PetscInt row_vec = i_ch + _n_channels * iz_ind;
        VecSetValues(_amc_gravity_rhs, 1, &row_vec, &value_vec, ADD_VALUES);
      }
    }
    /// Assembling system
    MatZeroEntries(_amc_sys_mdot_mat);
    VecZeroEntries(_amc_sys_mdot_rhs);
    MatAssemblyBegin(_amc_time_derivative_mat, MAT_FINAL_ASSEMBLY);
    MatAssemblyEnd(_amc_time_derivative_mat, MAT_FINAL_ASSEMBLY);
    MatAssemblyBegin(_amc_advective_derivative_mat, MAT_FINAL_ASSEMBLY);
    MatAssemblyEnd(_amc_advective_derivative_mat, MAT_FINAL_ASSEMBLY);
    MatAssemblyBegin(_amc_cross_derivative_mat, MAT_FINAL_ASSEMBLY);
    MatAssemblyEnd(_amc_cross_derivative_mat, MAT_FINAL_ASSEMBLY);
    MatAssemblyBegin(_amc_friction_force_mat, MAT_FINAL_ASSEMBLY);
    MatAssemblyEnd(_amc_friction_force_mat, MAT_FINAL_ASSEMBLY);
    MatAssemblyBegin(_amc_sys_mdot_mat, MAT_FINAL_ASSEMBLY);
    MatAssemblyEnd(_amc_sys_mdot_mat, MAT_FINAL_ASSEMBLY);
    // Matrix
#if !PETSC_VERSION_LESS_THAN(3, 15, 0)
    MatAXPY(_amc_sys_mdot_mat, 1.0, _amc_time_derivative_mat, UNKNOWN_NONZERO_PATTERN);
    MatAssemblyBegin(_amc_sys_mdot_mat, MAT_FINAL_ASSEMBLY);
    MatAssemblyEnd(_amc_sys_mdot_mat, MAT_FINAL_ASSEMBLY);
    MatAXPY(_amc_sys_mdot_mat, 1.0, _amc_advective_derivative_mat, UNKNOWN_NONZERO_PATTERN);
    MatAssemblyBegin(_amc_sys_mdot_mat, MAT_FINAL_ASSEMBLY);
    MatAssemblyEnd(_amc_sys_mdot_mat, MAT_FINAL_ASSEMBLY);
    MatAXPY(_amc_sys_mdot_mat, 1.0, _amc_cross_derivative_mat, UNKNOWN_NONZERO_PATTERN);
    MatAssemblyBegin(_amc_sys_mdot_mat, MAT_FINAL_ASSEMBLY);
    MatAssemblyEnd(_amc_sys_mdot_mat, MAT_FINAL_ASSEMBLY);
    MatAXPY(_amc_sys_mdot_mat, 1.0, _amc_friction_force_mat, UNKNOWN_NONZERO_PATTERN);
#else
    MatAXPY(_amc_sys_mdot_mat, 1.0, _amc_time_derivative_mat, DIFFERENT_NONZERO_PATTERN);
    MatAssemblyBegin(_amc_sys_mdot_mat, MAT_FINAL_ASSEMBLY);
    MatAssemblyEnd(_amc_sys_mdot_mat, MAT_FINAL_ASSEMBLY);
    MatAXPY(_amc_sys_mdot_mat, 1.0, _amc_advective_derivative_mat, DIFFERENT_NONZERO_PATTERN);
    MatAssemblyBegin(_amc_sys_mdot_mat, MAT_FINAL_ASSEMBLY);
    MatAssemblyEnd(_amc_sys_mdot_mat, MAT_FINAL_ASSEMBLY);
    MatAXPY(_amc_sys_mdot_mat, 1.0, _amc_cross_derivative_mat, DIFFERENT_NONZERO_PATTERN);
    MatAssemblyBegin(_amc_sys_mdot_mat, MAT_FINAL_ASSEMBLY);
    MatAssemblyEnd(_amc_sys_mdot_mat, MAT_FINAL_ASSEMBLY);
    MatAXPY(_amc_sys_mdot_mat, 1.0, _amc_friction_force_mat, DIFFERENT_NONZERO_PATTERN);
#endif
    MatAssemblyBegin(_amc_sys_mdot_mat, MAT_FINAL_ASSEMBLY);
    MatAssemblyEnd(_amc_sys_mdot_mat, MAT_FINAL_ASSEMBLY);
    if (_verbose_subchannel)
      _console << "Block: " << iblock << " - Linear momentum conservation matrix assembled"
               << std::endl;
    // RHS
    VecAXPY(_amc_sys_mdot_rhs, 1.0, _amc_time_derivative_rhs);
    VecAXPY(_amc_sys_mdot_rhs, 1.0, _amc_advective_derivative_rhs);
    VecAXPY(_amc_sys_mdot_rhs, 1.0, _amc_cross_derivative_rhs);
    VecAXPY(_amc_sys_mdot_rhs, 1.0, _amc_friction_force_rhs);
    VecAXPY(_amc_sys_mdot_rhs, 1.0, _amc_gravity_rhs);

    if (_segregated_bool)
    {
      // Assembly the matrix system
      populateVectorFromHandle<SolutionHandle *>(
          _prod, _mdot_soln, first_node, last_node, _n_channels);
      Vec ls;
      VecDuplicate(_amc_sys_mdot_rhs, &ls);
      MatMult(_amc_sys_mdot_mat, _prod, ls);
      VecAXPY(ls, -1.0, _amc_sys_mdot_rhs);
      PetscScalar * xx;
      VecGetArray(ls, &xx);
      for (unsigned int iz = first_node; iz < last_node + 1; iz++)
      {
        auto iz_ind = iz - first_node;
        for (unsigned int i_ch = 0; i_ch < _n_channels; i_ch++)
        {
          // Setting nodes
          auto * node_in = _subchannel_mesh.getChannelNode(i_ch, iz - 1);
          auto * node_out = _subchannel_mesh.getChannelNode(i_ch, iz);

          // inlet, outlet, and interpolated axial surface area
          auto S_in = (*_S_flow_soln)(node_in);
          auto S_out = (*_S_flow_soln)(node_out);
          auto S_interp = this->computeInterpolatedValue(S_out, S_in, "central_difference", 0.5);

          // Setting solutions
          if (S_interp != 0)
          {
            auto DP = std::pow(S_interp, -1.0) * xx[iz_ind * _n_channels + i_ch];
            _DP_soln->set(node_out, DP);
          }
          else
          {
            auto DP = 0.0;
            _DP_soln->set(node_out, DP);
          }
        }
      }
      VecDestroy(&ls);
    }
  }
}

double
LiquidMetalSubChannel1PhaseProblem::computeMassFlowForDPDZ(double dpdz, int i_ch)
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
    auto fi = computeFrictionFactor(Rei, i_ch, Si, w_perim, Dhi);
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
LiquidMetalSubChannel1PhaseProblem::computeh(int iblock)
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

  if (!_implicit_bool)
  {
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
        const Real & rod_diameter = _subchannel_mesh.getRodDiameter();
        auto * node_in = _subchannel_mesh.getChannelNode(i_ch, iz - 1);
        auto * node_out = _subchannel_mesh.getChannelNode(i_ch, iz);
        auto mdot_in = (*_mdot_soln)(node_in);
        auto h_in = (*_h_soln)(node_in); // J/kg
        auto volume = dz * (*_S_flow_soln)(node_in);
        auto mdot_out = (*_mdot_soln)(node_out);
        auto h_out = 0.0;
        double sumWijh = 0.0;
        double sumWijPrimeDhij = 0.0;
        Real e_cond = 0.0;

        double added_enthalpy;
        if (z_grid[iz] > unheated_length_entry &&
            z_grid[iz] <= unheated_length_entry + heated_length)
        {
          // added_enthalpy = ((*_q_prime_soln)(node_out) + (*_q_prime_soln)(node_in)) * dz / 2.0;
          added_enthalpy = computeAddedHeatPin(i_ch, iz);
        }
        else
          added_enthalpy = 0.0;

        added_enthalpy += computeAddedHeatDuct(i_ch, iz);

        // compute the sweep flow enthalpy change
        auto subch_type = _subchannel_mesh.getSubchannelType(i_ch);
        Real sweep_enthalpy = 0.0;

        if (subch_type == EChannelType::EDGE || subch_type == EChannelType::CORNER)
        {
          const Real & pitch = _subchannel_mesh.getPitch();
          const Real & rod_diameter = _subchannel_mesh.getRodDiameter();
          const Real & wire_lead_length = _tri_sch_mesh.getWireLeadLength();
          const Real & wire_diameter = _tri_sch_mesh.getWireDiameter();
          auto gap = _tri_sch_mesh.getDuctToRodGap();
          auto w = rod_diameter + gap;
          auto theta =
              std::acos(wire_lead_length /
                        std::sqrt(std::pow(wire_lead_length, 2) +
                                  std::pow(libMesh::pi * (rod_diameter + wire_diameter), 2)));
          // in/out channels for i_ch
          auto sweep_in = _tri_sch_mesh.getSweepFlowChans(i_ch).first;
          auto * node_sin = _subchannel_mesh.getChannelNode(sweep_in, iz - 1);
          auto cs_t = 0.75 * std::pow(wire_lead_length / rod_diameter, 0.3);
          auto ar2 = libMesh::pi * (rod_diameter + wire_diameter) * wire_diameter / 4.0;
          auto a2p =
              pitch * (w - rod_diameter / 2.0) - libMesh::pi * std::pow(rod_diameter, 2) / 8.0;
          auto Sij_in = dz * gap;
          auto Sij_out = dz * gap;
          auto wsweep_in = gedge_ave * cs_t * std::pow((ar2 / a2p), 0.5) * std::tan(theta) * Sij_in;
          auto wsweep_out =
              gedge_ave * cs_t * std::pow((ar2 / a2p), 0.5) * std::tan(theta) * Sij_out;
          auto sweep_hin = (*_h_soln)(node_sin);
          auto sweep_hout = (*_h_soln)(node_in);
          sweep_enthalpy = (wsweep_in * sweep_hin - wsweep_out * sweep_hout);
        }

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
          if (_Wij(i_gap, iz) > 0.0)
            h_star = (*_h_soln)(node_in_i);
          else if (_Wij(i_gap, iz) < 0.0)
            h_star = (*_h_soln)(node_in_j);
          // take care of the sign by applying the map, use donor cell
          sumWijh += _subchannel_mesh.getCrossflowSign(i_ch, counter) * _Wij(i_gap, iz) * h_star;
          sumWijPrimeDhij += _WijPrime(i_gap, iz) * (2 * (*_h_soln)(node_in) -
                                                     (*_h_soln)(node_in_j) - (*_h_soln)(node_in_i));
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
          auto shape_factor = 0.66 * (pitch / rod_diameter) *
                              std::pow((_subchannel_mesh.getGapWidth(i_gap) / rod_diameter), -0.3);
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
          e_cond += 0.5 * (thcon_i + thcon_j) * Si *
                    ((*_T_soln)(node_in_j) - (*_T_soln)(node_in_i)) / dist_ij;
        }

        // end of radial heat conduction calc.
        h_out =
            (mdot_in * h_in - sumWijh - sumWijPrimeDhij + added_enthalpy + e_cond + sweep_enthalpy +
             _TR * _rho_soln->old(node_out) * _h_soln->old(node_out) * volume / _dt) /
            (mdot_out + _TR * (*_rho_soln)(node_out)*volume / _dt);

        if (h_out < 0)
        {
          _console << "Wij = : " << _Wij << "\n";
          mooseError(name(),
                     " : Calculation of negative Enthalpy h_out = : ",
                     h_out,
                     " Axial Level= : ",
                     iz);
        }
        _h_soln->set(node_out, h_out); // J/kg
      }
    }
  }
  else
  {
    MatZeroEntries(_hc_time_derivative_mat);
    MatZeroEntries(_hc_advective_derivative_mat);
    MatZeroEntries(_hc_cross_derivative_mat);
    MatZeroEntries(_hc_axial_heat_conduction_mat);
    MatZeroEntries(_hc_radial_heat_conduction_mat);
    MatZeroEntries(_hc_sweep_enthalpy_mat);

    VecZeroEntries(_hc_time_derivative_rhs);
    VecZeroEntries(_hc_advective_derivative_rhs);
    VecZeroEntries(_hc_cross_derivative_rhs);
    VecZeroEntries(_hc_added_heat_rhs);
    VecZeroEntries(_hc_axial_heat_conduction_rhs);
    VecZeroEntries(_hc_radial_heat_conduction_rhs);
    VecZeroEntries(_hc_sweep_enthalpy_rhs);

    MatZeroEntries(_hc_sys_h_mat);
    VecZeroEntries(_hc_sys_h_rhs);

    for (unsigned int iz = first_node; iz < last_node + 1; iz++)
    {
      auto dz = _z_grid[iz] - _z_grid[iz - 1];
      auto heated_length = _subchannel_mesh.getHeatedLength();
      auto unheated_length_entry = _subchannel_mesh.getHeatedLengthEntry();
      auto pitch = _subchannel_mesh.getPitch();
      auto rod_diameter = _subchannel_mesh.getRodDiameter();
      auto iz_ind = iz - first_node;

      for (unsigned int i_ch = 0; i_ch < _n_channels; i_ch++)
      {
        auto * node_in = _subchannel_mesh.getChannelNode(i_ch, iz - 1);
        auto * node_out = _subchannel_mesh.getChannelNode(i_ch, iz);
        auto S_in = (*_S_flow_soln)(node_in);
        auto S_out = (*_S_flow_soln)(node_out);

        // interpolation weight coefficient
        PetscScalar Pe = 0.5;
        auto alpha = computeInterpolationCoefficients("central_difference", Pe);
        auto S_interp = computeInterpolatedValue(S_in, S_out, "central_difference", Pe);
        auto volume = dz * S_interp;

        /// Time derivative term
        if (iz == first_node)
        {
          PetscScalar value_vec_tt =
              -1.0 * _TR * alpha * (*_rho_soln)(node_in) * (*_h_soln)(node_in)*volume / _dt;
          PetscInt row_vec_tt = i_ch + _n_channels * iz_ind;
          VecSetValues(_hc_time_derivative_rhs, 1, &row_vec_tt, &value_vec_tt, ADD_VALUES);
        }
        else
        {
          PetscInt row_tt = i_ch + _n_channels * iz_ind;
          PetscInt col_tt = i_ch + _n_channels * (iz_ind - 1);
          PetscScalar value_tt = _TR * alpha * (*_rho_soln)(node_in)*volume / _dt;
          MatSetValues(_hc_time_derivative_mat, 1, &row_tt, 1, &col_tt, &value_tt, INSERT_VALUES);
        }

        // Adding diagonal elements
        PetscInt row_tt = i_ch + _n_channels * iz_ind;
        PetscInt col_tt = i_ch + _n_channels * iz_ind;
        PetscScalar value_tt = _TR * (1.0 - alpha) * (*_rho_soln)(node_out)*volume / _dt;
        MatSetValues(_hc_time_derivative_mat, 1, &row_tt, 1, &col_tt, &value_tt, INSERT_VALUES);

        // Adding RHS elements
        PetscScalar rho_old_interp = computeInterpolatedValue(
            _rho_soln->old(node_out), _rho_soln->old(node_in), "central_difference", Pe);
        PetscScalar h_old_interp = computeInterpolatedValue(
            _h_soln->old(node_out), _h_soln->old(node_in), "central_difference", Pe);
        PetscScalar value_vec_tt = _TR * rho_old_interp * h_old_interp * volume / _dt;
        PetscInt row_vec_tt = i_ch + _n_channels * iz_ind;
        VecSetValues(_hc_time_derivative_rhs, 1, &row_vec_tt, &value_vec_tt, ADD_VALUES);

        /// Advective derivative term
        if (iz == first_node)
        {
          PetscScalar value_vec_at = (*_mdot_soln)(node_in) * (*_h_soln)(node_in);
          PetscInt row_vec_at = i_ch + _n_channels * iz_ind;
          VecSetValues(_hc_advective_derivative_rhs, 1, &row_vec_at, &value_vec_at, ADD_VALUES);
        }
        else if (iz == last_node)
        {
          PetscInt row_at = i_ch + _n_channels * iz_ind;
          PetscInt col_at = i_ch + _n_channels * (iz_ind - 1);
          PetscScalar value_at = -1.0 * (*_mdot_soln)(node_in);
          MatSetValues(
              _hc_advective_derivative_mat, 1, &row_at, 1, &col_at, &value_at, INSERT_VALUES);
        }
        else
        {
          PetscInt row_at = i_ch + _n_channels * iz_ind;
          PetscInt col_at = i_ch + _n_channels * (iz_ind - 1);
          PetscScalar value_at = -1.0 * (*_mdot_soln)(node_in);
          MatSetValues(
              _hc_advective_derivative_mat, 1, &row_at, 1, &col_at, &value_at, INSERT_VALUES);
        }

        // Adding diagonal elements
        PetscInt row_at = i_ch + _n_channels * iz_ind;
        PetscInt col_at = i_ch + _n_channels * iz_ind;
        PetscScalar value_at = (*_mdot_soln)(node_out);
        MatSetValues(
            _hc_advective_derivative_mat, 1, &row_at, 1, &col_at, &value_at, INSERT_VALUES);

        /// Axial heat conduction
        auto * node_center = _subchannel_mesh.getChannelNode(i_ch, iz);
        auto K_center = _fp->k_from_p_T((*_P_soln)(node_center), (*_T_soln)(node_center));
        auto cp_center = _fp->cp_from_p_T((*_P_soln)(node_center), (*_T_soln)(node_center));
        auto diff_center = K_center / (cp_center + 1e-15);

        if (iz == first_node)
        {
          auto * node_top = _subchannel_mesh.getChannelNode(i_ch, iz + 1);
          auto * node_bottom = _subchannel_mesh.getChannelNode(i_ch, iz - 1);
          auto K_bottom = _fp->k_from_p_T((*_P_soln)(node_bottom), (*_T_soln)(node_bottom));
          auto K_top = _fp->k_from_p_T((*_P_soln)(node_top), (*_T_soln)(node_top));
          auto cp_bottom = _fp->cp_from_p_T((*_P_soln)(node_bottom), (*_T_soln)(node_bottom));
          auto cp_top = _fp->cp_from_p_T((*_P_soln)(node_top), (*_T_soln)(node_top));
          auto diff_bottom = K_bottom / (cp_bottom + 1e-15);
          auto diff_top = K_top / (cp_top + 1e-15);

          auto dz_up = _z_grid[iz + 1] - _z_grid[iz];
          auto dz_down = _z_grid[iz] - _z_grid[iz - 1];
          auto S_up = 0.5 * ((*_S_flow_soln)(node_top) + (*_S_flow_soln)(node_center)); // TODO:
          auto S_down = 0.5 * ((*_S_flow_soln)(node_center) + (*_S_flow_soln)(node_bottom));
          auto diff_up = 0.5 * (diff_top + diff_center);
          auto diff_down = 0.5 * (diff_center + diff_bottom);

          // Diagonal  value
          PetscInt row_at = i_ch + _n_channels * iz_ind;
          PetscInt col_at = i_ch + _n_channels * iz_ind;
          PetscScalar value_at = diff_up * S_up / dz_up + diff_down * S_down / dz_down;
          MatSetValues(
              _hc_axial_heat_conduction_mat, 1, &row_at, 1, &col_at, &value_at, INSERT_VALUES);

          // Bottom value
          value_at = 1.0 * diff_down * S_down / dz_down * (*_h_soln)(node_bottom);
          VecSetValues(_hc_axial_heat_conduction_rhs, 1, &row_at, &value_at, ADD_VALUES);

          // Top value
          col_at = i_ch + _n_channels * (iz_ind + 1);
          value_at = -diff_up * S_up / dz_up;
          MatSetValues(
              _hc_axial_heat_conduction_mat, 1, &row_at, 1, &col_at, &value_at, INSERT_VALUES);
        }
        else if (iz == last_node)
        {
          auto * node_bottom = _subchannel_mesh.getChannelNode(i_ch, iz - 1);
          auto K_bottom = _fp->k_from_p_T((*_P_soln)(node_bottom), (*_T_soln)(node_bottom));
          auto cp_bottom = _fp->cp_from_p_T((*_P_soln)(node_bottom), (*_T_soln)(node_bottom));
          auto diff_bottom = K_bottom / (cp_bottom + 1e-15);

          auto dz_down = _z_grid[iz] - _z_grid[iz - 1];
          auto S_down = 0.5 * ((*_S_flow_soln)(node_center) + (*_S_flow_soln)(node_bottom));
          auto diff_down = 0.5 * (diff_center + diff_bottom);

          // Diagonal  value
          PetscInt row_at = i_ch + _n_channels * iz_ind;
          PetscInt col_at = i_ch + _n_channels * iz_ind;
          PetscScalar value_at = diff_down * S_down / dz_down;
          MatSetValues(
              _hc_axial_heat_conduction_mat, 1, &row_at, 1, &col_at, &value_at, INSERT_VALUES);

          // Bottom value
          col_at = i_ch + _n_channels * (iz_ind - 1);
          value_at = -diff_down * S_down / dz_down;
          MatSetValues(
              _hc_axial_heat_conduction_mat, 1, &row_at, 1, &col_at, &value_at, INSERT_VALUES);

          // Outflow derivative
          /// TODO: Current axial derivative is zero - check if outflow conditions may make a difference
          // value_at = -1.0 * (*_mdot_soln)(node_center) * (*_h_soln)(node_center);
          // VecSetValues(_hc_axial_heat_conduction_rhs, 1, &row_at, &value_at, ADD_VALUES);
        }
        else
        {
          auto * node_top = _subchannel_mesh.getChannelNode(i_ch, iz + 1);
          auto * node_bottom = _subchannel_mesh.getChannelNode(i_ch, iz - 1);
          auto K_bottom = _fp->k_from_p_T((*_P_soln)(node_bottom), (*_T_soln)(node_bottom));
          auto K_top = _fp->k_from_p_T((*_P_soln)(node_top), (*_T_soln)(node_top));
          auto cp_bottom = _fp->cp_from_p_T((*_P_soln)(node_bottom), (*_T_soln)(node_bottom));
          auto cp_top = _fp->cp_from_p_T((*_P_soln)(node_top), (*_T_soln)(node_top));
          auto diff_bottom = K_bottom / (cp_bottom + 1e-15);
          auto diff_top = K_top / (cp_top + 1e-15);

          auto dz_up = _z_grid[iz + 1] - _z_grid[iz];
          auto dz_down = _z_grid[iz] - _z_grid[iz - 1];
          auto S_up = 0.5 * ((*_S_flow_soln)(node_top) + (*_S_flow_soln)(node_center)); // TODO:
          auto S_down = 0.5 * ((*_S_flow_soln)(node_center) + (*_S_flow_soln)(node_bottom));
          auto diff_up = 0.5 * (diff_top + diff_center);
          auto diff_down = 0.5 * (diff_center + diff_bottom);

          // Diagonal value
          PetscInt row_at = i_ch + _n_channels * iz_ind;
          PetscInt col_at = i_ch + _n_channels * iz_ind;
          PetscScalar value_at = diff_up * S_up / dz_up + diff_down * S_down / dz_down;
          MatSetValues(
              _hc_axial_heat_conduction_mat, 1, &row_at, 1, &col_at, &value_at, INSERT_VALUES);

          // Bottom value
          col_at = i_ch + _n_channels * (iz_ind - 1);
          value_at = -diff_down * S_down / dz_down;
          MatSetValues(
              _hc_axial_heat_conduction_mat, 1, &row_at, 1, &col_at, &value_at, INSERT_VALUES);

          // Top value
          col_at = i_ch + _n_channels * (iz_ind + 1);
          value_at = -diff_up * S_up / dz_up;
          MatSetValues(
              _hc_axial_heat_conduction_mat, 1, &row_at, 1, &col_at, &value_at, INSERT_VALUES);
        }

        /// Radial Terms
        unsigned int counter = 0;
        unsigned int cross_index = iz;
        // Real radial_heat_conduction(0.0);
        for (auto i_gap : _subchannel_mesh.getChannelGaps(i_ch))
        {
          auto chans = _subchannel_mesh.getGapNeighborChannels(i_gap);
          unsigned int ii_ch = chans.first;
          unsigned int jj_ch = chans.second;
          auto * node_in_i = _subchannel_mesh.getChannelNode(ii_ch, iz - 1);
          auto * node_in_j = _subchannel_mesh.getChannelNode(jj_ch, iz - 1);
          PetscScalar h_star;
          // figure out donor axial velocity
          if (_Wij(i_gap, cross_index) > 0.0)
          {
            if (iz == first_node)
            {
              h_star = (*_h_soln)(node_in_i);
              PetscScalar value_vec_ct = -1.0 * alpha *
                                         _subchannel_mesh.getCrossflowSign(i_ch, counter) *
                                         _Wij(i_gap, cross_index) * h_star;
              PetscInt row_vec_ct = i_ch + _n_channels * iz_ind;
              VecSetValues(_hc_cross_derivative_rhs, 1, &row_vec_ct, &value_vec_ct, ADD_VALUES);
            }
            else
            {
              PetscScalar value_ct = alpha * _subchannel_mesh.getCrossflowSign(i_ch, counter) *
                                     _Wij(i_gap, cross_index);
              PetscInt row_ct = i_ch + _n_channels * iz_ind;
              PetscInt col_ct = ii_ch + _n_channels * (iz_ind - 1);
              MatSetValues(_hc_cross_derivative_mat, 1, &row_ct, 1, &col_ct, &value_ct, ADD_VALUES);
            }
            PetscScalar value_ct = (1.0 - alpha) *
                                   _subchannel_mesh.getCrossflowSign(i_ch, counter) *
                                   _Wij(i_gap, cross_index);
            PetscInt row_ct = i_ch + _n_channels * iz_ind;
            PetscInt col_ct = ii_ch + _n_channels * iz_ind;
            MatSetValues(_hc_cross_derivative_mat, 1, &row_ct, 1, &col_ct, &value_ct, ADD_VALUES);
          }
          else if (_Wij(i_gap, cross_index) < 0.0) // _Wij=0 operations not necessary
          {
            if (iz == first_node)
            {
              h_star = (*_h_soln)(node_in_j);
              PetscScalar value_vec_ct = -1.0 * alpha *
                                         _subchannel_mesh.getCrossflowSign(i_ch, counter) *
                                         _Wij(i_gap, cross_index) * h_star;
              PetscInt row_vec_ct = i_ch + _n_channels * iz_ind;
              VecSetValues(_hc_cross_derivative_rhs, 1, &row_vec_ct, &value_vec_ct, ADD_VALUES);
            }
            else
            {
              PetscScalar value_ct = alpha * _subchannel_mesh.getCrossflowSign(i_ch, counter) *
                                     _Wij(i_gap, cross_index);
              PetscInt row_ct = i_ch + _n_channels * iz_ind;
              PetscInt col_ct = jj_ch + _n_channels * (iz_ind - 1);
              MatSetValues(_hc_cross_derivative_mat, 1, &row_ct, 1, &col_ct, &value_ct, ADD_VALUES);
            }
            PetscScalar value_ct = (1.0 - alpha) *
                                   _subchannel_mesh.getCrossflowSign(i_ch, counter) *
                                   _Wij(i_gap, cross_index);
            PetscInt row_ct = i_ch + _n_channels * iz_ind;
            PetscInt col_ct = jj_ch + _n_channels * iz_ind;
            MatSetValues(_hc_cross_derivative_mat, 1, &row_ct, 1, &col_ct, &value_ct, ADD_VALUES);
          }

          // Turbulent cross flows
          if (iz == first_node)
          {
            PetscScalar value_vec_ct =
                -2.0 * alpha * (*_h_soln)(node_in)*_WijPrime(i_gap, cross_index);
            value_vec_ct += alpha * (*_h_soln)(node_in_j)*_WijPrime(i_gap, cross_index);
            value_vec_ct += alpha * (*_h_soln)(node_in_i)*_WijPrime(i_gap, cross_index);
            PetscInt row_vec_ct = i_ch + _n_channels * iz_ind;
            VecSetValues(_hc_cross_derivative_rhs, 1, &row_vec_ct, &value_vec_ct, ADD_VALUES);
          }
          else
          {
            PetscScalar value_center_ct = 2.0 * alpha * _WijPrime(i_gap, cross_index);
            PetscInt row_ct = i_ch + _n_channels * iz_ind;
            PetscInt col_ct = i_ch + _n_channels * (iz_ind - 1);
            MatSetValues(
                _hc_cross_derivative_mat, 1, &row_ct, 1, &col_ct, &value_center_ct, ADD_VALUES);

            PetscScalar value_left_ct = -1.0 * alpha * _WijPrime(i_gap, cross_index);
            row_ct = i_ch + _n_channels * iz_ind;
            col_ct = jj_ch + _n_channels * (iz_ind - 1);
            MatSetValues(
                _hc_cross_derivative_mat, 1, &row_ct, 1, &col_ct, &value_left_ct, ADD_VALUES);

            PetscScalar value_right_ct = -1.0 * alpha * _WijPrime(i_gap, cross_index);
            row_ct = i_ch + _n_channels * iz_ind;
            col_ct = ii_ch + _n_channels * (iz_ind - 1);
            MatSetValues(
                _hc_cross_derivative_mat, 1, &row_ct, 1, &col_ct, &value_right_ct, ADD_VALUES);
          }
          PetscScalar value_center_ct = 2.0 * (1.0 - alpha) * _WijPrime(i_gap, cross_index);
          PetscInt row_ct = i_ch + _n_channels * iz_ind;
          PetscInt col_ct = i_ch + _n_channels * iz_ind;
          MatSetValues(
              _hc_cross_derivative_mat, 1, &row_ct, 1, &col_ct, &value_center_ct, ADD_VALUES);

          PetscScalar value_left_ct = -1.0 * (1.0 - alpha) * _WijPrime(i_gap, cross_index);
          row_ct = i_ch + _n_channels * iz_ind;
          col_ct = jj_ch + _n_channels * iz_ind;
          MatSetValues(
              _hc_cross_derivative_mat, 1, &row_ct, 1, &col_ct, &value_left_ct, ADD_VALUES);

          PetscScalar value_right_ct = -1.0 * (1.0 - alpha) * _WijPrime(i_gap, cross_index);
          row_ct = i_ch + _n_channels * iz_ind;
          col_ct = ii_ch + _n_channels * iz_ind;
          MatSetValues(
              _hc_cross_derivative_mat, 1, &row_ct, 1, &col_ct, &value_right_ct, ADD_VALUES);

          /// Radial heat conduction
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
          auto K_i = _fp->k_from_p_T((*_P_soln)(node_in_i), (*_T_soln)(node_in_i));
          auto K_j = _fp->k_from_p_T((*_P_soln)(node_in_j), (*_T_soln)(node_in_j));
          auto cp_i = _fp->cp_from_p_T((*_P_soln)(node_in_i), (*_T_soln)(node_in_i));
          auto cp_j = _fp->cp_from_p_T((*_P_soln)(node_in_j), (*_T_soln)(node_in_j));
          auto A_i = K_i / cp_i;
          auto A_j = K_j / cp_j;
          auto harm_A = 2.0 * A_i * A_j / (A_i + A_j);
          auto shape_factor = 0.66 * (pitch / rod_diameter) *
                              std::pow((_subchannel_mesh.getGapWidth(i_gap) / rod_diameter), -0.3);
          // auto base_value =  0.5 * (A_i + A_j) * Sij * shape_factor / dist_ij;
          auto base_value = harm_A * shape_factor * Sij / dist_ij;
          auto neg_base_value = -1.0 * base_value;

          row_ct = ii_ch + _n_channels * iz_ind;
          col_ct = ii_ch + _n_channels * iz_ind;
          MatSetValues(
              _hc_radial_heat_conduction_mat, 1, &row_ct, 1, &col_ct, &base_value, ADD_VALUES);

          row_ct = jj_ch + _n_channels * iz_ind;
          col_ct = jj_ch + _n_channels * iz_ind;
          MatSetValues(
              _hc_radial_heat_conduction_mat, 1, &row_ct, 1, &col_ct, &base_value, ADD_VALUES);

          row_ct = ii_ch + _n_channels * iz_ind;
          col_ct = jj_ch + _n_channels * iz_ind;
          MatSetValues(
              _hc_radial_heat_conduction_mat, 1, &row_ct, 1, &col_ct, &neg_base_value, ADD_VALUES);

          row_ct = jj_ch + _n_channels * iz_ind;
          col_ct = ii_ch + _n_channels * iz_ind;
          MatSetValues(
              _hc_radial_heat_conduction_mat, 1, &row_ct, 1, &col_ct, &neg_base_value, ADD_VALUES);
          counter++;
        }

        // compute the sweep flow enthalpy change
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
        auto subch_type = _subchannel_mesh.getSubchannelType(i_ch);
        PetscScalar sweep_enthalpy = 0.0;
        if (subch_type == EChannelType::EDGE || subch_type == EChannelType::CORNER)
        {
          const Real & pitch = _subchannel_mesh.getPitch();
          const Real & rod_diameter = _subchannel_mesh.getRodDiameter();
          const Real & wire_lead_length = _tri_sch_mesh.getWireLeadLength();
          const Real & wire_diameter = _tri_sch_mesh.getWireDiameter();
          auto gap = _tri_sch_mesh.getDuctToRodGap();
          auto w = rod_diameter + gap;
          auto theta =
              std::acos(wire_lead_length /
                        std::sqrt(std::pow(wire_lead_length, 2) +
                                  std::pow(libMesh::pi * (rod_diameter + wire_diameter), 2)));
          // in/out channels for i_ch
          auto sweep_in = _tri_sch_mesh.getSweepFlowChans(i_ch).first;
          auto * node_sin = _subchannel_mesh.getChannelNode(sweep_in, iz - 1);
          auto cs_t = 0.75 * std::pow(wire_lead_length / rod_diameter, 0.3);
          auto ar2 = libMesh::pi * (rod_diameter + wire_diameter) * wire_diameter / 4.0;
          auto a2p =
              pitch * (w - rod_diameter / 2.0) - libMesh::pi * std::pow(rod_diameter, 2) / 8.0;
          auto Sij_in = dz * gap;
          auto Sij_out = dz * gap;
          auto wsweep_in = gedge_ave * cs_t * std::pow((ar2 / a2p), 0.5) * std::tan(theta) * Sij_in;
          auto wsweep_out =
              gedge_ave * cs_t * std::pow((ar2 / a2p), 0.5) * std::tan(theta) * Sij_out;
          auto sweep_hin = (*_h_soln)(node_sin);
          auto sweep_hout = (*_h_soln)(node_in);
          sweep_enthalpy = (wsweep_in * sweep_hin - wsweep_out * sweep_hout);

          if (iz == first_node)
          {
            PetscInt row_sh = i_ch + _n_channels * iz_ind;
            PetscScalar value_hs = -sweep_enthalpy;
            VecSetValues(_hc_sweep_enthalpy_rhs, 1, &row_sh, &value_hs, ADD_VALUES);
          }
          else
          {
            PetscInt row_sh = i_ch + _n_channels * (iz_ind - 1);
            PetscInt col_sh = i_ch + _n_channels * (iz_ind - 1);
            MatSetValues(_hc_sweep_enthalpy_mat, 1, &row_sh, 1, &col_sh, &wsweep_out, ADD_VALUES);
            PetscInt col_sh_l = sweep_in + _n_channels * (iz_ind - 1);
            PetscScalar neg_sweep_in = -1.0 * wsweep_in;
            MatSetValues(
                _hc_sweep_enthalpy_mat, 1, &row_sh, 1, &col_sh_l, &(neg_sweep_in), ADD_VALUES);
          }
        }

        /// Add heat enthalpy from pin
        PetscScalar added_enthalpy;
        if (_z_grid[iz] > unheated_length_entry &&
            _z_grid[iz] <= unheated_length_entry + heated_length)
          added_enthalpy = computeAddedHeatPin(i_ch, iz);
        else
          added_enthalpy = 0.0;
        PetscInt row_vec_ht = i_ch + _n_channels * iz_ind;
        VecSetValues(_hc_added_heat_rhs, 1, &row_vec_ht, &added_enthalpy, ADD_VALUES);
      }
    }
    /// Assembling system
    MatAssemblyBegin(_hc_time_derivative_mat, MAT_FINAL_ASSEMBLY);
    MatAssemblyEnd(_hc_time_derivative_mat, MAT_FINAL_ASSEMBLY);
    MatAssemblyBegin(_hc_advective_derivative_mat, MAT_FINAL_ASSEMBLY);
    MatAssemblyEnd(_hc_advective_derivative_mat, MAT_FINAL_ASSEMBLY);
    MatAssemblyBegin(_hc_cross_derivative_mat, MAT_FINAL_ASSEMBLY);
    MatAssemblyEnd(_hc_cross_derivative_mat, MAT_FINAL_ASSEMBLY);
    MatAssemblyBegin(_hc_axial_heat_conduction_mat, MAT_FINAL_ASSEMBLY);
    MatAssemblyEnd(_hc_axial_heat_conduction_mat, MAT_FINAL_ASSEMBLY);
    MatAssemblyBegin(_hc_radial_heat_conduction_mat, MAT_FINAL_ASSEMBLY);
    MatAssemblyEnd(_hc_radial_heat_conduction_mat, MAT_FINAL_ASSEMBLY);
    MatAssemblyBegin(_hc_sweep_enthalpy_mat, MAT_FINAL_ASSEMBLY);
    MatAssemblyEnd(_hc_sweep_enthalpy_mat, MAT_FINAL_ASSEMBLY);
    MatAssemblyBegin(_hc_sys_h_mat, MAT_FINAL_ASSEMBLY);
    MatAssemblyEnd(_hc_sys_h_mat, MAT_FINAL_ASSEMBLY);
    // Matrix
    // #if !PETSC_VERSION_LESS_THAN(3, 15, 0)
    //   MatAXPY(_hc_sys_h_mat, 1.0, _hc_time_derivative_mat, UNKNOWN_NONZERO_PATTERN);
    //   MatAssemblyBegin(_hc_sys_h_mat, MAT_FINAL_ASSEMBLY);
    //   MatAssemblyEnd(_hc_sys_h_mat, MAT_FINAL_ASSEMBLY);
    //   MatAXPY(_hc_sys_h_mat, 1.0, _hc_advective_derivative_mat, UNKNOWN_NONZERO_PATTERN);
    //   MatAssemblyBegin(_hc_sys_h_mat, MAT_FINAL_ASSEMBLY);
    //   MatAssemblyEnd(_hc_sys_h_mat, MAT_FINAL_ASSEMBLY);
    //   MatAXPY(_hc_sys_h_mat, 1.0, _hc_cross_derivative_mat, UNKNOWN_NONZERO_PATTERN);
    //   MatAssemblyBegin(_hc_sys_h_mat, MAT_FINAL_ASSEMBLY);
    //   MatAssemblyEnd(_hc_sys_h_mat, MAT_FINAL_ASSEMBLY);
    //   MatAXPY(_hc_sys_h_mat, 1.0, _hc_axial_heat_conduction_mat, UNKNOWN_NONZERO_PATTERN);
    //   MatAssemblyBegin(_hc_sys_h_mat, MAT_FINAL_ASSEMBLY);
    //   MatAssemblyEnd(_hc_sys_h_mat, MAT_FINAL_ASSEMBLY);
    //   MatAXPY(_hc_sys_h_mat, 1.0, _hc_radial_heat_conduction_mat, UNKNOWN_NONZERO_PATTERN);
    // #else
    MatAXPY(_hc_sys_h_mat, 1.0, _hc_time_derivative_mat, DIFFERENT_NONZERO_PATTERN);
    MatAssemblyBegin(_hc_sys_h_mat, MAT_FINAL_ASSEMBLY);
    MatAssemblyEnd(_hc_sys_h_mat, MAT_FINAL_ASSEMBLY);
    MatAXPY(_hc_sys_h_mat, 1.0, _hc_advective_derivative_mat, DIFFERENT_NONZERO_PATTERN);
    MatAssemblyBegin(_hc_sys_h_mat, MAT_FINAL_ASSEMBLY);
    MatAssemblyEnd(_hc_sys_h_mat, MAT_FINAL_ASSEMBLY);
    MatAXPY(_hc_sys_h_mat, 1.0, _hc_cross_derivative_mat, DIFFERENT_NONZERO_PATTERN);
    MatAssemblyBegin(_hc_sys_h_mat, MAT_FINAL_ASSEMBLY);
    MatAssemblyEnd(_hc_sys_h_mat, MAT_FINAL_ASSEMBLY);
    MatAXPY(_hc_sys_h_mat, 1.0, _hc_axial_heat_conduction_mat, DIFFERENT_NONZERO_PATTERN);
    MatAssemblyBegin(_hc_sys_h_mat, MAT_FINAL_ASSEMBLY);
    MatAssemblyEnd(_hc_sys_h_mat, MAT_FINAL_ASSEMBLY);
    MatAXPY(_hc_sys_h_mat, 1.0, _hc_radial_heat_conduction_mat, DIFFERENT_NONZERO_PATTERN);
    MatAssemblyBegin(_hc_sys_h_mat, MAT_FINAL_ASSEMBLY);
    MatAssemblyEnd(_hc_sys_h_mat, MAT_FINAL_ASSEMBLY);
    MatAXPY(_hc_sys_h_mat, 1.0, _hc_sweep_enthalpy_mat, DIFFERENT_NONZERO_PATTERN);
    // #endif
    MatAssemblyBegin(_hc_sys_h_mat, MAT_FINAL_ASSEMBLY);
    MatAssemblyEnd(_hc_sys_h_mat, MAT_FINAL_ASSEMBLY);
    if (_verbose_subchannel)
      _console << "Block: " << iblock << " - Enthalpy conservation matrix assembled" << std::endl;
    // RHS
    VecAXPY(_hc_sys_h_rhs, 1.0, _hc_time_derivative_rhs);
    VecAXPY(_hc_sys_h_rhs, 1.0, _hc_advective_derivative_rhs);
    VecAXPY(_hc_sys_h_rhs, 1.0, _hc_cross_derivative_rhs);
    VecAXPY(_hc_sys_h_rhs, 1.0, _hc_added_heat_rhs);
    VecAXPY(_hc_sys_h_rhs, 1.0, _hc_axial_heat_conduction_rhs);
    VecAXPY(_hc_sys_h_rhs, 1.0, _hc_radial_heat_conduction_rhs);
    VecAXPY(_hc_sys_h_rhs, 1.0, _hc_sweep_enthalpy_rhs);

    // MatView(_hc_sys_h_mat, PETSC_VIEWER_STDOUT_WORLD);
    // VecView(_hc_sys_h_rhs, PETSC_VIEWER_STDOUT_WORLD);

    if (_segregated_bool || (!_monolithic_thermal_bool))
    {
      // Assembly the matrix system
      KSP ksploc;
      PC pc;
      Vec sol;
      VecDuplicate(_hc_sys_h_rhs, &sol);
      KSPCreate(PETSC_COMM_WORLD, &ksploc);
      KSPSetOperators(ksploc, _hc_sys_h_mat, _hc_sys_h_mat);
      KSPGetPC(ksploc, &pc);
      PCSetType(pc, PCJACOBI);
      KSPSetTolerances(ksploc, _rtol, _atol, _dtol, _maxit);
      KSPSetFromOptions(ksploc);
      KSPSolve(ksploc, _hc_sys_h_rhs, sol);
      // VecView(sol, PETSC_VIEWER_STDOUT_WORLD);
      PetscScalar * xx;
      VecGetArray(sol, &xx);
      for (unsigned int iz = first_node; iz < last_node + 1; iz++)
      {
        auto iz_ind = iz - first_node;
        for (unsigned int i_ch = 0; i_ch < _n_channels; i_ch++)
        {
          auto * node_out = _subchannel_mesh.getChannelNode(i_ch, iz);
          _h_soln->set(node_out, xx[iz_ind * _n_channels + i_ch]);
        }
      }
      KSPDestroy(&ksploc);
      VecDestroy(&sol);
    }
  }
}

void
LiquidMetalSubChannel1PhaseProblem::externalSolve()
{
  _console << "Executing subchannel solver\n";
  auto P_error = 1.0;
  unsigned int P_it = 0;
  unsigned int P_it_max;

  if (_segregated_bool)
    P_it_max = 2 * _n_blocks;
  else
    P_it_max = 300;

  if ((_n_blocks == 1) && (_segregated_bool))
    P_it_max = 5;
  if (!_segregated_bool)
  {
    initializeSolution();
    if (_verbose_subchannel)
      _console << "Solution initialized" << std::endl;
  }
  while ((P_error > _P_tol && P_it < P_it_max))
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

        if (_segregated_bool)
        {
          computeWijFromSolve(iblock);
          if (_compute_power)
          {
            computeh(iblock);
            computeT(iblock);
          }
        }
        else
        {
          if (_monolithic_thermal_bool)
          {
            implicitPetscSolve(iblock);
            computeT(iblock);
          }
          else
          {
            implicitPetscSolve(iblock);
            if (_verbose_subchannel)
              _console << "Done with main solve." << std::endl;
            if (_compute_power)
            {
              if (_verbose_subchannel)
                _console << "Starting enthalpy solve." << std::endl;
              computeh(iblock);
              if (_verbose_subchannel)
                _console << "Done with enthalpy solve." << std::endl;
              computeT(iblock);
            }
            if (_verbose_subchannel)
              _console << "Done with thermal solve." << std::endl;
          }
        }

        if (_compute_density)
          computeRho(iblock);

        if (_compute_viscosity)
          computeMu(iblock);

        if (_verbose_subchannel)
          _console << "Done updating thermophysical properties." << std::endl;

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
    if (_verbose_subchannel)
    {
      _console << "Iteration:  " << P_it << std::endl;
      _console << "Maximum iterations: " << P_it_max << std::endl;
    }
  }
  // update old crossflow matrix
  _Wij_old = _Wij;
  _console << "Finished executing subchannel solver\n";

  /// Assigning temperature to the fuel pins
  if (_pin_mesh_exist)
  {
    _console << "Commencing calculation of Pin surface temperature" << std::endl;
    for (unsigned int i_pin = 0; i_pin < _n_pins; i_pin++)
    {
      for (unsigned int iz = 0; iz < _n_cells + 1; ++iz)
      {
        auto * pin_node = _subchannel_mesh.getPinNode(i_pin, iz);
        Real sumTemp = 0.0;
        Real rod_counter = 0.0;
        // Calculate sum of pin surface temperatures that the channels around the pin see
        for (auto i_ch : _subchannel_mesh.getPinChannels(i_pin))
        {
          auto * node = _subchannel_mesh.getChannelNode(i_ch, iz);

          auto mu = (*_mu_soln)(node);
          auto S = (*_S_flow_soln)(node);
          auto w_perim = (*_w_perim_soln)(node);
          auto Dh_i = 4.0 * S / w_perim;
          auto Re = (((*_mdot_soln)(node) / S) * Dh_i / mu);

          auto k = _fp->k_from_p_T((*_P_soln)(node) + _P_out, (*_T_soln)(node));
          auto cp = _fp->cp_from_p_T((*_P_soln)(node) + _P_out, (*_T_soln)(node));
          auto Pr = (*_mu_soln)(node)*cp / k;

          auto Nu = 0.023 * std::pow(Re, 0.8) * std::pow(Pr, 0.4);
          auto hw = Nu * k / Dh_i;

          sumTemp += (*_q_prime_soln)(pin_node) / (_subchannel_mesh.getRodDiameter() * M_PI * hw) +
                     (*_T_soln)(node);

          rod_counter += 1.0;
        }

        _Tpin_soln->set(pin_node, sumTemp / rod_counter);
      }
    }
  }

  /// Assigning temperatures to duct
  if (_duct_mesh_exist)
  {
    _console << "Commencing calculation of duct surface temperature " << std::endl;
    /// TODO: looping over the channels omits the corners - should check later
    //    for (unsigned int i_ch = 0; i_ch < _n_channels; i_ch++)
    //    {
    //      for (unsigned int iz = 0; iz < _n_cells + 1; ++iz)
    //        {
    //          auto subch_type = _subchannel_mesh.getSubchannelType(i_ch);
    //          if (subch_type == EChannelType::EDGE || subch_type == EChannelType::CORNER)
    //          {
    //            //auto dz = _z_grid[iz] - _z_grid[iz - 1];
    //            auto * node_chan = _subchannel_mesh.getChannelNode(i_ch, iz);
    //            auto * node_duct = _subchannel_mesh.getDuctNodeFromChannel(node_chan);
    //            auto T_chan = (*_T_soln)(node_chan);
    //            //_console << "T_chan: " << T_chan << std::endl;
    //            _Tduct_soln->set(node_duct, T_chan);
    //          }
    //        }
    //    }
    auto duct_nodes = _subchannel_mesh.getDuctNodes();
    for (Node * dn : duct_nodes)
    {

      auto * node_chan = _subchannel_mesh.getChannelNodeFromDuct(dn);

      auto mu = (*_mu_soln)(node_chan);
      auto S = (*_S_flow_soln)(node_chan);
      auto w_perim = (*_w_perim_soln)(node_chan);
      auto Dh_i = 4.0 * S / w_perim;
      auto Re = (((*_mdot_soln)(node_chan) / S) * Dh_i / mu);

      auto k = _fp->k_from_p_T((*_P_soln)(node_chan) + _P_out, (*_T_soln)(node_chan));
      auto cp = _fp->cp_from_p_T((*_P_soln)(node_chan) + _P_out, (*_T_soln)(node_chan));
      auto Pr = (*_mu_soln)(node_chan)*cp / k;

      auto Nu = 0.023 * std::pow(Re, 0.8) * std::pow(Pr, 0.4);
      auto hw = Nu * k / Dh_i;

      auto T_chan = (*_q_prime_duct_soln)(dn) / (_subchannel_mesh.getRodDiameter() * M_PI * hw) +
                    (*_T_soln)(node_chan);
      ;
      _Tduct_soln->set(dn, T_chan);
    }
  }
  _aux->solution().close();

  auto power_in = 0.0;
  auto power_out = 0.0;
  auto Total_surface_area = 0.0;
  auto mass_flow_in = 0.0;
  auto mass_flow_out = 0.0;
  for (unsigned int i_ch = 0; i_ch < _n_channels; i_ch++)
  {
    auto * node_in = _subchannel_mesh.getChannelNode(i_ch, 0);
    auto * node_out = _subchannel_mesh.getChannelNode(i_ch, _n_cells);
    Total_surface_area += (*_S_flow_soln)(node_in);
    power_in += (*_mdot_soln)(node_in) * (*_h_soln)(node_in);
    power_out += (*_mdot_soln)(node_out) * (*_h_soln)(node_out);
    mass_flow_in += (*_mdot_soln)(node_in);
    mass_flow_out += (*_mdot_soln)(node_out);
  }

  auto h_bulk_out = power_out / mass_flow_out;
  auto T_bulk_out = _fp->T_from_p_h(_P_out, h_bulk_out);

  _console << "Bulk sodium temperature at the bundle outlet :" << T_bulk_out << std::endl;
  _console << "Power added to coolant is: " << power_out - power_in << " Watt" << std::endl;
  _console << "Mass balance is: " << mass_flow_out - mass_flow_in << " Kg/sec" << std::endl;
  _console << " ============================ " << std::endl;
}
