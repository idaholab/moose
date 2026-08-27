//* This file is part of the MOOSE framework
//* https://mooseframework.inl.gov
//*
//* All rights reserved, see COPYRIGHT for full restrictions
//* https://github.com/idaholab/moose/blob/master/COPYRIGHT
//*
//* Licensed under LGPL 2.1, please see LICENSE for details
//* https://www.gnu.org/licenses/lgpl-2.1.html

#include "SCMMixingChenTodreas.h"

registerMooseObject("SubChannelApp", SCMMixingChenTodreas);

InputParameters
SCMMixingChenTodreas::validParams()
{
  InputParameters params = SCMMixingClosureBase::validParams();
  params.addClassDescription("Class that models the turbulent mixing coefficient for wire-wrapped "
                             "triangular assemblies using the Chen Todreas correlations.");
  MooseEnum mixing_model("1986 Pacio", "1986");
  params.addParam<MooseEnum>(
      "mixing_model",
      mixing_model,
      "Mixing-model parameterization for triangular wire-wrapped Chen-Todreas correlations.");
  return params;
}

SCMMixingChenTodreas::SCMMixingChenTodreas(const InputParameters & parameters)
  : SCMMixingClosureBase(parameters),
    _is_tri_lattice(dynamic_cast<const TriSubChannelMesh *>(&_subchannel_mesh) != nullptr),
    _tri_sch_mesh(dynamic_cast<const TriSubChannelMesh *>(&_subchannel_mesh)),
    _mixing_model(getParam<MooseEnum>("mixing_model")),
    _S_soln(_subproblem.getVariable(0, "S")),
    _mdot_soln(_subproblem.getVariable(0, "mdot")),
    _rho_soln(_subproblem.getVariable(0, "rho"))
{
  if (!_is_tri_lattice)
    mooseError("This correlation applies only for triangular assemblies");

  if (_tri_sch_mesh->getWireLeadLength() == 0 || _tri_sch_mesh->getWireDiameter() == 0)
    mooseError("This correlation applies only for wire-wrapped assemblies");

  const auto pitch = _subchannel_mesh.getPitch();
  const auto pin_diameter = _subchannel_mesh.getPinDiameter();
  const auto p_over_d = pitch / pin_diameter;
  const auto wire_lead_to_diameter = _tri_sch_mesh->getWireLeadLength() / pin_diameter;
  const unsigned int Nr = _tri_sch_mesh->getNumOfRings();
  const unsigned int num_pins = 1 + 3 * Nr * (Nr - 1);
  const auto Reb = _scm_problem.getBulkReynoldsNumber();

  if (_mixing_model == "1986")
  {
    if (p_over_d < 1.07 || p_over_d > 1.30)
      flagSolutionWarning("Pitch-over-pin diameter ratio (P/D) outside the 1986 "
                          "Chen-Todreas friction correlation data range.");
    if (wire_lead_to_diameter < 4.0 || wire_lead_to_diameter > 52.0)
      flagSolutionWarning("Wire lead length-over-pin diameter ratio (H/D) outside the 1986 "
                          "Chen-Todreas friction correlation data range.");
    if (num_pins < 7 || num_pins > 217)
      flagSolutionWarning("Number of pins outside the 1986 Chen-Todreas friction correlation "
                          "data range.");
    if (Reb < 400.0 || Reb > 1.0e6)
      flagSolutionWarning("Bulk Reynolds number (Reb) outside the 1986 Chen-Todreas friction "
                          "correlation data range.");
  }
  else
  {
    if (p_over_d < 1.02 || p_over_d > 1.42)
      flagSolutionWarning("Pitch-over-pin diameter ratio (P/D) outside the "
                          "Pacio-Chen-Todreas friction correlation data range.");
    if (wire_lead_to_diameter < 7.5 || wire_lead_to_diameter > 54.0)
      flagSolutionWarning("Wire lead length-over-pin diameter ratio (H/D) outside the "
                          "Pacio-Chen-Todreas friction correlation data range.");
    if (num_pins < 19 || num_pins > 217)
      flagSolutionWarning("Number of pins outside the Pacio-Chen-Todreas friction correlation "
                          "data range.");
    if (Reb < 10.0 || Reb > 3.0e5)
      flagSolutionWarning("Bulk Reynolds number (Reb) outside the Pacio-Chen-Todreas friction "
                          "correlation data range.");
  }
}

