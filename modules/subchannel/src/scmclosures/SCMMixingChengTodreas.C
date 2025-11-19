//* This file is part of the MOOSE framework
//* https://mooseframework.inl.gov
//*
//* All rights reserved, see COPYRIGHT for full restrictions
//* https://github.com/idaholab/moose/blob/master/COPYRIGHT
//*
//* Licensed under LGPL 2.1, please see LICENSE for details
//* https://www.gnu.org/licenses/lgpl-2.1.html

#include "SCMMixingChengTodreas.h"

registerMooseObject("SubChannelApp", SCMMixingChengTodreas);

InputParameters
SCMMixingChengTodreas::validParams()
{
  InputParameters params = SCMMixingClosureBase::validParams();
  params.addClassDescription("Class that models the turbulent mixing parameter for wire-wrapped "
                             "trianguar assemblies using the Cheng Todreas correlations.");
  return params;
}

SCMMixingChengTodreas::SCMMixingChengTodreas(const InputParameters & parameters)
  : SCMMixingClosureBase(parameters),
    _is_tri_lattice(dynamic_cast<const TriSubChannelMesh *>(&_subchannel_mesh) != nullptr),
    _tri_sch_mesh(dynamic_cast<const TriSubChannelMesh *>(&_subchannel_mesh))
{
}

