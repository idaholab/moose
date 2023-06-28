/********************************************************************/
/*                   DO NOT MODIFY THIS HEADER                      */
/*          Subchannel: Thermal Hydraulics Reactor Analysis         */
/*                                                                  */
/*              (c) 2022 Battelle Energy Alliance, LLC              */
/*                      ALL RIGHTS RESERVED                         */
/*                                                                  */
/*             Prepared by Battelle Energy Alliance, LLC            */
/*               Under Contract No. DE-AC07-05ID14517               */
/*               With the U. S. Department of Energy                */
/*                                                                  */
/*               See COPYRIGHT for full restrictions                */
/********************************************************************/

#include "LiquidWaterSubChannel1PhaseProblem.h"

registerMooseObject("SubChannelApp", LiquidWaterSubChannel1PhaseProblem);

InputParameters
LiquidWaterSubChannel1PhaseProblem::validParams()
{
  InputParameters params = SubChannel1PhaseProblem::validParams();
  params.addRequiredParam<Real>("beta",
                                "Thermal diffusion coefficient used in turbulent crossflow. This "
                                "parameter in not user defined in triangular subchannels");
  params.addParam<bool>(
      "default_friction_model", true, "Boolean to define which friction model to use");
  params.addParam<bool>(
      "constant_beta", true, "Boolean to define the use of constant beta or computeBeta");
  return params;
}

LiquidWaterSubChannel1PhaseProblem::LiquidWaterSubChannel1PhaseProblem(
    const InputParameters & params)
  : SubChannel1PhaseProblem(params),
    _subchannel_mesh(dynamic_cast<QuadSubChannelMesh &>(_mesh)),
    _beta(getParam<Real>("beta")),
    _default_friction_model(getParam<bool>("default_friction_model")),
    _constant_beta(getParam<bool>("constant_beta"))
{
}

Real
LiquidWaterSubChannel1PhaseProblem::computeFrictionFactor(_friction_args_struct friction_args)
{
  auto Re = friction_args.Re;
  auto i_ch = friction_args.i_ch;
  if (_default_friction_model)
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
  else
  {
    Real aL, b1L, b2L, CL, ratio;
    Real aT, b1T, b2T, CT;
    auto pitch = _subchannel_mesh.getPitch();
    auto rod_diameter = _subchannel_mesh.getRodDiameter();
    auto gap = _subchannel_mesh.getGap();
    auto w = (rod_diameter / 2.0) + (pitch / 2.0) + gap;
    auto p_over_d = pitch / rod_diameter;
    auto w_over_d = w / rod_diameter;
    auto ReL = std::pow(10, (p_over_d - 1)) * 320.0;
    auto ReT = std::pow(10, 0.7 * (p_over_d - 1)) * 1.0E+4;
    auto psi = std::log(Re / ReL) / std::log(ReT / ReL);
    auto subch_type = _subchannel_mesh.getSubchannelType(i_ch);
    const Real lambda = 7.0;

    if (subch_type == EChannelType::CORNER)
    {
      ratio = w_over_d;
      if ((1.0 <= p_over_d) && (p_over_d <= 1.1))
      {
        aL = 28.62, b1L = 715.9, b2L = -2807;
        aT = 0.09755, b1T = 1.127, b2T = -6.304;
      }
      else if ((1.1 < p_over_d) && (p_over_d <= 1.5))
      {
        aL = 58.83, b1L = 160.7, b2L = -203.5;
        aT = 0.1452, b1T = 0.02681, b2T = -0.03411;
      }
      else
        mooseError(" Geometric parameters beyond scope of friction factor model ");
    }
    else if (subch_type == EChannelType::EDGE)
    {
      ratio = w_over_d;
      if ((1.0 <= p_over_d) && (p_over_d <= 1.1))
      {
        aL = 26.18, b1L = 554.5, b2L = -1480;
        aT = 0.09377, b1T = 0.8732, b2T = -3.341;
      }
      else if ((1.1 < p_over_d) && (p_over_d <= 1.5))
      {
        aL = 44.40, b1L = 256.7, b2L = -267.6;
        aT = 0.1430, b1T = 0.04199, b2T = -0.04428;
      }
      else
        mooseError(" Geometric parameters beyond scope of friction factor model ");
    }
    else
    {
      ratio = p_over_d;
      if ((1.0 <= p_over_d) && (p_over_d <= 1.1))
      {
        aL = 26.37, b1L = 374.2, b2L = -493.9;
        aT = 0.09423, b1T = 0.5806, b2T = -1.239;
      }
      else if ((1.1 < p_over_d) && (p_over_d <= 1.5))
      {
        aL = 35.55, b1L = 263.7, b2L = -190.2;
        aT = 0.1339, b1T = 0.09059, b2T = -0.09926;
      }
      else
        mooseError(" Geometric parameters beyond scope of friction factor model ");
    }

    CL = aL + b1L * (ratio - 1) + b2L * (ratio - 1) * (ratio - 1);
    CT = aT + b1T * (ratio - 1) + b2T * (ratio - 1) * (ratio - 1);

    auto fL = CL / Re;
    auto fT = CT * std::pow(Re, -0.18);

    if (Re < ReL)
    {
      // laminar flow
      return fL;
    }
    else if (Re > ReT)
    {
      // turbulent flow
      return fT;
    }
    else
    {
      // transition flow
      return fL * std::pow((1 - psi), 1.0 / 3.0) * (1 - std::pow(psi, lambda)) +
             fT * std::pow(psi, 1.0 / 3.0);
    }
  }
}

