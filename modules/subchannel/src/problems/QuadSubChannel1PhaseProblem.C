//* This file is part of the MOOSE framework
//* https://mooseframework.inl.gov
//*
//* All rights reserved, see COPYRIGHT for full restrictions
//* https://github.com/idaholab/moose/blob/master/COPYRIGHT
//*
//* Licensed under LGPL 2.1, please see LICENSE for details
//* https://www.gnu.org/licenses/lgpl-2.1.html

#include "QuadSubChannel1PhaseProblem.h"
#include "AuxiliarySystem.h"
#include "SCM.h"

registerMooseObject("SubChannelApp", QuadSubChannel1PhaseProblem);

InputParameters
QuadSubChannel1PhaseProblem::validParams()
{
  InputParameters params = SubChannel1PhaseProblem::validParams();
  params.addClassDescription(
      "Solver class for subchannels in a square lattice assembly and bare fuel pins");
  params.addRequiredParam<Real>("beta",
                                "Thermal diffusion coefficient used in turbulent crossflow.");
  params.addParam<bool>(
      "default_friction_model",
      true,
      "Boolean to define which friction model to use (default: Pang, B. et al. "
      "KIT, 2013. / non-default: Todreas-Kazimi NUCLEAR SYSTEMS, second edition, Volume 1, 2011)");
  params.addParam<bool>(
      "constant_beta",
      true,
      "Boolean to define the use of a constant beta or beta correlation (Kim and Chung, 2001)");
  return params;
}

QuadSubChannel1PhaseProblem::QuadSubChannel1PhaseProblem(const InputParameters & params)
  : SubChannel1PhaseProblem(params),
    _subchannel_mesh(SCM::getMesh<QuadSubChannelMesh>(_mesh)),
    _beta(getParam<Real>("beta")),
    _default_friction_model(getParam<bool>("default_friction_model")),
    _constant_beta(getParam<bool>("constant_beta"))
{
}