Real
SCMMixingChenTodreas::computeMixingParameter(const unsigned int i_gap, const unsigned int iz) const
{
  Real beta = 0.0;

  const Real pitch = _subchannel_mesh.getPitch();
  const Real pin_diameter = _subchannel_mesh.getPinDiameter();
  const Real p_over_d = pitch / pin_diameter;

  const Real wire_lead_length = _tri_sch_mesh->getWireLeadLength();
  const Real wire_diameter = _tri_sch_mesh->getWireDiameter();
  const unsigned int Nr = _tri_sch_mesh->getNumOfRings();

  const auto chans = _subchannel_mesh.getGapChannels(i_gap);
  const unsigned int i_ch = chans.first;
  const unsigned int j_ch = chans.second;

  const auto subch_type_i = _subchannel_mesh.getSubchannelType(i_ch);
  const auto subch_type_j = _subchannel_mesh.getSubchannelType(j_ch);

  const Node * const node_in_i = _subchannel_mesh.getChannelNode(i_ch, iz - 1);
  const Node * const node_out_i = _subchannel_mesh.getChannelNode(i_ch, iz);
  const Node * const node_in_j = _subchannel_mesh.getChannelNode(j_ch, iz - 1);
  const Node * const node_out_j = _subchannel_mesh.getChannelNode(j_ch, iz);

  // Surface area
  const Real Si_in = _S_soln(node_in_i);
  const Real Sj_in = _S_soln(node_in_j);
  const Real Si_out = _S_soln(node_out_i);
  const Real Sj_out = _S_soln(node_out_j);
  const Real Si = 0.5 * (Si_in + Si_out);
  const Real Sj = 0.5 * (Sj_in + Sj_out);

  const Real bulk_Re = _scm_problem.getBulkReynoldsNumber();

  // Calculation of flow regime
  Real ReL;
  Real ReT;
  if (_mixing_model == "1986")
  {
    ReL = 320.0 * std::pow(10.0, p_over_d - 1.0);
    ReT = 10000.0 * std::pow(10.0, 0.7 * (p_over_d - 1.0));
  }
  else
  {
    ReL = 700.0;
    ReT = 10000.0;
  }

  // This beta is used by the global turbulent crossflow relation:
  // w'_ij = beta * S_ij * G_bar. Peripheral sweep flow is handled separately by
  // computeSweepFlowMixingParameter().
  if (subch_type_i == EChannelType::CENTER || subch_type_j == EChannelType::CENTER)
  {
    // wire angle
    const Real theta =
        std::acos(wire_lead_length /
                  std::sqrt(Utility::pow<2>(wire_lead_length) +
                            Utility::pow<2>(libMesh::pi * (pin_diameter + wire_diameter))));

    // projected area of wire on center subchannel
    const Real Ar1 = libMesh::pi * (pin_diameter + wire_diameter) * wire_diameter / 6.0;

    // bare center-subchannel flow area
    const Real A1prime = (std::sqrt(3.0) / 4.0) * Utility::pow<2>(pitch) -
                         libMesh::pi * Utility::pow<2>(pin_diameter) / 8.0;

    // 1986 coefficients
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

    Real Cm = 0.0;

    if (_mixing_model == "1986")
    {
      // Preserve the original Chen-Todreas (1986) path.
      if (bulk_Re < ReL)
        Cm = CmL;
      else if (bulk_Re > ReT)
        Cm = CmT;
      else
      {
        const Real psi = std::log(bulk_Re / ReL) / std::log(ReT / ReL);
        const Real gamma = 2.0 / 3.0;
        Cm = CmL + (CmT - CmL) * std::pow(psi, gamma);
      }

      beta = Cm * std::sqrt(Ar1 / A1prime) * std::tan(theta);
    }
    else
    {
      // Pacio-only flow-split calculation.
      const Real rho_i_in = _rho_soln(node_in_i);
      const Real rho_j_in = _rho_soln(node_in_j);
      const Real rho_i_out = _rho_soln(node_out_i);
      const Real rho_j_out = _rho_soln(node_out_j);
      const Real rho_i = 0.5 * (rho_i_in + rho_i_out);
      const Real rho_j = 0.5 * (rho_j_in + rho_j_out);

      const Real Vi = 0.5 * (_mdot_soln(node_in_i) + _mdot_soln(node_out_i)) / (rho_i * Si);
      const Real Vj = 0.5 * (_mdot_soln(node_in_j) + _mdot_soln(node_out_j)) / (rho_j * Sj);

      const Real bulk_V = _scm_problem.getBulkVelocity();
      if (MooseUtils::absoluteFuzzyEqual(bulk_V, 0.0))
        return 0.0;

      const Real Xi = Vi / bulk_V;
      const Real Xj = Vj / bulk_V;

      constexpr Real flow_split_exponent = 2.0 - 0.18;
      Real fraction;
      if (MooseUtils::absoluteFuzzyEqual(Xi, Xj))
      {
        const Real Xavg = 0.5 * (Xi + Xj);
        if (Xavg < 0.0)
          mooseError("The Pacio mixing correlation does not support negative flow splits "
                     "when evaluating the fractional flow-split exponent.");
        fraction = flow_split_exponent * std::pow(Xavg, flow_split_exponent - 1.0);
      }
      else
      {
        if (Xi < 0.0 || Xj < 0.0)
          mooseError("The Pacio mixing correlation does not support negative flow splits "
                     "when evaluating the fractional flow-split exponent.");
        fraction =
            (std::pow(Xi, flow_split_exponent) - std::pow(Xj, flow_split_exponent)) / (Xi - Xj);
      }

      const Real WmL = 0.0;
      const Real WmT = 8.8 * fraction / std::pow(std::max(bulk_Re, 1.0), 0.18);

      if (bulk_Re < ReL)
        Cm = WmL;
      else if (bulk_Re > ReT)
        Cm = WmT;
      else
      {
        const Real psi = std::log(bulk_Re / ReL) / std::log(ReT / ReL);
        const Real gamma = 2.0 / 3.0;
        Cm = WmL + (WmT - WmL) * std::pow(psi, gamma);
      }

      // distance from pin surface to duct
      const Real dpgap = _tri_sch_mesh->getDuctToPinGap();

      // Edge pitch parameter defined as pin diameter plus distance to duct wall
      const Real w = pin_diameter + dpgap;

      const Real Ar2 = libMesh::pi * (pin_diameter + wire_diameter) * wire_diameter / 4.0;

      const Real A2prime =
          pitch * (w - pin_diameter / 2.0) - libMesh::pi * Utility::pow<2>(pin_diameter) / 8.0;

      beta = Cm * std::sqrt(Ar2 / A2prime) * std::tan(theta);
    }
  }

  return beta;
}