void
LiquidWaterSubChannel1PhaseProblem::computeWijPrime(int iblock)
{
  unsigned int last_node = (iblock + 1) * _block_size;
  unsigned int first_node = iblock * _block_size + 1;
  const Real & pitch = _subchannel_mesh.getPitch();
  const Real & rod_diameter = _subchannel_mesh.getRodDiameter();
  if (!_implicit_bool)
  {
    for (unsigned int iz = first_node; iz < last_node + 1; iz++)
    {
      auto dz = _z_grid[iz] - _z_grid[iz - 1];
      for (unsigned int i_gap = 0; i_gap < _n_gaps; i_gap++)
      {
        auto chans = _subchannel_mesh.getGapNeighborChannels(i_gap);
        unsigned int i_ch = chans.first;
        unsigned int j_ch = chans.second;
        auto * node_in_i = _subchannel_mesh.getChannelNode(i_ch, iz - 1);
        auto * node_out_i = _subchannel_mesh.getChannelNode(i_ch, iz);
        auto * node_in_j = _subchannel_mesh.getChannelNode(j_ch, iz - 1);
        auto * node_out_j = _subchannel_mesh.getChannelNode(j_ch, iz);
        auto Si_in = (*_S_flow_soln)(node_in_i);
        auto Sj_in = (*_S_flow_soln)(node_in_j);
        auto Si_out = (*_S_flow_soln)(node_out_i);
        auto Sj_out = (*_S_flow_soln)(node_out_j);
        // crossflow area between channels i,j (dz*gap_width)
        auto gap = _subchannel_mesh.getGapWidth(i_gap);
        auto Sij = dz * gap;
        auto avg_massflux =
            0.5 * (((*_mdot_soln)(node_in_i) + (*_mdot_soln)(node_in_j)) / (Si_in + Sj_in) +
                   ((*_mdot_soln)(node_out_i) + (*_mdot_soln)(node_out_j)) / (Si_out + Sj_out));

        auto beta = _beta;
        if (!_constant_beta)
        {
          auto w_perim_i = (*_w_perim_soln)(node_in_i);
          auto w_perim_j = (*_w_perim_soln)(node_in_j);
          auto mu_i = (*_mu_soln)(node_in_i);
          auto mu_j = (*_mu_soln)(node_in_j);
          // hydraulic diameter in the i direction
          auto hD_i = 4.0 * Si_in / w_perim_i;
          auto hD_j = 4.0 * Sj_in / w_perim_j;
          auto avg_hD = 0.5 * (hD_i + hD_j);
          auto avg_mu = 0.5 * (mu_i + mu_j);
          auto Re = avg_massflux * avg_hD / avg_mu;

          Real gamma = 20.0; // empirical constant
          Real sf = 1.0;     // shape factor
          Real a = 0.18;
          Real b = 0.2;
          auto f = a * std::pow(Re, -b); // Rehme 1992 circular tube friction factor
          auto k =
              0.25 * (_fp->k_from_p_T((*_P_soln)(node_out_i) + _P_out, (*_T_soln)(node_out_i)) +
                      _fp->k_from_p_T((*_P_soln)(node_in_i) + _P_out, (*_T_soln)(node_in_i)) +
                      _fp->k_from_p_T((*_P_soln)(node_out_j) + _P_out, (*_T_soln)(node_out_j)) +
                      _fp->k_from_p_T((*_P_soln)(node_in_j) + _P_out, (*_T_soln)(node_in_j)));
          auto cp =
              0.25 * (_fp->cp_from_p_T((*_P_soln)(node_out_i) + _P_out, (*_T_soln)(node_out_i)) +
                      _fp->cp_from_p_T((*_P_soln)(node_in_i) + _P_out, (*_T_soln)(node_in_i)) +
                      _fp->cp_from_p_T((*_P_soln)(node_out_j) + _P_out, (*_T_soln)(node_out_j)) +
                      _fp->cp_from_p_T((*_P_soln)(node_in_j) + _P_out, (*_T_soln)(node_in_j)));
          auto Pr = avg_mu * cp / k;                          // Prandtl number
          auto Pr_t = Pr * (Re / gamma) * std::sqrt(f / 8.0); // Turbulent Prandtl number
          auto delta = pitch;                                 // centroid to centroid distance
          auto L_x = sf * delta;  // axial length scale (gap is the lateral length scale)
          auto lamda = gap / L_x; // aspect ratio
          auto a_x = 1.0 - 2.0 * lamda * lamda / libMesh::pi; // velocity coefficient
          auto z_FP_over_D =
              (2.0 * L_x / rod_diameter) *
              (1 + (-0.5 * std::log(lamda) + 0.5 * std::log(4.0) - 0.25) * lamda * lamda);
          auto Str =
              1.0 / (0.822 * (gap / rod_diameter) + 0.144); // Strouhal number (Wu & Trupp 1994)
          auto dum1 = 2.0 / std::pow(gamma, 2) * std::sqrt(a / 8.0) * (avg_hD / gap);
          auto dum2 = (1 / Pr_t) * lamda;
          auto dum3 = a_x * z_FP_over_D * Str;
          // Mixing Stanton number: Stg (eq 25,Kim and Chung (2001), eq 19 (Jeong et. al 2005)
          beta = dum1 * (dum2 + dum3) * std::pow(Re, -b / 2.0);
        }

        // Calculation of Turbulent Crossflow
        _WijPrime(i_gap, iz) = beta * avg_massflux * Sij;
      }
    }
  }
  else
  {
    for (unsigned int iz = first_node; iz < last_node + 1; iz++)
    {
      auto dz = _z_grid[iz] - _z_grid[iz - 1];
      auto iz_ind = iz - first_node;
      for (unsigned int i_gap = 0; i_gap < _n_gaps; i_gap++)
      {
        auto chans = _subchannel_mesh.getGapNeighborChannels(i_gap);
        unsigned int i_ch = chans.first;
        unsigned int j_ch = chans.second;
        auto * node_in_i = _subchannel_mesh.getChannelNode(i_ch, iz - 1);
        auto * node_out_i = _subchannel_mesh.getChannelNode(i_ch, iz);
        auto * node_in_j = _subchannel_mesh.getChannelNode(j_ch, iz - 1);
        auto * node_out_j = _subchannel_mesh.getChannelNode(j_ch, iz);
        auto Si_in = (*_S_flow_soln)(node_in_i);
        auto Sj_in = (*_S_flow_soln)(node_in_j);
        auto Si_out = (*_S_flow_soln)(node_out_i);
        auto Sj_out = (*_S_flow_soln)(node_out_j);
        // crossflow area between channels i,j (dz*gap_width)
        auto gap = _subchannel_mesh.getGapWidth(i_gap);
        auto Sij = dz * gap;
        auto avg_massflux =
            0.5 * (((*_mdot_soln)(node_in_i) + (*_mdot_soln)(node_in_j)) / (Si_in + Sj_in) +
                   ((*_mdot_soln)(node_out_i) + (*_mdot_soln)(node_out_j)) / (Si_out + Sj_out));
        auto beta = _beta;
        if (!_constant_beta)
        {
          auto w_perim_i = (*_w_perim_soln)(node_in_i);
          auto w_perim_j = (*_w_perim_soln)(node_in_j);
          auto mu_i = (*_mu_soln)(node_in_i);
          auto mu_j = (*_mu_soln)(node_in_j);
          // hydraulic diameter in the i direction
          auto hD_i = 4.0 * Si_in / w_perim_i;
          auto hD_j = 4.0 * Sj_in / w_perim_j;
          auto avg_hD = 0.5 * (hD_i + hD_j);
          auto avg_mu = 0.5 * (mu_i + mu_j);
          auto Re = avg_massflux * avg_hD / avg_mu;

          Real gamma = 20.0; // empirical constant
          Real sf = 1.0;     // shape factor
          Real a = 0.18;
          Real b = 0.2;
          auto f = a * std::pow(Re, -b); // Rehme 1992 circular tube friction factor
          auto k =
              0.25 * (_fp->k_from_p_T((*_P_soln)(node_out_i) + _P_out, (*_T_soln)(node_out_i)) +
                      _fp->k_from_p_T((*_P_soln)(node_in_i) + _P_out, (*_T_soln)(node_in_i)) +
                      _fp->k_from_p_T((*_P_soln)(node_out_j) + _P_out, (*_T_soln)(node_out_j)) +
                      _fp->k_from_p_T((*_P_soln)(node_in_j) + _P_out, (*_T_soln)(node_in_j)));
          auto cp =
              0.25 * (_fp->cp_from_p_T((*_P_soln)(node_out_i) + _P_out, (*_T_soln)(node_out_i)) +
                      _fp->cp_from_p_T((*_P_soln)(node_in_i) + _P_out, (*_T_soln)(node_in_i)) +
                      _fp->cp_from_p_T((*_P_soln)(node_out_j) + _P_out, (*_T_soln)(node_out_j)) +
                      _fp->cp_from_p_T((*_P_soln)(node_in_j) + _P_out, (*_T_soln)(node_in_j)));
          auto Pr = avg_mu * cp / k;                          // Prandtl number
          auto Pr_t = Pr * (Re / gamma) * std::sqrt(f / 8.0); // Turbulent Prandtl number
          auto delta = pitch;                                 // centroid to centroid distance
          auto L_x = sf * delta;  // axial length scale (gap is the lateral length scale)
          auto lamda = gap / L_x; // aspect ratio
          auto a_x = 1.0 - 2.0 * lamda * lamda / libMesh::pi; // velocity coefficient
          auto z_FP_over_D =
              (2.0 * L_x / rod_diameter) *
              (1 + (-0.5 * std::log(lamda) + 0.5 * std::log(4.0) - 0.25) * lamda * lamda);
          auto Str =
              1.0 / (0.822 * (gap / rod_diameter) + 0.144); // Strouhal number (Wu & Trupp 1994)
          auto dum1 = 2.0 / std::pow(gamma, 2) * std::sqrt(a / 8.0) * (avg_hD / gap);
          auto dum2 = (1 / Pr_t) * lamda;
          auto dum3 = a_x * z_FP_over_D * Str;
          // Mixing Stanton number: Stg (eq 25,Kim and Chung (2001), eq 19 (Jeong et. al 2005)
          beta = dum1 * (dum2 + dum3) * std::pow(Re, -b / 2.0);
        }

        // Base value - I don't want to write it every time
        PetscScalar base_value = beta * 0.5 * Sij;

        // Bottom values
        if (iz == first_node)
        {
          PetscScalar value_tl = -1.0 * base_value / (Si_in + Sj_in) *
                                 ((*_mdot_soln)(node_in_i) + (*_mdot_soln)(node_in_j));
          PetscInt row = i_gap + _n_gaps * iz_ind;
          VecSetValues(_amc_turbulent_cross_flows_rhs, 1, &row, &value_tl, INSERT_VALUES);
        }
        else
        {
          PetscScalar value_tl = base_value / (Si_in + Sj_in);
          PetscInt row = i_gap + _n_gaps * iz_ind;

          PetscInt col_ich = i_ch + _n_channels * (iz_ind - 1);
          MatSetValues(
              _amc_turbulent_cross_flows_mat, 1, &row, 1, &col_ich, &value_tl, INSERT_VALUES);

          PetscInt col_jch = j_ch + _n_channels * (iz_ind - 1);
          MatSetValues(
              _amc_turbulent_cross_flows_mat, 1, &row, 1, &col_jch, &value_tl, INSERT_VALUES);
        }

        // Top values
        PetscScalar value_bl = base_value / (Si_out + Sj_out);
        PetscInt row = i_gap + _n_gaps * iz_ind;

        PetscInt col_ich = i_ch + _n_channels * iz_ind;
        MatSetValues(
            _amc_turbulent_cross_flows_mat, 1, &row, 1, &col_ich, &value_bl, INSERT_VALUES);

        PetscInt col_jch = j_ch + _n_channels * iz_ind;
        MatSetValues(
            _amc_turbulent_cross_flows_mat, 1, &row, 1, &col_jch, &value_bl, INSERT_VALUES);
      }
    }
    MatAssemblyBegin(_amc_turbulent_cross_flows_mat, MAT_FINAL_ASSEMBLY);
    MatAssemblyEnd(_amc_turbulent_cross_flows_mat, MAT_FINAL_ASSEMBLY);

    /// Update turbulent crossflow
    Vec loc_prod;
    Vec loc_Wij;
    VecDuplicate(_amc_sys_mdot_rhs, &loc_prod);
    VecDuplicate(_Wij_vec, &loc_Wij);
    populateVectorFromHandle<SolutionHandle>(
        loc_prod, *_mdot_soln, first_node, last_node, _n_channels);
    MatMult(_amc_turbulent_cross_flows_mat, loc_prod, loc_Wij);
    VecAXPY(loc_Wij, -1.0, _amc_turbulent_cross_flows_rhs);
    populateDenseFromVector<libMesh::DenseMatrix<Real>>(
        loc_Wij, _WijPrime, first_node, last_node, _n_gaps);
    VecDestroy(&loc_prod);
    VecDestroy(&loc_Wij);
  }
}