void
QuadSubChannel1PhaseProblem::initializeSolution()
{
  /// update surface area, wetted perimeter based on: Dpin, displacement
  if (_deformation)
  {
    Real standard_area, additional_area, wetted_perimeter, displaced_area;
    auto pitch = _subchannel_mesh.getPitch();
    auto gap = _subchannel_mesh.getGap();
    auto z_blockage = _subchannel_mesh.getZBlockage();
    auto index_blockage = _subchannel_mesh.getIndexBlockage();
    auto reduction_blockage = _subchannel_mesh.getReductionBlockage();
    for (unsigned int iz = 0; iz < _n_cells + 1; iz++)
    {
      for (unsigned int i_ch = 0; i_ch < _n_channels; i_ch++)
      {
        auto * node = _subchannel_mesh.getChannelNode(i_ch, iz);
        auto subch_type = _subchannel_mesh.getSubchannelType(i_ch);
        auto Z = _z_grid[iz];
        Real rod_area = 0.0;
        Real rod_perimeter = 0.0;
        for (auto i_pin : _subchannel_mesh.getChannelPins(i_ch))
        {
          auto * pin_node = _subchannel_mesh.getPinNode(i_pin, iz);
          rod_area += 0.25 * 0.25 * M_PI * (*_Dpin_soln)(pin_node) * (*_Dpin_soln)(pin_node);
          rod_perimeter += 0.25 * M_PI * (*_Dpin_soln)(pin_node);
        }

        if (subch_type == EChannelType::CORNER)
        {
          standard_area = 0.25 * pitch * pitch;
          displaced_area = (2 * gap + pitch) * (*_displacement_soln)(node) / sqrt(2) +
                           (*_displacement_soln)(node) * (*_displacement_soln)(node) / 2;
          additional_area = pitch * gap + gap * gap;
          wetted_perimeter =
              rod_perimeter + pitch + 2 * gap + 2 * (*_displacement_soln)(node) / sqrt(2);
        }
        else if (subch_type == EChannelType::EDGE)
        {
          standard_area = 0.5 * pitch * pitch;
          additional_area = pitch * gap;
          displaced_area = pitch * (*_displacement_soln)(node);
          wetted_perimeter = rod_perimeter + pitch;
        }
        else
        {
          standard_area = pitch * pitch;
          displaced_area = 0.0;
          additional_area = 0.0;
          wetted_perimeter = rod_perimeter;
        }

        /// Calculate subchannel area
        auto subchannel_area = displaced_area + standard_area + additional_area - rod_area;

        /// Correct subchannel area and wetted perimeter in case of overlapping pins
        auto overlapping_pin_area = 0.0;
        auto overlapping_wetted_perimeter = 0.0;
        for (auto i_gap : _subchannel_mesh.getChannelGaps(i_ch))
        {
          auto gap_pins = _subchannel_mesh.getGapPins(i_gap);
          auto pin_1 = gap_pins.first;
          auto pin_2 = gap_pins.second;
          auto * pin_node_1 = _subchannel_mesh.getPinNode(pin_1, iz);
          auto * pin_node_2 = _subchannel_mesh.getPinNode(pin_2, iz);
          auto Diameter1 = (*_Dpin_soln)(pin_node_1);
          auto Radius1 = Diameter1 / 2.0;
          auto Diameter2 = (*_Dpin_soln)(pin_node_2);
          auto Radius2 = Diameter2 / 2.0;
          auto pitch = _subchannel_mesh.getPitch();

          if (pitch < (Radius1 + Radius2)) // overlapping pins
          {
            mooseWarning(" The gap of index : '", i_gap, " at axial cell ", iz, " ' is blocked.");
            auto cos1 =
                (pitch * pitch + Radius1 * Radius1 - Radius2 * Radius2) / (2 * pitch * Radius1);
            auto cos2 =
                (pitch * pitch + Radius2 * Radius2 - Radius1 * Radius1) / (2 * pitch * Radius2);
            auto angle1 = 2.0 * acos(cos1);
            auto angle2 = 2.0 * acos(cos2);
            // half of the intersecting arc-length
            overlapping_wetted_perimeter += 0.5 * angle1 * Radius1 + 0.5 * angle2 * Radius2;
            // Half of the overlapping area
            overlapping_pin_area +=
                0.5 * Radius1 * Radius1 * acos(cos1) + 0.5 * Radius2 * Radius2 * acos(cos2) -
                0.25 * sqrt((-pitch + Radius1 + Radius2) * (pitch + Radius1 - Radius2) *
                            (pitch - Radius1 + Radius2) * (pitch + Radius1 + Radius2));
          }
        }
        subchannel_area += overlapping_pin_area;           // correct surface area
        wetted_perimeter += -overlapping_wetted_perimeter; // correct wetted perimeter

        /// Apply area reduction on subchannels affected by blockage
        auto index = 0;
        for (const auto & i_blockage : index_blockage)
        {
          if (i_ch == i_blockage && (Z >= z_blockage.front() && Z <= z_blockage.back()))
          {
            subchannel_area *= reduction_blockage[index];
          }
          index++;
        }

        _S_flow_soln->set(node, subchannel_area);
        _w_perim_soln->set(node, wetted_perimeter);
      }
    }
    /// update map of gap between pins (gij) based on: Dpin, displacement
    for (unsigned int iz = 0; iz < _n_cells + 1; iz++)
    {
      for (unsigned int i_gap = 0; i_gap < _n_gaps; i_gap++)
      {
        auto gap_pins = _subchannel_mesh.getGapPins(i_gap);
        auto pin_1 = gap_pins.first;
        auto pin_2 = gap_pins.second;
        auto * pin_node_1 = _subchannel_mesh.getPinNode(pin_1, iz);
        auto * pin_node_2 = _subchannel_mesh.getPinNode(pin_2, iz);
        if (pin_1 == pin_2) // Corner or edge gap
        {
          auto displacement = 0.0;
          auto counter = 0.0;
          for (auto i_ch : _subchannel_mesh.getPinChannels(pin_1))
          {
            auto subch_type = _subchannel_mesh.getSubchannelType(i_ch);
            auto * node = _subchannel_mesh.getChannelNode(i_ch, iz);
            if (subch_type == EChannelType::EDGE || subch_type == EChannelType::CORNER)
            {
              displacement += (*_displacement_soln)(node);
              counter += 1.0;
            }
          }
          displacement = displacement / counter;
          _subchannel_mesh._gij_map[iz][i_gap] =
              (pitch - (*_Dpin_soln)(pin_node_1)) / 2.0 + gap + displacement;
        }
        else // center gap
        {
          _subchannel_mesh._gij_map[iz][i_gap] =
              pitch - (*_Dpin_soln)(pin_node_1) / 2.0 - (*_Dpin_soln)(pin_node_2) / 2.0;
        }
        // if pins come in contact, the gap is zero
        if (_subchannel_mesh._gij_map[iz][i_gap] <= 0.0)
          _subchannel_mesh._gij_map[iz][i_gap] = 0.0;
      }
    }
  }

  for (unsigned int iz = 1; iz < _n_cells + 1; iz++)
  {
    for (unsigned int i_ch = 0; i_ch < _n_channels; i_ch++)
    {
      auto * node_out = _subchannel_mesh.getChannelNode(i_ch, iz);
      auto * node_in = _subchannel_mesh.getChannelNode(i_ch, iz - 1);
      _mdot_soln->set(node_out, (*_mdot_soln)(node_in));
    }
  }

  // We must do a global assembly to make sure data is parallel consistent before we do things
  // like compute L2 norms
  _aux->solution().close();
}