Real
SCMMixingChenTodreas::computeSweepFlowMixingParameter(const unsigned int i_gap,
                                                      const unsigned int iz) const
{
  Real beta = 0.0;

  const Real pitch = _subchannel_mesh.getPitch();
  const Real pin_diameter = _subchannel_mesh.getPinDiameter();
  const Real p_over_d = pitch / pin_diameter;

  const Real wire_lead_length = _tri_sch_mesh->getWireLeadLength();
  const Real wire_diameter = _tri_sch_mesh->getWireDiameter();
  const unsigned int Nr = _tri_sch_mesh->getNumOfRings();

  const auto chans = _subchannel_mesh.getGapChannels(i_gap);
  const unsigned int i_ch = chans.first;
  const unsigned int j_ch = chans.second;

  const auto subch_type_i = _subchannel_mesh.getSubchannelType(i_ch);
  const auto subch_type_j = _subchannel_mesh.getSubchannelType(j_ch);

  const Node * const node_in_i = _subchannel_mesh.getChannelNode(i_ch, iz - 1);
  const Node * const node_out_i = _subchannel_mesh.getChannelNode(i_ch, iz);
  const Node * const node_in_j = _subchannel_mesh.getChannelNode(j_ch, iz - 1);
  const Node * const node_out_j = _subchannel_mesh.getChannelNode(j_ch, iz);

  // Surface area
  const Real Si_in = _S_soln(node_in_i);
  const Real Sj_in = _S_soln(node_in_j);
  const Real Si_out = _S_soln(node_out_i);
  const Real Sj_out = _S_soln(node_out_j);
  const Real Si = 0.5 * (Si_in + Si_out);
  const Real Sj = 0.5 * (Sj_in + Sj_out);

  const Real bulk_Re = _scm_problem.getBulkReynoldsNumber();

  // Calculation of flow regime
  Real ReL;
  Real ReT;
  if (_mixing_model == "1986")
  {
    ReL = 320.0 * std::pow(10.0, p_over_d - 1.0);
    ReT = 10000.0 * std::pow(10.0, 0.7 * (p_over_d - 1.0));
  }
  else
  {
    ReL = 700.0;
    ReT = 10000.0;
  }

  if ((subch_type_i == EChannelType::CORNER || subch_type_i == EChannelType::EDGE) &&
      (subch_type_j == EChannelType::CORNER || subch_type_j == EChannelType::EDGE))
  {
    const Real theta =
        std::acos(wire_lead_length /
                  std::sqrt(Utility::pow<2>(wire_lead_length) +
                            Utility::pow<2>(libMesh::pi * (pin_diameter + wire_diameter))));

    // distance from pin surface to duct
    const Real dpgap = _tri_sch_mesh->getDuctToPinGap();

    // Edge pitch parameter defined as pin diameter plus distance to duct wall
    const Real w = pin_diameter + dpgap;

    const Real Ar2 = libMesh::pi * (pin_diameter + wire_diameter) * wire_diameter / 4.0;

    const Real A2prime =
        pitch * (w - pin_diameter / 2.0) - libMesh::pi * Utility::pow<2>(pin_diameter) / 8.0;

    // 1986 sweep coefficients
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

    Real Cs = 0.0;

    if (_mixing_model == "1986")
    {
      // Preserve the original Chen-Todreas (1986) path.
      if (bulk_Re < ReL)
        Cs = CsL;
      else if (bulk_Re > ReT)
        Cs = CsT;
      else
      {
        const Real psi = std::log(bulk_Re / ReL) / std::log(ReT / ReL);
        const Real gamma = 2.0 / 3.0;
        Cs = CsL + (CsT - CsL) * std::pow(psi, gamma);
      }
    }
    else
    {
      // Pacio-only flow-split calculation.
      const Real rho_i_in = _rho_soln(node_in_i);
      const Real rho_j_in = _rho_soln(node_in_j);
      const Real rho_i_out = _rho_soln(node_out_i);
      const Real rho_j_out = _rho_soln(node_out_j);
      const Real rho_i = 0.5 * (rho_i_in + rho_i_out);
      const Real rho_j = 0.5 * (rho_j_in + rho_j_out);

      const Real Vi = 0.5 * (_mdot_soln(node_in_i) + _mdot_soln(node_out_i)) / (rho_i * Si);
      const Real Vj = 0.5 * (_mdot_soln(node_in_j) + _mdot_soln(node_out_j)) / (rho_j * Sj);

      const Real bulk_V = _scm_problem.getBulkVelocity();
      if (MooseUtils::absoluteFuzzyEqual(bulk_V, 0.0))
        return 0.0;

      const Real Xi = Vi / bulk_V;
      const Real Xj = Vj / bulk_V;

      constexpr Real flow_split_exponent = 2.0 - 0.18;
      Real fraction;
      if (MooseUtils::absoluteFuzzyEqual(Xi, Xj))
      {
        const Real Xavg = 0.5 * (Xi + Xj);
        if (Xavg < 0.0)
          mooseError("The Pacio mixing correlation does not support negative flow splits "
                     "when evaluating the fractional flow-split exponent.");
        fraction = flow_split_exponent * std::pow(Xavg, flow_split_exponent - 1.0);
      }
      else
      {
        if (Xi < 0.0 || Xj < 0.0)
          mooseError("The Pacio mixing correlation does not support negative flow splits "
                     "when evaluating the fractional flow-split exponent.");
        fraction =
            (std::pow(Xi, flow_split_exponent) - std::pow(Xj, flow_split_exponent)) / (Xi - Xj);
      }

      const Real WsL = 0.0;
      const Real WsT = 8.8 * fraction / std::pow(std::max(bulk_Re, 1.0), 0.18);

      if (bulk_Re < ReL)
        Cs = WsL;
      else if (bulk_Re > ReT)
        Cs = WsT;
      else
      {
        const Real psi = std::log(bulk_Re / ReL) / std::log(ReT / ReL);
        const Real gamma = 2.0 / 3.0;
        Cs = WsL + (WsT - WsL) * std::pow(psi, gamma);
      }
    }

    // Sweep-flow coefficient used only by the peripheral enthalpy calculation.
    beta = Cs * std::sqrt(Ar2 / A2prime) * std::tan(theta);
  }

  return beta;
}
