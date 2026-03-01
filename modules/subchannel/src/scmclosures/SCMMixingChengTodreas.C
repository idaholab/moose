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
  params.addClassDescription("Class that models the turbulent mixing coefficient for wire-wrapped "
                             "trianguar assemblies using the Cheng Todreas correlations.");
  return params;
}

SCMMixingChengTodreas::SCMMixingChengTodreas(const InputParameters & parameters)
  : SCMMixingClosureBase(parameters),
    _is_tri_lattice(dynamic_cast<const TriSubChannelMesh *>(&_subchannel_mesh) != nullptr),
    _tri_sch_mesh(dynamic_cast<const TriSubChannelMesh *>(&_subchannel_mesh)),
    _S_soln(_subproblem.getVariable(0, "S")),
    _mdot_soln(_subproblem.getVariable(0, "mdot")),
    _w_perim_soln(_subproblem.getVariable(0, "w_perim")),
    _mu_soln(_subproblem.getVariable(0, "mu"))
{
}

Real
SCMMixingChengTodreas::computeMixingParameter(const unsigned int i_gap,
                                              const unsigned int iz,
                                              const bool sweep_flow) const
{
  if (!_is_tri_lattice)
    mooseError("This corelation applies only for triangular assemblies");

  Real beta = std::numeric_limits<double>::quiet_NaN();

  const Real pitch = _subchannel_mesh.getPitch();
  const Real pin_diameter = _subchannel_mesh.getPinDiameter();

  const Real wire_lead_length = _tri_sch_mesh->getWireLeadLength();
  const Real wire_diameter = _tri_sch_mesh->getWireDiameter();

  if (wire_lead_length == 0 && wire_diameter == 0)
    mooseError("This corelation applies only for wire-wrapped assemblies");

  const auto chans = _subchannel_mesh.getGapChannels(i_gap);
  const unsigned int i_ch = chans.first;
  const unsigned int j_ch = chans.second;

  const unsigned int Nr = _tri_sch_mesh->getNumOfRings();

  const auto subch_type_i = _subchannel_mesh.getSubchannelType(i_ch);
  const auto subch_type_j = _subchannel_mesh.getSubchannelType(j_ch);

  const Node * const node_in_i = _subchannel_mesh.getChannelNode(i_ch, iz - 1);
  const Node * const node_out_i = _subchannel_mesh.getChannelNode(i_ch, iz);
  const Node * const node_in_j = _subchannel_mesh.getChannelNode(j_ch, iz - 1);
  const Node * const node_out_j = _subchannel_mesh.getChannelNode(j_ch, iz);

  const Real Si_in = _S_soln(node_in_i);
  const Real Sj_in = _S_soln(node_in_j);
  const Real Si_out = _S_soln(node_out_i);
  const Real Sj_out = _S_soln(node_out_j);

  const Real S_total = Si_in + Sj_in + Si_out + Sj_out;
  const Real Si = 0.5 * (Si_in + Si_out);
  const Real Sj = 0.5 * (Sj_in + Sj_out);

  const Real w_perim_i = 0.5 * (_w_perim_soln(node_in_i) + _w_perim_soln(node_out_i));
  const Real w_perim_j = 0.5 * (_w_perim_soln(node_in_j) + _w_perim_soln(node_out_j));

  const Real avg_mu =
      (1.0 / S_total) * (_mu_soln(node_out_i) * Si_out + _mu_soln(node_in_i) * Si_in +
                         _mu_soln(node_out_j) * Sj_out + _mu_soln(node_in_j) * Sj_in);

  const Real avg_hD = 4.0 * (Si + Sj) / (w_perim_i + w_perim_j);

  const Real avg_massflux =
      0.5 * ((_mdot_soln(node_in_i) + _mdot_soln(node_in_j)) / (Si_in + Sj_in) +
             (_mdot_soln(node_out_i) + _mdot_soln(node_out_j)) / (Si_out + Sj_out));

  const Real Re = avg_massflux * avg_hD / avg_mu;

  // Calculation of flow regime
  const Real ReL = 320.0 * std::pow(10.0, pitch / pin_diameter - 1.0);
  const Real ReT = 10000.0 * std::pow(10.0, 0.7 * (pitch / pin_diameter - 1.0));

  // Calculation of Turbulent Crossflow for wire-wrapped triangular assemblies. Cheng &
  // Todreas (1986).
  // INNER SUBCHANNELS
  if ((subch_type_i == EChannelType::CENTER || subch_type_j == EChannelType::CENTER) &&
      (wire_lead_length != 0) && (wire_diameter != 0))
  {
    // Calculation of geometric parameters
    // wire angle
    const Real theta =
        std::acos(wire_lead_length /
                  std::sqrt(Utility::pow<2>(wire_lead_length) +
                            Utility::pow<2>(libMesh::pi * (pin_diameter + wire_diameter))));

    // projected area of wire on subchannel
    const Real Ar1 = libMesh::pi * (pin_diameter + wire_diameter) * wire_diameter / 6.0;

    // bare subchannel flow area
    const Real A1prime = (std::sqrt(3.0) / 4.0) * Utility::pow<2>(pitch) -
                         libMesh::pi * Utility::pow<2>(pin_diameter) / 8.0;

    // wire-wrapped subchannel flow area
    const Real A1 = A1prime - libMesh::pi * Utility::pow<2>(wire_diameter) / 8.0 / std::cos(theta);

    // empirical constant for mixing parameter
    Real Cm = 0.0;
    Real CmL_constant = 0.0;
    Real CmT_constant = 0.0;

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

    const Real CmT = CmT_constant * std::pow((pitch - pin_diameter) / pin_diameter, -0.5);
    const Real CmL = CmL_constant * std::pow((pitch - pin_diameter) / pin_diameter, -0.5);

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
      const Real psi = (std::log(Re) - std::log(ReL)) / (std::log(ReT) - std::log(ReL));
      const Real gamma = 2.0 / 3.0;
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
    const Real theta =
        std::acos(wire_lead_length /
                  std::sqrt(Utility::pow<2>(wire_lead_length) +
                            Utility::pow<2>(libMesh::pi * (pin_diameter + wire_diameter))));

    // Calculation of geometric parameters
    // distance from pin surface to duct
    const Real dpgap = _tri_sch_mesh->getDuctToPinGap();

    // Edge pitch parameter defined as pin diameter plus distance to duct wall
    const Real w = pin_diameter + dpgap;

    const Real Ar2 = libMesh::pi * (pin_diameter + wire_diameter) * wire_diameter / 4.0;

    const Real A2prime =
        pitch * (w - pin_diameter / 2.0) - libMesh::pi * Utility::pow<2>(pin_diameter) / 8.0;

    const Real A2 = A2prime - libMesh::pi * Utility::pow<2>(wire_diameter) / 8.0 / std::cos(theta);

    // empirical constant for mixing parameter
    Real Cs = 0.0;
    Real CsL_constant = 0.0;
    Real CsT_constant = 0.0;

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

    const Real CsL = CsL_constant * std::pow(wire_lead_length / pin_diameter, 0.3);
    const Real CsT = CsT_constant * std::pow(wire_lead_length / pin_diameter, 0.3);

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
      const Real psi = (std::log(Re) - std::log(ReL)) / (std::log(ReT) - std::log(ReL));
      const Real gamma = 2.0 / 3.0;
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