Real
QuadSubChannel1PhaseProblem::computeFrictionFactor(FrictionStruct friction_args)
{
  auto Re = friction_args.Re;
  auto i_ch = friction_args.i_ch;
  /// Pang, B. et al. KIT, 2013
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
  /// Todreas-Kazimi NUCLEAR SYSTEMS, second edition, Volume 1, 2011
  else
  {
    Real aL, b1L, b2L, cL;
    Real aT, b1T, b2T, cT;
    auto pitch = _subchannel_mesh.getPitch();
    auto pin_diameter = _subchannel_mesh.getPinDiameter();
    // This gap is a constant value for the whole assembly. Might want to make it
    // subchannel specific in the future if we have duct deformation.
    auto gap = _subchannel_mesh.getGap();
    auto w = (pin_diameter / 2.0) + (pitch / 2.0) + gap;
    auto p_over_d = pitch / pin_diameter;
    auto w_over_d = w / pin_diameter;
    auto ReL = std::pow(10, (p_over_d - 1)) * 320.0;
    auto ReT = std::pow(10, 0.7 * (p_over_d - 1)) * 1.0E+4;
    auto psi = std::log(Re / ReL) / std::log(ReT / ReL);
    auto subch_type = _subchannel_mesh.getSubchannelType(i_ch);
    const Real lambda = 7.0;

    // Find the coefficients of bare Pin bundle friction factor
    // correlations for turbulent and laminar flow regimes. Todreas & Kazimi, Nuclear Systems Volume
    // 1
    if (subch_type == EChannelType::CENTER)
    {
      if (p_over_d < 1.1)
      {
        aL = 26.37;
        b1L = 374.2;
        b2L = -493.9;
        aT = 0.09423;
        b1T = 0.5806;
        b2T = -1.239;
      }
      else
      {
        aL = 35.55;
        b1L = 263.7;
        b2L = -190.2;
        aT = 0.1339;
        b1T = 0.09059;
        b2T = -0.09926;
      }
      // laminar flow friction factor for bare Pin bundle - Center subchannel
      cL = aL + b1L * (p_over_d - 1) + b2L * std::pow((p_over_d - 1), 2);
      // turbulent flow friction factor for bare Pin bundle - Center subchannel
      cT = aT + b1T * (p_over_d - 1) + b2T * std::pow((p_over_d - 1), 2);
    }
    else if (subch_type == EChannelType::EDGE)
    {
      if (p_over_d < 1.1)
      {
        aL = 26.18;
        b1L = 554.5;
        b2L = -1480;
        aT = 0.09377;
        b1T = 0.8732;
        b2T = -3.341;
      }
      else
      {
        aL = 44.40;
        b1L = 256.7;
        b2L = -267.6;
        aT = 0.1430;
        b1T = 0.04199;
        b2T = -0.04428;
      }
      // laminar flow friction factor for bare Pin bundle - Edge subchannel
      cL = aL + b1L * (w_over_d - 1) + b2L * std::pow((w_over_d - 1), 2);
      // turbulent flow friction factor for bare Pin bundle - Edge subchannel
      cT = aT + b1T * (w_over_d - 1) + b2T * std::pow((w_over_d - 1), 2);
    }
    else
    {
      if (p_over_d < 1.1)
      {
        aL = 28.62;
        b1L = 715.9;
        b2L = -2807;
        aT = 0.09755;
        b1T = 1.127;
        b2T = -6.304;
      }
      else
      {
        aL = 58.83;
        b1L = 160.7;
        b2L = -203.5;
        aT = 0.1452;
        b1T = 0.02681;
        b2T = -0.03411;
      }
      // laminar flow friction factor for bare Pin bundle - Corner subchannel
      cL = aL + b1L * (w_over_d - 1) + b2L * std::pow((w_over_d - 1), 2);
      // turbulent flow friction factor for bare Pin bundle - Corner subchannel
      cT = aT + b1T * (w_over_d - 1) + b2T * std::pow((w_over_d - 1), 2);
    }

    // laminar friction factor
    auto fL = cL / Re;
    // turbulent friction factor
    auto fT = cT / std::pow(Re, 0.18);

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
      // transient flow: psi definition uses a Bulk ReT/ReL number, same for all channels
      return fL * std::pow((1 - psi), 1.0 / 3.0) * (1 - std::pow(psi, lambda)) +
             fT * std::pow(psi, 1.0 / 3.0);
    }
  }
}