Real
SCMMixingChengTodreas::computeMixingParameter(const unsigned int & i_gap,
                                              const unsigned int & iz,
                                              const bool & sweep_flow) const
{
  if (!_is_tri_lattice)
    mooseError("This corelation applies only for triangular assemblies");
  auto beta = std::numeric_limits<double>::quiet_NaN();
  const Real & pitch = _subchannel_mesh.getPitch();
  const Real & pin_diameter = _subchannel_mesh.getPinDiameter();
  const Real & wire_lead_length = _tri_sch_mesh->getWireLeadLength();
  const Real & wire_diameter = _tri_sch_mesh->getWireDiameter();
  if (wire_lead_length == 0 && wire_diameter == 0)
    mooseError("This corelation applies only for wire-wrapped assemblies");
  auto S_soln = SolutionHandle(_subproblem.getVariable(0, "S"));
  auto mdot_soln = SolutionHandle(_subproblem.getVariable(0, "mdot"));
  auto w_perim_soln = SolutionHandle(_subproblem.getVariable(0, "w_perim"));
  auto mu_soln = SolutionHandle(_subproblem.getVariable(0, "mu"));
  auto chans = _subchannel_mesh.getGapChannels(i_gap);
  auto Nr = _tri_sch_mesh->getNumOfRings();
  unsigned int i_ch = chans.first;
  unsigned int j_ch = chans.second;
  auto subch_type_i = _subchannel_mesh.getSubchannelType(i_ch);
  auto subch_type_j = _subchannel_mesh.getSubchannelType(j_ch);
  auto * node_in_i = _subchannel_mesh.getChannelNode(i_ch, iz - 1);
  auto * node_out_i = _subchannel_mesh.getChannelNode(i_ch, iz);
  auto * node_in_j = _subchannel_mesh.getChannelNode(j_ch, iz - 1);
  auto * node_out_j = _subchannel_mesh.getChannelNode(j_ch, iz);
  auto Si_in = (S_soln)(node_in_i);
  auto Sj_in = (S_soln)(node_in_j);
  auto Si_out = (S_soln)(node_out_i);
  auto Sj_out = (S_soln)(node_out_j);
  auto S_total = Si_in + Sj_in + Si_out + Sj_out;
  auto Si = 0.5 * (Si_in + Si_out);
  auto Sj = 0.5 * (Sj_in + Sj_out);
  auto w_perim_i = 0.5 * ((w_perim_soln)(node_in_i) + (w_perim_soln)(node_out_i));
  auto w_perim_j = 0.5 * ((w_perim_soln)(node_in_j) + (w_perim_soln)(node_out_j));
  auto avg_mu = (1 / S_total) * ((mu_soln)(node_out_i)*Si_out + (mu_soln)(node_in_i)*Si_in +
                                 (mu_soln)(node_out_j)*Sj_out + (mu_soln)(node_in_j)*Sj_in);
  auto avg_hD = 4.0 * (Si + Sj) / (w_perim_i + w_perim_j);
  auto avg_massflux =
      0.5 * (((mdot_soln)(node_in_i) + (mdot_soln)(node_in_j)) / (Si_in + Sj_in) +
             ((mdot_soln)(node_out_i) + (mdot_soln)(node_out_j)) / (Si_out + Sj_out));
  auto Re = avg_massflux * avg_hD / avg_mu;
  // Calculation of flow regime
  auto ReL = 320.0 * std::pow(10.0, pitch / pin_diameter - 1);
  auto ReT = 10000.0 * std::pow(10.0, 0.7 * (pitch / pin_diameter - 1));
  // Calculation of Turbulent Crossflow for wire-wrapped triangular assemblies. Cheng &
  // Todreas (1986).
  // INNER SUBCHANNELS
  if ((subch_type_i == EChannelType::CENTER || subch_type_j == EChannelType::CENTER) &&
      (wire_lead_length != 0) && (wire_diameter != 0))
  {
    // Calculation of geometric parameters
    // wire angle
    auto theta =
        std::acos(wire_lead_length /
                  std::sqrt(Utility::pow<2>(wire_lead_length) +
                            Utility::pow<2>(libMesh::pi * (pin_diameter + wire_diameter))));
    // projected area of wire on subchannel
    auto Ar1 = libMesh::pi * (pin_diameter + wire_diameter) * wire_diameter / 6.0;
    // bare subchannel flow area
    auto A1prime = (std::sqrt(3.0) / 4.0) * Utility::pow<2>(pitch) -
                   libMesh::pi * Utility::pow<2>(pin_diameter) / 8.0;
    // wire-wrapped subchannel flow area
    auto A1 = A1prime - libMesh::pi * Utility::pow<2>(wire_diameter) / 8.0 / std::cos(theta);
    // empirical constant for mixing parameter
    auto Cm = 0.0;
    auto CmL_constant = 0.0;
    auto CmT_constant = 0.0;

    if (Nr == 1)
    {
      CmT_constant = 0.1;
      CmL_constant = 0.055;
    }
    else
    {
      CmT_constant = 0.14;
      CmL_constant = 0.077;
    }

    auto CmT = CmT_constant * std::pow((pitch - pin_diameter) / pin_diameter, -0.5);
    auto CmL = CmL_constant * std::pow((pitch - pin_diameter) / pin_diameter, -0.5);

    if (Re < ReL)
    {
      Cm = CmL;
    }
    else if (Re > ReT)
    {
      Cm = CmT;
    }
    else
    {
      auto psi = (std::log(Re) - std::log(ReL)) / (std::log(ReT) - std::log(ReL));
      auto gamma = 2.0 / 3.0;
      Cm = CmL + (CmT - CmL) * std::pow(psi, gamma);
    }
    // mixing parameter
    beta = Cm * std::sqrt(Ar1 / A1) * std::tan(theta);
  }
  // EDGE OR CORNER SUBCHANNELS/ SWEEP FLOW
  else if ((subch_type_i == EChannelType::CORNER || subch_type_i == EChannelType::EDGE) &&
           (subch_type_j == EChannelType::CORNER || subch_type_j == EChannelType::EDGE) &&
           (wire_lead_length != 0) && (wire_diameter != 0))
  {
    auto theta =
        std::acos(wire_lead_length /
                  std::sqrt(Utility::pow<2>(wire_lead_length) +
                            Utility::pow<2>(libMesh::pi * (pin_diameter + wire_diameter))));
    // Calculation of geometric parameters
    // distance from pin surface to duct
    auto dpgap = _tri_sch_mesh->getDuctToPinGap();
    // Edge pitch parameter defined as pin diameter plus distance to duct wall
    auto w = pin_diameter + dpgap;
    auto Ar2 = libMesh::pi * (pin_diameter + wire_diameter) * wire_diameter / 4.0;
    auto A2prime =
        pitch * (w - pin_diameter / 2.0) - libMesh::pi * Utility::pow<2>(pin_diameter) / 8.0;
    auto A2 = A2prime - libMesh::pi * Utility::pow<2>(wire_diameter) / 8.0 / std::cos(theta);
    // empirical constant for mixing parameter
    auto Cs = 0.0;
    auto CsL_constant = 0.0;
    auto CsT_constant = 0.0;
    if (Nr == 1)
    {
      CsT_constant = 0.6;
      CsL_constant = 0.33;
    }
    else
    {
      CsT_constant = 0.75;
      CsL_constant = 0.413;
    }
    auto CsL = CsL_constant * std::pow(wire_lead_length / pin_diameter, 0.3);
    auto CsT = CsT_constant * std::pow(wire_lead_length / pin_diameter, 0.3);

    if (Re < ReL)
    {
      Cs = CsL;
    }
    else if (Re > ReT)
    {
      Cs = CsT;
    }
    else
    {
      auto psi = (std::log(Re) - std::log(ReL)) / (std::log(ReT) - std::log(ReL));
      auto gamma = 2.0 / 3.0;
      Cs = CsL + (CsT - CsL) * std::pow(psi, gamma);
    }
    // Calculation of turbulent mixing parameter used for sweep flow only in enthalpy calculation
    if (sweep_flow)
      beta = Cs * std::sqrt(Ar2 / A2) * std::tan(theta);
    else
      beta = 0.0;
  }
  return beta;
}
