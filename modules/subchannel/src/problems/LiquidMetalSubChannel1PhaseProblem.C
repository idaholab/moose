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

#include "LiquidMetalSubChannel1PhaseProblem.h"
#include "AuxiliarySystem.h"
#include "TriSubChannelMesh.h"

registerMooseObject("SubChannelApp", LiquidMetalSubChannel1PhaseProblem);

InputParameters
LiquidMetalSubChannel1PhaseProblem::validParams()
{
  InputParameters params = SubChannel1PhaseProblem::validParams();
  params.addClassDescription("Solver class for metal-cooled subchannels in a triangular lattice "
                             "assembly and bare/wire-wrapped fuel rods");
  return params;
}

LiquidMetalSubChannel1PhaseProblem::LiquidMetalSubChannel1PhaseProblem(
    const InputParameters & params)
  : SubChannel1PhaseProblem(params),
    _tri_sch_mesh(dynamic_cast<TriSubChannelMesh &>(_subchannel_mesh))
{
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

LiquidMetalSubChannel1PhaseProblem::~LiquidMetalSubChannel1PhaseProblem()
{
  // Clean up heat conduction system
  MatDestroy(&_hc_axial_heat_conduction_mat);
  VecDestroy(&_hc_axial_heat_conduction_rhs);
  MatDestroy(&_hc_radial_heat_conduction_mat);
  VecDestroy(&_hc_radial_heat_conduction_rhs);
  MatDestroy(&_hc_sweep_enthalpy_mat);
  VecDestroy(&_hc_sweep_enthalpy_rhs);
}

void
LiquidMetalSubChannel1PhaseProblem::initializeSolution()
{
  auto pin_mesh_exist = _subchannel_mesh.pinMeshExist();
  auto duct_mesh_exist = _subchannel_mesh.ductMeshExist();
  if (pin_mesh_exist || duct_mesh_exist)
  {
    Real standard_area, wire_area, additional_area, wetted_perimeter, displaced_area;
    auto flat_to_flat = _tri_sch_mesh.getFlatToFlat();
    auto n_rings = _tri_sch_mesh.getNumOfRings();
    auto pitch = _subchannel_mesh.getPitch();
    auto rod_diameter = _subchannel_mesh.getRodDiameter();
    auto wire_diameter = _tri_sch_mesh.getWireDiameter();
    auto wire_lead_length = _tri_sch_mesh.getWireLeadLength();
    auto gap = _tri_sch_mesh.getDuctToRodGap();
    auto z_blockage = _subchannel_mesh.getZBlockage();
    auto index_blockage = _subchannel_mesh.getIndexBlockage();
    auto reduction_blockage = _subchannel_mesh.getReductionBlockage();
    auto theta = std::acos(wire_lead_length /
                           std::sqrt(std::pow(wire_lead_length, 2) +
                                     std::pow(libMesh::pi * (rod_diameter + wire_diameter), 2)));
    for (unsigned int iz = 0; iz < _n_cells + 1; iz++)
    {
      for (unsigned int i_ch = 0; i_ch < _n_channels; i_ch++)
      {
        auto subch_type = _subchannel_mesh.getSubchannelType(i_ch);
        auto * node = _subchannel_mesh.getChannelNode(i_ch, iz);
        auto Z = _z_grid[iz];
        Real rod_area = 0.0;
        Real rod_perimeter = 0.0;
        if (pin_mesh_exist)
        {
          for (auto i_pin : _subchannel_mesh.getChannelPins(i_ch))
          {
            auto * pin_node = _subchannel_mesh.getPinNode(i_pin, iz);
            if (subch_type == EChannelType::CENTER || subch_type == EChannelType::CORNER)
            {
              rod_area +=
                  (1.0 / 6.0) * 0.25 * M_PI * (*_Dpin_soln)(pin_node) * (*_Dpin_soln)(pin_node);
              rod_perimeter += (1.0 / 6.0) * M_PI * (*_Dpin_soln)(pin_node);
            }
            else
            {
              rod_area +=
                  (1.0 / 4.0) * 0.25 * M_PI * (*_Dpin_soln)(pin_node) * (*_Dpin_soln)(pin_node);
              rod_perimeter += (1.0 / 4.0) * M_PI * (*_Dpin_soln)(pin_node);
            }
          }
        }
        else
        {
          if (subch_type == EChannelType::CENTER || subch_type == EChannelType::EDGE)
          {
            rod_area = (1.0 / 2.0) * 0.25 * M_PI * rod_diameter * rod_diameter;
            rod_perimeter = (1.0 / 2.0) * M_PI * rod_diameter;
          }
          else
          {
            rod_area = (1.0 / 6.0) * 0.25 * M_PI * rod_diameter * rod_diameter;
            rod_perimeter = (1.0 / 6.0) * M_PI * rod_diameter;
          }
        }

        if (subch_type == EChannelType::CENTER)
        {
          standard_area = std::pow(pitch, 2.0) * std::sqrt(3.0) / 4.0;
          additional_area = 0.0;
          displaced_area = 0.0;
          wire_area = libMesh::pi * std::pow(wire_diameter, 2.0) / 8.0 / std::cos(theta);
          wetted_perimeter = rod_perimeter + 0.5 * libMesh::pi * wire_diameter / std::cos(theta);
        }
        else if (subch_type == EChannelType::EDGE)
        {
          standard_area = pitch * (rod_diameter / 2.0 + gap);
          additional_area = 0.0;
          displaced_area = (*_displacement_soln)(node)*pitch;
          wire_area = libMesh::pi * std::pow(wire_diameter, 2.0) / 8.0 / std::cos(theta);
          wetted_perimeter =
              rod_perimeter + 0.5 * libMesh::pi * wire_diameter / std::cos(theta) + pitch;
        }
        else
        {
          standard_area = 1.0 / std::sqrt(3.0) * std::pow((rod_diameter / 2.0 + gap), 2.0);
          additional_area = 0.0;
          displaced_area = 1.0 / std::sqrt(3.0) *
                           (rod_diameter + 2.0 * gap + (*_displacement_soln)(node)) *
                           (*_displacement_soln)(node);
          wire_area = libMesh::pi / 24.0 * std::pow(wire_diameter, 2.0) / std::cos(theta);
          wetted_perimeter =
              rod_perimeter + libMesh::pi * wire_diameter / std::cos(theta) / 6.0 +
              2.0 / std::sqrt(3.0) * (rod_diameter / 2.0 + gap + (*_displacement_soln)(node));
        }

        /// Calculate subchannel area
        auto subchannel_area =
            standard_area + additional_area + displaced_area - rod_area - wire_area;

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

    if (pin_mesh_exist)
    {
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
            _tri_sch_mesh._gij_map[iz][i_gap] =
                0.5 * (flat_to_flat - (n_rings - 1) * pitch * std::sqrt(3.0) -
                       (*_Dpin_soln)(pin_node_1)) +
                displacement;
          }
          else // center gap
          {
            _tri_sch_mesh._gij_map[iz][i_gap] =
                pitch - (*_Dpin_soln)(pin_node_1) / 2.0 - (*_Dpin_soln)(pin_node_2) / 2.0;
          }
        }
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
LiquidMetalSubChannel1PhaseProblem::computeFrictionFactor(_friction_args_struct friction_args)
{
  auto Re = friction_args.Re;
  auto i_ch = friction_args.i_ch;
  auto S = friction_args.S;
  auto w_perim = friction_args.w_perim;
  auto Dh_i = friction_args.Dh_i;
  Real aL, b1L, b2L, cL;
  Real aT, b1T, b2T, cT;
  const Real & pitch = _subchannel_mesh.getPitch();
  const Real & rod_diameter = _subchannel_mesh.getRodDiameter();
  const Real & wire_lead_length = _tri_sch_mesh.getWireLeadLength();
  const Real & wire_diameter = _tri_sch_mesh.getWireDiameter();
  auto p_over_d = pitch / rod_diameter;
  auto subch_type = _subchannel_mesh.getSubchannelType(i_ch);
  auto gap = _tri_sch_mesh.getDuctToRodGap();
  auto w_over_d = (rod_diameter + gap) / rod_diameter;
  auto ReL = std::pow(10, (p_over_d - 1)) * 320.0;
  auto ReT = std::pow(10, 0.7 * (p_over_d - 1)) * 1.0E+4;
  const Real lambda = 7.0;
  auto psi = std::log(Re / ReL) / std::log(ReT / ReL);
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

  // Find the coefficients of bare rod bundle friction factor
  // correlations for turbulent and laminar flow regimes. Todreas & Kazimi, Nuclear Systems Volume 1
  if (subch_type == EChannelType::CENTER)
  {
    if (p_over_d < 1.1)
    {
      aL = 26.0;
      b1L = 888.2;
      b2L = -3334.0;
      aT = 0.09378;
      b1T = 1.398;
      b2T = -8.664;
    }
    else
    {
      aL = 62.97;
      b1L = 216.9;
      b2L = -190.2;
      aT = 0.1458;
      b1T = 0.03632;
      b2T = -0.03333;
    }
    // laminar flow friction factor for bare rod bundle - Center subchannel
    cL = aL + b1L * (p_over_d - 1) + b2L * std::pow((p_over_d - 1), 2);
    // turbulent flow friction factor for bare rod bundle - Center subchannel
    cT = aT + b1T * (p_over_d - 1) + b2T * std::pow((p_over_d - 1), 2);
  }
  else if (subch_type == EChannelType::EDGE)
  {
    if (w_over_d < 1.1)
    {
      aL = 26.18;
      b1L = 554.5;
      b2L = -1480.0;
      aT = 0.09377;
      b1T = 0.8732;
      b2T = -3.341;
    }
    else
    {
      aL = 44.4;
      b1L = 256.7;
      b2L = -267.6;
      aT = 0.1430;
      b1T = 0.04199;
      b2T = -0.04428;
    }
    // laminar flow friction factor for bare rod bundle - Edge subchannel
    cL = aL + b1L * (w_over_d - 1) + b2L * std::pow((w_over_d - 1), 2);
    // turbulent flow friction factor for bare rod bundle - Edge subchannel
    cT = aT + b1T * (w_over_d - 1) + b2T * std::pow((w_over_d - 1), 2);
  }
  else
  {
    if (w_over_d < 1.1)
    {
      aL = 26.98;
      b1L = 1636.0;
      b2L = -10050.0;
      aT = 0.1004;
      b1T = 1.625;
      b2T = -11.85;
    }
    else
    {
      aL = 87.26;
      b1L = 38.59;
      b2L = -55.12;
      aT = 0.1499;
      b1T = 0.006706;
      b2T = -0.0009567;
    }
    // laminar flow friction factor for bare rod bundle - Corner subchannel
    cL = aL + b1L * (w_over_d - 1) + b2L * std::pow((w_over_d - 1), 2);
    // turbulent flow friction factor for bare rod bundle - Corner subchannel
    cT = aT + b1T * (w_over_d - 1) + b2T * std::pow((w_over_d - 1), 2);
  }

  // Find the coefficients of wire-wrapped rod bundle friction factor
  // correlations for turbulent and laminar flow regimes. Todreas & Kazimi, Nuclear Systems Volume 1
  // also Chen and Todreas (2018).
  if ((wire_diameter != 0.0) && (wire_lead_length != 0.0))
  {
    if (subch_type == EChannelType::CENTER)
    {
      // wetted perimeter for center subchannel and bare rod bundle
      pw_p = libMesh::pi * rod_diameter / 2.0;
      // wire projected area - center subchannel wire-wrapped bundle
      ar = libMesh::pi * (rod_diameter + wire_diameter) * wire_diameter / 6.0;
      // bare rod bundle center subchannel flow area (normal area + wire area)
      a_p = S + libMesh::pi * std::pow(wire_diameter, 2.0) / 8.0 / std::cos(theta);
      // turbulent friction factor equation constant - Center subchannel
      cT = cT * (pw_p / w_perim) + wd_t * (3 * ar / a_p) * (Dh_i / wire_lead_length) *
                                       std::pow((Dh_i / wire_diameter), 0.18);
      // laminar friction factor equation constant - Center subchannel
      cL = cL * (pw_p / w_perim) +
           wd_l * (3 * ar / a_p) * (Dh_i / wire_lead_length) * (Dh_i / wire_diameter);
    }
    else if (subch_type == EChannelType::EDGE)
    {
      // wire projected area - edge subchannel wire-wrapped bundle
      ar = libMesh::pi * (rod_diameter + wire_diameter) * wire_diameter / 4.0;
      // bare rod bundle edge subchannel flow area (normal area + wire area)
      a_p = S + libMesh::pi * std::pow(wire_diameter, 2.0) / 8.0 / std::cos(theta);
      // turbulent friction factor equation constant - Edge subchannel
      cT = cT * std::pow((1 + ws_t * (ar / a_p) * std::pow(std::tan(theta), 2.0)), 1.41);
      // laminar friction factor equation constant - Edge subchannel
      cL = cL * (1 + ws_l * (ar / a_p) * std::pow(std::tan(theta), 2.0));
    }
    else
    {
      // wire projected area - corner subchannel wire-wrapped bundle
      ar = libMesh::pi * (rod_diameter + wire_diameter) * wire_diameter / 6.0;
      // bare rod bundle corner subchannel flow area (normal area + wire area)
      a_p = S + libMesh::pi * std::pow(wire_diameter, 2.0) / 24.0 / std::cos(theta);
      // turbulent friction factor equation constant - Corner subchannel
      cT = cT * std::pow((1 + ws_t * (ar / a_p) * std::pow(std::tan(theta), 2.0)), 1.41);
      // laminar friction factor equation constant - Corner subchannel
      cL = cL * (1 + ws_l * (ar / a_p) * std::pow(std::tan(theta), 2.0));
    }
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
    // transition flow
    return fL * std::pow((1 - psi), 1.0 / 3.0) * (1 - std::pow(psi, lambda)) +
           fT * std::pow(psi, 1.0 / 3.0);
  }
}

void
LiquidMetalSubChannel1PhaseProblem::computeWijPrime(int iblock)
{
  unsigned int last_node = (iblock + 1) * _block_size;
  unsigned int first_node = iblock * _block_size + 1;
  const Real & pitch = _subchannel_mesh.getPitch();
  const Real & rod_diameter = _subchannel_mesh.getRodDiameter();
  const Real & wire_lead_length = _tri_sch_mesh.getWireLeadLength();
  const Real & wire_diameter = _tri_sch_mesh.getWireDiameter();
  for (unsigned int iz = first_node; iz < last_node + 1; iz++)
  {
    auto z_grid = _subchannel_mesh.getZGrid();
    auto dz = z_grid[iz] - z_grid[iz - 1];
    for (unsigned int i_gap = 0; i_gap < _n_gaps; i_gap++)
    {
      auto chans = _subchannel_mesh.getGapChannels(i_gap);
      unsigned int i_ch = chans.first;
      unsigned int j_ch = chans.second;
      auto subch_type1 = _subchannel_mesh.getSubchannelType(i_ch);
      auto subch_type2 = _subchannel_mesh.getSubchannelType(j_ch);
      auto * node_in_i = _subchannel_mesh.getChannelNode(i_ch, iz - 1);
      auto * node_out_i = _subchannel_mesh.getChannelNode(i_ch, iz);
      auto * node_in_j = _subchannel_mesh.getChannelNode(j_ch, iz - 1);
      auto * node_out_j = _subchannel_mesh.getChannelNode(j_ch, iz);
      auto Si_in = (*_S_flow_soln)(node_in_i);
      auto Sj_in = (*_S_flow_soln)(node_in_j);
      auto Si_out = (*_S_flow_soln)(node_out_i);
      auto Sj_out = (*_S_flow_soln)(node_out_j);
      auto S_total = Si_in + Sj_in + Si_out + Sj_out;
      auto Si = 0.5 * (Si_in + Si_out);
      auto Sj = 0.5 * (Sj_in + Sj_out);
      auto w_perim_i = 0.5 * ((*_w_perim_soln)(node_in_i) + (*_w_perim_soln)(node_out_i));
      auto w_perim_j = 0.5 * ((*_w_perim_soln)(node_in_j) + (*_w_perim_soln)(node_out_j));
      auto avg_mu = (1 / S_total) * ((*_mu_soln)(node_out_i)*Si_out + (*_mu_soln)(node_in_i)*Si_in +
                                     (*_mu_soln)(node_out_j)*Sj_out + (*_mu_soln)(node_in_j)*Sj_in);
      auto avg_hD = 4.0 * (Si + Sj) / (w_perim_i + w_perim_j);
      auto avg_massflux =
          0.5 * (((*_mdot_soln)(node_in_i) + (*_mdot_soln)(node_in_j)) / (Si_in + Sj_in) +
                 ((*_mdot_soln)(node_out_i) + (*_mdot_soln)(node_out_j)) / (Si_out + Sj_out));
      auto Re = avg_massflux * avg_hD / avg_mu;
      // crossflow area between channels i,j (dz*gap_width)
      auto gap = _subchannel_mesh.getGapWidth(iz, i_gap);
      auto Sij = dz * gap;
      // Calculation of flow regime
      auto ReL = 320.0 * std::pow(10.0, pitch / rod_diameter - 1);
      auto ReT = 10000.0 * std::pow(10.0, 0.7 * (pitch / rod_diameter - 1));
      _WijPrime(i_gap, iz) = 0.0;
      auto beta = 0.0;
      // Calculation of Turbulent Crossflow for wire-wrapped triangular assemblies. Cheng & Todreas
      // (1986)
      if ((subch_type1 == EChannelType::CENTER || subch_type2 == EChannelType::CENTER) &&
          (wire_lead_length != 0) && (wire_diameter != 0))
      {
        // Calculation of geometric parameters
        auto theta =
            std::acos(wire_lead_length /
                      std::sqrt(std::pow(wire_lead_length, 2) +
                                std::pow(libMesh::pi * (rod_diameter + wire_diameter), 2)));
        auto Ar1 = libMesh::pi * (rod_diameter + wire_diameter) * wire_diameter / 6.0;
        auto A1prime = (std::sqrt(3.0) / 4.0) * std::pow(pitch, 2) -
                       libMesh::pi * std::pow(rod_diameter, 2) / 8.0;
        auto A1 = A1prime - libMesh::pi * std::pow(wire_diameter, 2) / 8.0 / std::cos(theta);
        auto Cm = 0.0;
        if (Re < ReL)
        {
          Cm = 0.077 * std::pow((pitch - rod_diameter) / rod_diameter, -0.5);
        }
        else if (Re > ReT)
        {
          Cm = 0.14 * std::pow((pitch - rod_diameter) / rod_diameter, -0.5);
        }
        else
        {
          auto psi = (std::log(Re) - std::log(ReL)) / (std::log(ReT) - std::log(ReL));
          auto gamma = 2.0 / 3.0;
          Cm = 0.14 * std::pow((pitch - rod_diameter) / rod_diameter, -0.5) +
               (0.14 * std::pow((pitch - rod_diameter) / rod_diameter, -0.5) -
                0.077 * std::pow((pitch - rod_diameter) / rod_diameter, -0.5)) *
                   std::pow(psi, gamma);
        }
        // Calculation of turbulent mixing parameter
        beta = Cm * std::pow(Ar1 / A1, 0.5) * std::tan(theta);
      }
      // Calculation of Turbulent Crossflow for bare assemblies, from Kim and Chung (2001).
      else if ((wire_lead_length == 0) && (wire_diameter == 0))
      {
        Real gamma = 20.0;   // empirical constant
        Real sf = 2.0 / 3.0; // shape factor
        Real a = 0.18;
        Real b = 0.2;
        auto f = a * std::pow(Re, -b); // Rehme 1992 circular tube friction factor
        auto k =
            (1 / S_total) *
            (_fp->k_from_p_T((*_P_soln)(node_out_i) + _P_out, (*_T_soln)(node_out_i)) * Si_out +
             _fp->k_from_p_T((*_P_soln)(node_in_i) + _P_out, (*_T_soln)(node_in_i)) * Si_in +
             _fp->k_from_p_T((*_P_soln)(node_out_j) + _P_out, (*_T_soln)(node_out_j)) * Sj_out +
             _fp->k_from_p_T((*_P_soln)(node_in_j) + _P_out, (*_T_soln)(node_in_j)) * Sj_in);
        auto cp =
            (1 / S_total) *
            (_fp->cp_from_p_T((*_P_soln)(node_out_i) + _P_out, (*_T_soln)(node_out_i)) * Si_out +
             _fp->cp_from_p_T((*_P_soln)(node_in_i) + _P_out, (*_T_soln)(node_in_i)) * Si_in +
             _fp->cp_from_p_T((*_P_soln)(node_out_j) + _P_out, (*_T_soln)(node_out_j)) * Sj_out +
             _fp->cp_from_p_T((*_P_soln)(node_in_j) + _P_out, (*_T_soln)(node_in_j)) * Sj_in);
        auto Pr = avg_mu * cp / k;                          // Prandtl number
        auto Pr_t = Pr * (Re / gamma) * std::sqrt(f / 8.0); // Turbulent Prandtl number
        auto delta = pitch / std::sqrt(3.0);                // centroid to centroid distance
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
      // Calculation of turbulent crossflow
      _WijPrime(i_gap, iz) = beta * avg_massflux * Sij;
    }
  }
}

Real
LiquidMetalSubChannel1PhaseProblem::computeAddedHeatPin(unsigned int i_ch, unsigned int iz)
{
  auto dz = _z_grid[iz] - _z_grid[iz - 1];
  auto subch_type = _subchannel_mesh.getSubchannelType(i_ch);
  // If pin mesh exists, then project pin power to subchannel
  if (_pin_mesh_exist)
  {
    auto heat_rate_in = 0.0;
    auto heat_rate_out = 0.0;
    if (subch_type == EChannelType::CENTER)
    {
      for (unsigned int j = 0; j < 3; j++)
      {
        auto i_pin = _subchannel_mesh.getChannelPins(i_ch)[j];
        auto * node_in = _subchannel_mesh.getPinNode(i_pin, iz - 1);
        auto * node_out = _subchannel_mesh.getPinNode(i_pin, iz);
        heat_rate_out += (*_q_prime_soln)(node_out);
        heat_rate_in += (*_q_prime_soln)(node_in);
      }
      return 1.0 / 6.0 * (heat_rate_in + heat_rate_out) * dz / 2.0;
    }
    else if (subch_type == EChannelType::EDGE)
    {
      for (unsigned int j = 0; j < 2; j++)
      {
        auto i_pin = _subchannel_mesh.getChannelPins(i_ch)[j];
        auto * node_in = _subchannel_mesh.getPinNode(i_pin, iz - 1);
        auto * node_out = _subchannel_mesh.getPinNode(i_pin, iz);
        heat_rate_out += (*_q_prime_soln)(node_out);
        heat_rate_in += (*_q_prime_soln)(node_in);
      }
      return 1.0 / 4.0 * (heat_rate_in + heat_rate_out) * dz / 2.0;
    }
    else
    {
      auto i_pin = _subchannel_mesh.getChannelPins(i_ch)[0];
      auto * node_in = _subchannel_mesh.getPinNode(i_pin, iz - 1);
      auto * node_out = _subchannel_mesh.getPinNode(i_pin, iz);
      heat_rate_out += (*_q_prime_soln)(node_out);
      heat_rate_in += (*_q_prime_soln)(node_in);
      return 1.0 / 6.0 * (heat_rate_in + heat_rate_out) * dz / 2.0;
    }
  }
  // If pin mesh does not exist, then apply power to subchannel directly
  // Note: this power was already set by  TriPowerIC
  else
  {
    auto * node_in = _subchannel_mesh.getChannelNode(i_ch, iz - 1);
    auto * node_out = _subchannel_mesh.getChannelNode(i_ch, iz);
    return ((*_q_prime_soln)(node_out) + (*_q_prime_soln)(node_in)) * dz / 2.0;
  }
}

void
LiquidMetalSubChannel1PhaseProblem::computeh(int iblock)
{
  unsigned int last_node = (iblock + 1) * _block_size;
  unsigned int first_node = iblock * _block_size + 1;
  auto heated_length = _subchannel_mesh.getHeatedLength();
  auto unheated_length_entry = _subchannel_mesh.getHeatedLengthEntry();
  const Real & wire_lead_length = _tri_sch_mesh.getWireLeadLength();
  const Real & wire_diameter = _tri_sch_mesh.getWireDiameter();
  const Real & pitch = _subchannel_mesh.getPitch();
  const Real & rod_diameter = _subchannel_mesh.getRodDiameter();

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
        if (z_grid[iz] > unheated_length_entry &&
            z_grid[iz] <= unheated_length_entry + heated_length)
        {
          added_enthalpy = computeAddedHeatPin(i_ch, iz);
        }
        else
          added_enthalpy = 0.0;

        added_enthalpy += computeAddedHeatDuct(i_ch, iz);

        // compute the sweep flow enthalpy change
        auto subch_type = _subchannel_mesh.getSubchannelType(i_ch);
        Real sweep_enthalpy = 0.0;

        if ((subch_type == EChannelType::EDGE || subch_type == EChannelType::CORNER) &&
            (wire_diameter != 0.0) && (wire_lead_length != 0.0))
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
          auto Sij = dz * gap;
          auto Si = (*_S_flow_soln)(node_in);
          // in/out channels for i_ch
          auto sweep_in = _tri_sch_mesh.getSweepFlowChans(i_ch).first;
          auto * node_sin = _subchannel_mesh.getChannelNode(sweep_in, iz - 1);

          // Calculation of flow regime
          auto ReL = 320.0 * std::pow(10.0, pitch / rod_diameter - 1);
          auto ReT = 10000.0 * std::pow(10.0, 0.7 * (pitch / rod_diameter - 1));
          auto massflux = (*_mdot_soln)(node_in) / Si;
          auto w_perim = (*_w_perim_soln)(node_in);
          auto mu = (*_mu_soln)(node_in);
          // hydraulic diameter
          auto hD = 4.0 * Si / w_perim;
          auto Re = massflux * hD / mu;
          // Calculation of geometric parameters
          auto Ar2 = libMesh::pi * (rod_diameter + wire_diameter) * wire_diameter / 4.0;
          auto A2prime =
              pitch * (w - rod_diameter / 2.0) - libMesh::pi * std::pow(rod_diameter, 2) / 8.0;
          auto A2 = A2prime - libMesh::pi * std::pow(wire_diameter, 2) / 8.0 / std::cos(theta);
          auto Cs = 0.0;
          if (Re < ReL)
          {
            Cs = 0.033 * std::pow(wire_lead_length / rod_diameter, 0.3);
          }
          else if (Re > ReT)
          {
            Cs = 0.75 * std::pow(wire_lead_length / rod_diameter, 0.3);
          }
          else
          {
            auto psi = (std::log(Re) - std::log(ReL)) / (std::log(ReT) - std::log(ReL));
            auto gamma = 2.0 / 3.0;
            Cs = 0.75 * std::pow(wire_lead_length / rod_diameter, 0.3) +
                 (0.75 * std::pow(wire_lead_length / rod_diameter, 0.3) -
                  0.033 * std::pow(wire_lead_length / rod_diameter, 0.3)) *
                     std::pow(psi, gamma);
          }
          // Calculation of turbulent mixing parameter
          auto beta = Cs * std::pow(Ar2 / A2, 0.5) * std::tan(theta);

          auto wsweep_in = gedge_ave * beta * Sij;
          auto wsweep_out = gedge_ave * beta * Sij;
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

          auto Sij = dz * _subchannel_mesh.getGapWidth(iz, i_gap);
          auto thcon_i = _fp->k_from_p_T((*_P_soln)(node_in_i) + _P_out, (*_T_soln)(node_in_i));
          auto thcon_j = _fp->k_from_p_T((*_P_soln)(node_in_j) + _P_out, (*_T_soln)(node_in_j));
          auto shape_factor =
              0.66 * (pitch / rod_diameter) *
              std::pow((_subchannel_mesh.getGapWidth(iz, i_gap) / rod_diameter), -0.3);
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
        auto thcon_i = _fp->k_from_p_T((*_P_soln)(node_in_i) + _P_out, (*_T_soln)(node_in_i));
        auto thcon_j = _fp->k_from_p_T((*_P_soln)(node_in_j) + _P_out, (*_T_soln)(node_in_j));
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
          auto thcon_i = _fp->k_from_p_T((*_P_soln)(node_in_i) + _P_out, (*_T_soln)(node_in_i));
          auto thcon_j = _fp->k_from_p_T((*_P_soln)(node_in_j) + _P_out, (*_T_soln)(node_in_j));
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
        auto S_interp = computeInterpolatedValue(S_out, S_in, "central_difference", 0.5);
        auto volume = dz * S_interp;

        PetscScalar Pe = 0.5;
        if (_interpolation_scheme.compare("exponential") == 0)
        {
          // Compute the Peclet number
          auto w_perim_in = (*_w_perim_soln)(node_in);
          auto w_perim_out = (*_w_perim_soln)(node_out);
          auto w_perim_interp =
              this->computeInterpolatedValue(w_perim_out, w_perim_in, "central_difference", 0.5);
          auto K_in = _fp->k_from_p_T((*_P_soln)(node_in) + _P_out, (*_T_soln)(node_in));
          auto K_out = _fp->k_from_p_T((*_P_soln)(node_out) + _P_out, (*_T_soln)(node_out));
          auto K = this->computeInterpolatedValue(K_out, K_in, "central_difference", 0.5);
          auto cp_in = _fp->cp_from_p_T((*_P_soln)(node_in) + _P_out, (*_T_soln)(node_in));
          auto cp_out = _fp->cp_from_p_T((*_P_soln)(node_out) + _P_out, (*_T_soln)(node_out));
          auto cp = this->computeInterpolatedValue(cp_out, cp_in, "central_difference", 0.5);
          auto mdot_loc = this->computeInterpolatedValue(
              (*_mdot_soln)(node_out), (*_mdot_soln)(node_in), "central_difference", 0.5);
          // hydraulic diameter in the i direction
          auto Dh_i = 4.0 * S_interp / w_perim_interp;
          Pe = mdot_loc * Dh_i * cp / (K * S_interp) * (mdot_loc / std::abs(mdot_loc));
        }
        auto alpha = computeInterpolationCoefficients(_interpolation_scheme, Pe);

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
            _rho_soln->old(node_out), _rho_soln->old(node_in), _interpolation_scheme, Pe);
        PetscScalar h_old_interp = computeInterpolatedValue(
            _h_soln->old(node_out), _h_soln->old(node_in), _interpolation_scheme, Pe);
        PetscScalar value_vec_tt = _TR * rho_old_interp * h_old_interp * volume / _dt;
        PetscInt row_vec_tt = i_ch + _n_channels * iz_ind;
        VecSetValues(_hc_time_derivative_rhs, 1, &row_vec_tt, &value_vec_tt, ADD_VALUES);

        /// Advective derivative term
        if (iz == first_node)
        {
          PetscInt row_at = i_ch + _n_channels * iz_ind;
          PetscScalar value_at = alpha * (*_mdot_soln)(node_in) * (*_h_soln)(node_in);
          VecSetValues(_hc_advective_derivative_rhs, 1, &row_at, &value_at, ADD_VALUES);

          value_at = alpha * (*_mdot_soln)(node_out) - (1 - alpha) * (*_mdot_soln)(node_in);
          PetscInt col_at = i_ch + _n_channels * iz_ind;
          MatSetValues(_hc_advective_derivative_mat, 1, &row_at, 1, &col_at, &value_at, ADD_VALUES);

          value_at = (1 - alpha) * (*_mdot_soln)(node_out);
          col_at = i_ch + _n_channels * (iz_ind + 1);
          MatSetValues(_hc_advective_derivative_mat, 1, &row_at, 1, &col_at, &value_at, ADD_VALUES);
        }
        else if (iz == last_node)
        {
          PetscInt row_at = i_ch + _n_channels * iz_ind;
          PetscScalar value_at = 1.0 * (*_mdot_soln)(node_out);
          PetscInt col_at = i_ch + _n_channels * iz_ind;
          MatSetValues(_hc_advective_derivative_mat, 1, &row_at, 1, &col_at, &value_at, ADD_VALUES);

          value_at = -1.0 * (*_mdot_soln)(node_in);
          col_at = i_ch + _n_channels * (iz_ind - 1);
          MatSetValues(_hc_advective_derivative_mat, 1, &row_at, 1, &col_at, &value_at, ADD_VALUES);
        }
        else
        {
          PetscInt row_at = i_ch + _n_channels * iz_ind;
          PetscInt col_at;

          PetscScalar value_at = -alpha * (*_mdot_soln)(node_in);
          col_at = i_ch + _n_channels * (iz_ind - 1);
          MatSetValues(_hc_advective_derivative_mat, 1, &row_at, 1, &col_at, &value_at, ADD_VALUES);

          value_at = alpha * (*_mdot_soln)(node_out) - (1 - alpha) * (*_mdot_soln)(node_in);
          col_at = i_ch + _n_channels * iz_ind;
          MatSetValues(_hc_advective_derivative_mat, 1, &row_at, 1, &col_at, &value_at, ADD_VALUES);

          value_at = (1 - alpha) * (*_mdot_soln)(node_out);
          col_at = i_ch + _n_channels * (iz_ind + 1);
          MatSetValues(_hc_advective_derivative_mat, 1, &row_at, 1, &col_at, &value_at, ADD_VALUES);
        }

        /// Axial heat conduction
        auto * node_center = _subchannel_mesh.getChannelNode(i_ch, iz);
        auto K_center = _fp->k_from_p_T((*_P_soln)(node_center) + _P_out, (*_T_soln)(node_center));
        auto cp_center =
            _fp->cp_from_p_T((*_P_soln)(node_center) + _P_out, (*_T_soln)(node_center));
        auto diff_center = K_center / (cp_center + 1e-15);

        if (iz == first_node)
        {
          auto * node_top = _subchannel_mesh.getChannelNode(i_ch, iz + 1);
          auto * node_bottom = _subchannel_mesh.getChannelNode(i_ch, iz - 1);
          auto K_bottom =
              _fp->k_from_p_T((*_P_soln)(node_bottom) + _P_out, (*_T_soln)(node_bottom));
          auto K_top = _fp->k_from_p_T((*_P_soln)(node_top) + _P_out, (*_T_soln)(node_top));
          auto cp_bottom =
              _fp->cp_from_p_T((*_P_soln)(node_bottom) + _P_out, (*_T_soln)(node_bottom));
          auto cp_top = _fp->cp_from_p_T((*_P_soln)(node_top) + _P_out, (*_T_soln)(node_top));
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
          auto K_bottom =
              _fp->k_from_p_T((*_P_soln)(node_bottom) + _P_out, (*_T_soln)(node_bottom));
          auto cp_bottom =
              _fp->cp_from_p_T((*_P_soln)(node_bottom) + _P_out, (*_T_soln)(node_bottom));
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
          auto K_bottom =
              _fp->k_from_p_T((*_P_soln)(node_bottom) + _P_out, (*_T_soln)(node_bottom));
          auto K_top = _fp->k_from_p_T((*_P_soln)(node_top) + _P_out, (*_T_soln)(node_top));
          auto cp_bottom =
              _fp->cp_from_p_T((*_P_soln)(node_bottom) + _P_out, (*_T_soln)(node_bottom));
          auto cp_top = _fp->cp_from_p_T((*_P_soln)(node_top) + _P_out, (*_T_soln)(node_top));
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

          auto Sij = dz * _subchannel_mesh.getGapWidth(iz, i_gap);
          auto K_i = _fp->k_from_p_T((*_P_soln)(node_in_i) + _P_out, (*_T_soln)(node_in_i));
          auto K_j = _fp->k_from_p_T((*_P_soln)(node_in_j) + _P_out, (*_T_soln)(node_in_j));
          auto cp_i = _fp->cp_from_p_T((*_P_soln)(node_in_i) + _P_out, (*_T_soln)(node_in_i));
          auto cp_j = _fp->cp_from_p_T((*_P_soln)(node_in_j) + _P_out, (*_T_soln)(node_in_j));
          auto A_i = K_i / cp_i;
          auto A_j = K_j / cp_j;
          auto harm_A = 2.0 * A_i * A_j / (A_i + A_j);
          auto shape_factor =
              0.66 * (pitch / rod_diameter) *
              std::pow((_subchannel_mesh.getGapWidth(iz, i_gap) / rod_diameter), -0.3);
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
        if ((subch_type == EChannelType::EDGE || subch_type == EChannelType::CORNER) &&
            (wire_diameter != 0.0) && (wire_lead_length != 0.0))
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
          auto Sij = dz * gap;
          auto Si = (*_S_flow_soln)(node_in);
          // in/out channels for i_ch
          auto sweep_in = _tri_sch_mesh.getSweepFlowChans(i_ch).first;
          auto * node_sin = _subchannel_mesh.getChannelNode(sweep_in, iz - 1);

          // Calculation of flow regime
          auto ReL = 320.0 * std::pow(10.0, pitch / rod_diameter - 1);
          auto ReT = 10000.0 * std::pow(10.0, 0.7 * (pitch / rod_diameter - 1));
          auto massflux = (*_mdot_soln)(node_in) / Si;
          auto w_perim = (*_w_perim_soln)(node_in);
          auto mu = (*_mu_soln)(node_in);
          // hydraulic diameter
          auto hD = 4.0 * Si / w_perim;
          auto Re = massflux * hD / mu;
          // Calculation of geometric parameters
          auto Ar2 = libMesh::pi * (rod_diameter + wire_diameter) * wire_diameter / 4.0;
          auto A2prime =
              pitch * (w - rod_diameter / 2.0) - libMesh::pi * std::pow(rod_diameter, 2) / 8.0;
          auto A2 = A2prime - libMesh::pi * std::pow(wire_diameter, 2) / 8.0 / std::cos(theta);
          auto Cs = 0.0;
          if (Re < ReL)
          {
            Cs = 0.033 * std::pow(wire_lead_length / rod_diameter, 0.3);
          }
          else if (Re > ReT)
          {
            Cs = 0.75 * std::pow(wire_lead_length / rod_diameter, 0.3);
          }
          else
          {
            auto psi = (std::log(Re) - std::log(ReL)) / (std::log(ReT) - std::log(ReL));
            auto gamma = 2.0 / 3.0;
            Cs = 0.75 * std::pow(wire_lead_length / rod_diameter, 0.3) +
                 (0.75 * std::pow(wire_lead_length / rod_diameter, 0.3) -
                  0.033 * std::pow(wire_lead_length / rod_diameter, 0.3)) *
                     std::pow(psi, gamma);
          }
          // Calculation of turbulent mixing parameter
          auto beta = Cs * std::pow(Ar2 / A2, 0.5) * std::tan(theta);

          auto wsweep_in = gedge_ave * beta * Sij;
          auto wsweep_out = gedge_ave * beta * Sij;
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
        added_enthalpy += computeAddedHeatDuct(i_ch, iz);
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
    /// Add all matrices together
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