Real
QuadSubChannel1PhaseProblem::computeBeta(unsigned int i_gap, unsigned int iz)
{
  auto beta = _beta;
  if (!_constant_beta)
  {
    const Real & pitch = _subchannel_mesh.getPitch();
    const Real & pin_diameter = _subchannel_mesh.getPinDiameter();
    auto chans = _subchannel_mesh.getGapChannels(i_gap);
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
    auto gap = _subchannel_mesh.getGapWidth(iz, i_gap);
    auto avg_massflux =
        0.5 * (((*_mdot_soln)(node_in_i) + (*_mdot_soln)(node_in_j)) / (Si_in + Sj_in) +
               ((*_mdot_soln)(node_out_i) + (*_mdot_soln)(node_out_j)) / (Si_out + Sj_out));
    auto S_total = Si_in + Sj_in + Si_out + Sj_out;
    auto Si = 0.5 * (Si_in + Si_out);
    auto Sj = 0.5 * (Sj_in + Sj_out);
    auto w_perim_i = 0.5 * ((*_w_perim_soln)(node_in_i) + (*_w_perim_soln)(node_out_i));
    auto w_perim_j = 0.5 * ((*_w_perim_soln)(node_in_j) + (*_w_perim_soln)(node_out_j));
    auto avg_mu = (1 / S_total) * ((*_mu_soln)(node_out_i)*Si_out + (*_mu_soln)(node_in_i)*Si_in +
                                   (*_mu_soln)(node_out_j)*Sj_out + (*_mu_soln)(node_in_j)*Sj_in);
    auto avg_hD = 4.0 * (Si + Sj) / (w_perim_i + w_perim_j);
    auto Re = avg_massflux * avg_hD / avg_mu;
    Real gamma = 20.0; // empirical constant
    Real sf = 1.0;     // shape factor
    Real a = 0.18;
    Real b = 0.2;
    auto f = a * std::pow(Re, -b); // Rehme 1992 circular tube friction factor
    auto k = (1 / S_total) *
             (_fp->k_from_p_T((*_P_soln)(node_out_i) + _P_out, (*_T_soln)(node_out_i)) * Si_out +
              _fp->k_from_p_T((*_P_soln)(node_in_i) + _P_out, (*_T_soln)(node_in_i)) * Si_in +
              _fp->k_from_p_T((*_P_soln)(node_out_j) + _P_out, (*_T_soln)(node_out_j)) * Sj_out +
              _fp->k_from_p_T((*_P_soln)(node_in_j) + _P_out, (*_T_soln)(node_in_j)) * Sj_in);
    auto cp = (1 / S_total) *
              (_fp->cp_from_p_T((*_P_soln)(node_out_i) + _P_out, (*_T_soln)(node_out_i)) * Si_out +
               _fp->cp_from_p_T((*_P_soln)(node_in_i) + _P_out, (*_T_soln)(node_in_i)) * Si_in +
               _fp->cp_from_p_T((*_P_soln)(node_out_j) + _P_out, (*_T_soln)(node_out_j)) * Sj_out +
               _fp->cp_from_p_T((*_P_soln)(node_in_j) + _P_out, (*_T_soln)(node_in_j)) * Sj_in);
    auto Pr = avg_mu * cp / k;                          // Prandtl number
    auto Pr_t = Pr * (Re / gamma) * std::sqrt(f / 8.0); // Turbulent Prandtl number
    auto delta = pitch;                                 // centroid to centroid distance
    auto L_x = sf * delta;  // axial length scale (gap is the lateral length scale)
    auto lamda = gap / L_x; // aspect ratio
    auto a_x = 1.0 - 2.0 * lamda * lamda / libMesh::pi; // velocity coefficient
    auto z_FP_over_D = (2.0 * L_x / pin_diameter) *
                       (1 + (-0.5 * std::log(lamda) + 0.5 * std::log(4.0) - 0.25) * lamda * lamda);
    auto Str = 1.0 / (0.822 * (gap / pin_diameter) + 0.144); // Strouhal number (Wu & Trupp 1994)
    auto freq_factor = 2.0 / std::pow(gamma, 2) * std::sqrt(a / 8.0) * (avg_hD / gap);
    auto rod_mixing = (1 / Pr_t) * lamda;
    auto axial_mixing = a_x * z_FP_over_D * Str;
    // Mixing Stanton number: Stg (eq 25,Kim and Chung (2001), eq 19 (Jeong et. al 2005)
    beta = freq_factor * (rod_mixing + axial_mixing) * std::pow(Re, -b / 2.0);
  }
  mooseAssert(beta > 0, "beta should be positive");
  return beta;
}

Real
QuadSubChannel1PhaseProblem::computeAddedHeatPin(unsigned int i_ch, unsigned int iz)
{
  // Compute axial location of nodes.
  auto z2 = _z_grid[iz];
  auto z1 = _z_grid[iz - 1];
  auto heated_length = _subchannel_mesh.getHeatedLength();
  auto unheated_length_entry = _subchannel_mesh.getHeatedLengthEntry();
  if (MooseUtils::absoluteFuzzyGreaterThan(z2, unheated_length_entry) &&
      MooseUtils::absoluteFuzzyLessThan(z1, unheated_length_entry + heated_length))
  {
    // Compute the height of this element.
    auto dz = z2 - z1;
    if (_pin_mesh_exist)
    {
      auto heat_rate_in = 0.0;
      auto heat_rate_out = 0.0;
      for (auto i_pin : _subchannel_mesh.getChannelPins(i_ch))
      {
        auto * node_in = _subchannel_mesh.getPinNode(i_pin, iz - 1);
        auto * node_out = _subchannel_mesh.getPinNode(i_pin, iz);
        heat_rate_out += 0.25 * (*_q_prime_soln)(node_out);
        heat_rate_in += 0.25 * (*_q_prime_soln)(node_in);
      }
      return (heat_rate_in + heat_rate_out) * dz / 2.0;
    }
    else
    {
      auto * node_in = _subchannel_mesh.getChannelNode(i_ch, iz - 1);
      auto * node_out = _subchannel_mesh.getChannelNode(i_ch, iz);
      return ((*_q_prime_soln)(node_out) + (*_q_prime_soln)(node_in)) * dz / 2.0;
    }
  }
  else
    return 0.0;
}

void
QuadSubChannel1PhaseProblem::computeh(int iblock)
{
  unsigned int last_node = (iblock + 1) * _block_size;
  unsigned int first_node = iblock * _block_size + 1;
  if (iblock == 0)
  {
    for (unsigned int i_ch = 0; i_ch < _n_channels; i_ch++)
    {
      auto * node = _subchannel_mesh.getChannelNode(i_ch, 0);
      auto h_out = _fp->h_from_p_T((*_P_soln)(node) + _P_out, (*_T_soln)(node));
      if (h_out < 0)
      {
        mooseError(
            name(), " : Calculation of negative Enthalpy h_out = : ", h_out, " Axial Level= : ", 0);
      }
      _h_soln->set(node, h_out);
    }
  }

  if (!_implicit_bool)
  {
    for (unsigned int iz = first_node; iz < last_node + 1; iz++)
    {
      auto dz = _z_grid[iz] - _z_grid[iz - 1];
      for (unsigned int i_ch = 0; i_ch < _n_channels; i_ch++)
      {
        auto * node_in = _subchannel_mesh.getChannelNode(i_ch, iz - 1);
        auto * node_out = _subchannel_mesh.getChannelNode(i_ch, iz);
        auto mdot_in = (*_mdot_soln)(node_in);
        auto h_in = (*_h_soln)(node_in); // J/kg
        auto volume = dz * (*_S_flow_soln)(node_in);
        auto mdot_out = (*_mdot_soln)(node_out);
        auto h_out = 0.0;
        Real sumWijh = 0.0;
        Real sumWijPrimeDhij = 0.0;
        Real added_enthalpy = computeAddedHeatPin(i_ch, iz);
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
          Real h_star = 0.0;
          if (_Wij(i_gap, iz) > 0.0)
            h_star = (*_h_soln)(node_in_i);
          else if (_Wij(i_gap, iz) < 0.0)
            h_star = (*_h_soln)(node_in_j);
          // take care of the sign by applying the map, use donor cell
          sumWijh += _subchannel_mesh.getCrossflowSign(i_ch, counter) * _Wij(i_gap, iz) * h_star;
          sumWijPrimeDhij += _WijPrime(i_gap, iz) * (2 * (*_h_soln)(node_in) -
                                                     (*_h_soln)(node_in_j) - (*_h_soln)(node_in_i));
          counter++;
        }
        h_out = (mdot_in * h_in - sumWijh - sumWijPrimeDhij + added_enthalpy +
                 _TR * _rho_soln->old(node_out) * _h_soln->old(node_out) * volume / _dt) /
                (mdot_out + _TR * (*_rho_soln)(node_out)*volume / _dt);
        if (h_out < 0)
        {
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
    LibmeshPetscCall(MatZeroEntries(_hc_time_derivative_mat));
    LibmeshPetscCall(MatZeroEntries(_hc_advective_derivative_mat));
    LibmeshPetscCall(MatZeroEntries(_hc_cross_derivative_mat));
    LibmeshPetscCall(VecZeroEntries(_hc_time_derivative_rhs));
    LibmeshPetscCall(VecZeroEntries(_hc_advective_derivative_rhs));
    LibmeshPetscCall(VecZeroEntries(_hc_cross_derivative_rhs));
    LibmeshPetscCall(VecZeroEntries(_hc_added_heat_rhs));
    LibmeshPetscCall(MatZeroEntries(_hc_sys_h_mat));
    LibmeshPetscCall(VecZeroEntries(_hc_sys_h_rhs));
    for (unsigned int iz = first_node; iz < last_node + 1; iz++)
    {
      auto dz = _z_grid[iz] - _z_grid[iz - 1];
      auto iz_ind = iz - first_node;
      for (unsigned int i_ch = 0; i_ch < _n_channels; i_ch++)
      {
        auto * node_in = _subchannel_mesh.getChannelNode(i_ch, iz - 1);
        auto * node_out = _subchannel_mesh.getChannelNode(i_ch, iz);
        auto S_in = (*_S_flow_soln)(node_in);
        auto S_out = (*_S_flow_soln)(node_out);
        auto S_interp = computeInterpolatedValue(S_out, S_in, 0.5);
        auto volume = dz * S_interp;

        // interpolation weight coefficient
        auto alpha = computeInterpolationCoefficients(0.5);

        /// Time derivative term
        if (iz == first_node)
        {
          PetscScalar value_vec_tt =
              -1.0 * _TR * alpha * (*_rho_soln)(node_in) * (*_h_soln)(node_in)*volume / _dt;
          PetscInt row_vec_tt = i_ch + _n_channels * iz_ind;
          LibmeshPetscCall(
              VecSetValues(_hc_time_derivative_rhs, 1, &row_vec_tt, &value_vec_tt, ADD_VALUES));
        }
        else
        {
          PetscInt row_tt = i_ch + _n_channels * iz_ind;
          PetscInt col_tt = i_ch + _n_channels * (iz_ind - 1);
          PetscScalar value_tt = _TR * alpha * (*_rho_soln)(node_in)*volume / _dt;
          LibmeshPetscCall(MatSetValues(
              _hc_time_derivative_mat, 1, &row_tt, 1, &col_tt, &value_tt, INSERT_VALUES));
        }

        // Adding diagonal elements
        PetscInt row_tt = i_ch + _n_channels * iz_ind;
        PetscInt col_tt = i_ch + _n_channels * iz_ind;
        PetscScalar value_tt = _TR * (1.0 - alpha) * (*_rho_soln)(node_out)*volume / _dt;
        LibmeshPetscCall(MatSetValues(
            _hc_time_derivative_mat, 1, &row_tt, 1, &col_tt, &value_tt, INSERT_VALUES));

        // Adding RHS elements
        PetscScalar rho_old_interp =
            computeInterpolatedValue(_rho_soln->old(node_out), _rho_soln->old(node_in), 0.5);
        PetscScalar h_old_interp =
            computeInterpolatedValue(_h_soln->old(node_out), _h_soln->old(node_in), 0.5);
        PetscScalar value_vec_tt = _TR * rho_old_interp * h_old_interp * volume / _dt;
        PetscInt row_vec_tt = i_ch + _n_channels * iz_ind;
        LibmeshPetscCall(
            VecSetValues(_hc_time_derivative_rhs, 1, &row_vec_tt, &value_vec_tt, ADD_VALUES));

        /// Advective derivative term
        if (iz == first_node)
        {
          PetscScalar value_vec_at = (*_mdot_soln)(node_in) * (*_h_soln)(node_in);
          PetscInt row_vec_at = i_ch + _n_channels * iz_ind;
          LibmeshPetscCall(VecSetValues(
              _hc_advective_derivative_rhs, 1, &row_vec_at, &value_vec_at, ADD_VALUES));
        }
        else
        {
          PetscInt row_at = i_ch + _n_channels * iz_ind;
          PetscInt col_at = i_ch + _n_channels * (iz_ind - 1);
          PetscScalar value_at = -1.0 * (*_mdot_soln)(node_in);
          LibmeshPetscCall(MatSetValues(
              _hc_advective_derivative_mat, 1, &row_at, 1, &col_at, &value_at, INSERT_VALUES));
        }

        // Adding diagonal elements
        PetscInt row_at = i_ch + _n_channels * iz_ind;
        PetscInt col_at = i_ch + _n_channels * iz_ind;
        PetscScalar value_at = (*_mdot_soln)(node_out);
        LibmeshPetscCall(MatSetValues(
            _hc_advective_derivative_mat, 1, &row_at, 1, &col_at, &value_at, INSERT_VALUES));

        /// Cross derivative term
        unsigned int counter = 0;
        unsigned int cross_index = iz; // iz-1;
        for (auto i_gap : _subchannel_mesh.getChannelGaps(i_ch))
        {
          auto chans = _subchannel_mesh.getGapChannels(i_gap);
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
              LibmeshPetscCall(VecSetValues(
                  _hc_cross_derivative_rhs, 1, &row_vec_ct, &value_vec_ct, ADD_VALUES));
            }
            else
            {
              PetscScalar value_ct = alpha * _subchannel_mesh.getCrossflowSign(i_ch, counter) *
                                     _Wij(i_gap, cross_index);
              PetscInt row_ct = i_ch + _n_channels * iz_ind;
              PetscInt col_ct = ii_ch + _n_channels * (iz_ind - 1);
              LibmeshPetscCall(MatSetValues(
                  _hc_cross_derivative_mat, 1, &row_ct, 1, &col_ct, &value_ct, ADD_VALUES));
            }
            PetscScalar value_ct = (1.0 - alpha) *
                                   _subchannel_mesh.getCrossflowSign(i_ch, counter) *
                                   _Wij(i_gap, cross_index);
            PetscInt row_ct = i_ch + _n_channels * iz_ind;
            PetscInt col_ct = ii_ch + _n_channels * iz_ind;
            LibmeshPetscCall(MatSetValues(
                _hc_cross_derivative_mat, 1, &row_ct, 1, &col_ct, &value_ct, ADD_VALUES));
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
              LibmeshPetscCall(VecSetValues(
                  _hc_cross_derivative_rhs, 1, &row_vec_ct, &value_vec_ct, ADD_VALUES));
            }
            else
            {
              PetscScalar value_ct = alpha * _subchannel_mesh.getCrossflowSign(i_ch, counter) *
                                     _Wij(i_gap, cross_index);
              PetscInt row_ct = i_ch + _n_channels * iz_ind;
              PetscInt col_ct = jj_ch + _n_channels * (iz_ind - 1);
              LibmeshPetscCall(MatSetValues(
                  _hc_cross_derivative_mat, 1, &row_ct, 1, &col_ct, &value_ct, ADD_VALUES));
            }
            PetscScalar value_ct = (1.0 - alpha) *
                                   _subchannel_mesh.getCrossflowSign(i_ch, counter) *
                                   _Wij(i_gap, cross_index);
            PetscInt row_ct = i_ch + _n_channels * iz_ind;
            PetscInt col_ct = jj_ch + _n_channels * iz_ind;
            LibmeshPetscCall(MatSetValues(
                _hc_cross_derivative_mat, 1, &row_ct, 1, &col_ct, &value_ct, ADD_VALUES));
          }

          // Turbulent cross flows
          if (iz == first_node)
          {
            PetscScalar value_vec_ct =
                -2.0 * alpha * (*_h_soln)(node_in)*_WijPrime(i_gap, cross_index);
            value_vec_ct += alpha * (*_h_soln)(node_in_j)*_WijPrime(i_gap, cross_index);
            value_vec_ct += alpha * (*_h_soln)(node_in_i)*_WijPrime(i_gap, cross_index);
            PetscInt row_vec_ct = i_ch + _n_channels * iz_ind;
            LibmeshPetscCall(
                VecSetValues(_hc_cross_derivative_rhs, 1, &row_vec_ct, &value_vec_ct, ADD_VALUES));
          }
          else
          {
            PetscScalar value_center_ct = 2.0 * alpha * _WijPrime(i_gap, cross_index);
            PetscInt row_ct = i_ch + _n_channels * iz_ind;
            PetscInt col_ct = i_ch + _n_channels * (iz_ind - 1);
            LibmeshPetscCall(MatSetValues(
                _hc_cross_derivative_mat, 1, &row_ct, 1, &col_ct, &value_center_ct, ADD_VALUES));

            PetscScalar value_left_ct = -1.0 * alpha * _WijPrime(i_gap, cross_index);
            row_ct = i_ch + _n_channels * iz_ind;
            col_ct = jj_ch + _n_channels * (iz_ind - 1);
            LibmeshPetscCall(MatSetValues(
                _hc_cross_derivative_mat, 1, &row_ct, 1, &col_ct, &value_left_ct, ADD_VALUES));

            PetscScalar value_right_ct = -1.0 * alpha * _WijPrime(i_gap, cross_index);
            row_ct = i_ch + _n_channels * iz_ind;
            col_ct = ii_ch + _n_channels * (iz_ind - 1);
            LibmeshPetscCall(MatSetValues(
                _hc_cross_derivative_mat, 1, &row_ct, 1, &col_ct, &value_right_ct, ADD_VALUES));
          }
          PetscScalar value_center_ct = 2.0 * (1.0 - alpha) * _WijPrime(i_gap, cross_index);
          PetscInt row_ct = i_ch + _n_channels * iz_ind;
          PetscInt col_ct = i_ch + _n_channels * iz_ind;
          LibmeshPetscCall(MatSetValues(
              _hc_cross_derivative_mat, 1, &row_ct, 1, &col_ct, &value_center_ct, ADD_VALUES));

          PetscScalar value_left_ct = -1.0 * (1.0 - alpha) * _WijPrime(i_gap, cross_index);
          row_ct = i_ch + _n_channels * iz_ind;
          col_ct = jj_ch + _n_channels * iz_ind;
          LibmeshPetscCall(MatSetValues(
              _hc_cross_derivative_mat, 1, &row_ct, 1, &col_ct, &value_left_ct, ADD_VALUES));

          PetscScalar value_right_ct = -1.0 * (1.0 - alpha) * _WijPrime(i_gap, cross_index);
          row_ct = i_ch + _n_channels * iz_ind;
          col_ct = ii_ch + _n_channels * iz_ind;
          LibmeshPetscCall(MatSetValues(
              _hc_cross_derivative_mat, 1, &row_ct, 1, &col_ct, &value_right_ct, ADD_VALUES));
          counter++;
        }

        /// Added heat enthalpy
        PetscScalar added_enthalpy = computeAddedHeatPin(i_ch, iz);
        PetscInt row_vec_ht = i_ch + _n_channels * iz_ind;
        LibmeshPetscCall(
            VecSetValues(_hc_added_heat_rhs, 1, &row_vec_ht, &added_enthalpy, ADD_VALUES));
      }
    }
    /// Assembling system
    LibmeshPetscCall(MatAssemblyBegin(_hc_time_derivative_mat, MAT_FINAL_ASSEMBLY));
    LibmeshPetscCall(MatAssemblyEnd(_hc_time_derivative_mat, MAT_FINAL_ASSEMBLY));
    LibmeshPetscCall(MatAssemblyBegin(_hc_advective_derivative_mat, MAT_FINAL_ASSEMBLY));
    LibmeshPetscCall(MatAssemblyEnd(_hc_advective_derivative_mat, MAT_FINAL_ASSEMBLY));
    LibmeshPetscCall(MatAssemblyBegin(_hc_cross_derivative_mat, MAT_FINAL_ASSEMBLY));
    LibmeshPetscCall(MatAssemblyEnd(_hc_cross_derivative_mat, MAT_FINAL_ASSEMBLY));
    LibmeshPetscCall(MatAssemblyBegin(_hc_sys_h_mat, MAT_FINAL_ASSEMBLY));
    LibmeshPetscCall(MatAssemblyEnd(_hc_sys_h_mat, MAT_FINAL_ASSEMBLY));
    // Matrix
#if !PETSC_VERSION_LESS_THAN(3, 15, 0)
    LibmeshPetscCall(MatAXPY(_hc_sys_h_mat, 1.0, _hc_time_derivative_mat, UNKNOWN_NONZERO_PATTERN));
    LibmeshPetscCall(MatAssemblyBegin(_hc_sys_h_mat, MAT_FINAL_ASSEMBLY));
    LibmeshPetscCall(MatAssemblyEnd(_hc_sys_h_mat, MAT_FINAL_ASSEMBLY));
    LibmeshPetscCall(
        MatAXPY(_hc_sys_h_mat, 1.0, _hc_advective_derivative_mat, UNKNOWN_NONZERO_PATTERN));
    LibmeshPetscCall(MatAssemblyBegin(_hc_sys_h_mat, MAT_FINAL_ASSEMBLY));
    LibmeshPetscCall(MatAssemblyEnd(_hc_sys_h_mat, MAT_FINAL_ASSEMBLY));
    LibmeshPetscCall(
        MatAXPY(_hc_sys_h_mat, 1.0, _hc_cross_derivative_mat, UNKNOWN_NONZERO_PATTERN));
#else
    LibmeshPetscCall(
        MatAXPY(_hc_sys_h_mat, 1.0, _hc_time_derivative_mat, DIFFERENT_NONZERO_PATTERN));
    LibmeshPetscCall(MatAssemblyBegin(_hc_sys_h_mat, MAT_FINAL_ASSEMBLY));
    LibmeshPetscCall(MatAssemblyEnd(_hc_sys_h_mat, MAT_FINAL_ASSEMBLY));
    LibmeshPetscCall(
        MatAXPY(_hc_sys_h_mat, 1.0, _hc_advective_derivative_mat, DIFFERENT_NONZERO_PATTERN));
    LibmeshPetscCall(MatAssemblyBegin(_hc_sys_h_mat, MAT_FINAL_ASSEMBLY));
    LibmeshPetscCall(MatAssemblyEnd(_hc_sys_h_mat, MAT_FINAL_ASSEMBLY));
    LibmeshPetscCall(
        MatAXPY(_hc_sys_h_mat, 1.0, _hc_cross_derivative_mat, DIFFERENT_NONZERO_PATTERN));
#endif
    LibmeshPetscCall(MatAssemblyBegin(_hc_sys_h_mat, MAT_FINAL_ASSEMBLY));
    LibmeshPetscCall(MatAssemblyEnd(_hc_sys_h_mat, MAT_FINAL_ASSEMBLY));
    if (_verbose_subchannel)
      _console << "Block: " << iblock << " - Enthalpy conservation matrix assembled" << std::endl;
    // RHS
    LibmeshPetscCall(VecAXPY(_hc_sys_h_rhs, 1.0, _hc_time_derivative_rhs));
    LibmeshPetscCall(VecAXPY(_hc_sys_h_rhs, 1.0, _hc_advective_derivative_rhs));
    LibmeshPetscCall(VecAXPY(_hc_sys_h_rhs, 1.0, _hc_cross_derivative_rhs));
    LibmeshPetscCall(VecAXPY(_hc_sys_h_rhs, 1.0, _hc_added_heat_rhs));

    if (_segregated_bool || (!_monolithic_thermal_bool))
    {
      // Assembly the matrix system
      KSP ksploc;
      PC pc;
      Vec sol;
      LibmeshPetscCall(VecDuplicate(_hc_sys_h_rhs, &sol));
      LibmeshPetscCall(KSPCreate(PETSC_COMM_WORLD, &ksploc));
      LibmeshPetscCall(KSPSetOperators(ksploc, _hc_sys_h_mat, _hc_sys_h_mat));
      LibmeshPetscCall(KSPGetPC(ksploc, &pc));
      LibmeshPetscCall(PCSetType(pc, PCJACOBI));
      LibmeshPetscCall(KSPSetTolerances(ksploc, _rtol, _atol, _dtol, _maxit));
      LibmeshPetscCall(KSPSetOptionsPrefix(ksploc, "h_sys_"));
      LibmeshPetscCall(KSPSetFromOptions(ksploc));
      LibmeshPetscCall(KSPSolve(ksploc, _hc_sys_h_rhs, sol));
      PetscScalar * xx;
      LibmeshPetscCall(VecGetArray(sol, &xx));
      for (unsigned int iz = first_node; iz < last_node + 1; iz++)
      {
        auto iz_ind = iz - first_node;
        for (unsigned int i_ch = 0; i_ch < _n_channels; i_ch++)
        {
          auto * node_out = _subchannel_mesh.getChannelNode(i_ch, iz);
          auto h_out = xx[iz_ind * _n_channels + i_ch];
          if (h_out < 0)
          {
            mooseError(name(),
                       " : Calculation of negative Enthalpy h_out = : ",
                       h_out,
                       " Axial Level= : ",
                       iz);
          }
          _h_soln->set(node_out, h_out);
        }
      }
      LibmeshPetscCall(KSPDestroy(&ksploc));
      LibmeshPetscCall(VecDestroy(&sol));
    }
  }
}
