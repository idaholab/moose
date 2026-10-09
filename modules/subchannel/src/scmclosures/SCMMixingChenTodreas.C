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
registerMooseObjectRenamed("SubChannelApp",
                           SCMMixingChengTodreas,
                           "09/30/2027 24:00",
                           SCMMixingChenTodreas);

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
    _rho_soln(_subproblem.getVariable(0, "rho")),
    // Not a number until computeBulkMixingParameters, so that the problem rejects any use before
    _beta_1986(std::numeric_limits<Real>::quiet_NaN()),
    _beta_sweep(std::numeric_limits<Real>::quiet_NaN())
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

  // Cheng and Todreas (1986), Table 3, reports the mixing parameter data range as
  // 1.07 <= P/D <= 1.30, 4 <= H/D <= 52, and 400 <= Reb <= 1.0e6. Their Tables 1 and 2 list
  // the mixing experiments, which span 7 <= Npin <= 217.
  // Pacio et al. (2022), Table 1, reports the PCTD range of validity as
  // 19 <= Npin <= 217, 1.02 <= P/D <= 1.42, 7.5 <= H/D <= 54, and 10 <= Reb <= 3.0e5.
  if (_mixing_model == "1986")
  {
    if (p_over_d < 1.07 || p_over_d > 1.30)
      flagSolutionWarning("Pitch-over-pin diameter ratio (P/D) outside the 1986 "
                          "Cheng-Todreas mixing correlation data range.");
    if (wire_lead_to_diameter < 4.0 || wire_lead_to_diameter > 52.0)
      flagSolutionWarning("Wire lead length-over-pin diameter ratio (H/D) outside the 1986 "
                          "Cheng-Todreas mixing correlation data range.");
    if (num_pins < 7 || num_pins > 217)
      flagSolutionWarning("Number of pins outside the 1986 Cheng-Todreas mixing correlation "
                          "data range.");
  }
  else
  {
    if (p_over_d < 1.02 || p_over_d > 1.42)
      flagSolutionWarning("Pitch-over-pin diameter ratio (P/D) outside the "
                          "Pacio-Chen-Todreas mixing correlation data range.");
    if (wire_lead_to_diameter < 7.5 || wire_lead_to_diameter > 54.0)
      flagSolutionWarning("Wire lead length-over-pin diameter ratio (H/D) outside the "
                          "Pacio-Chen-Todreas mixing correlation data range.");
    if (num_pins < 19 || num_pins > 217)
      flagSolutionWarning("Number of pins outside the Pacio-Chen-Todreas mixing correlation "
                          "data range.");
  }
}

void
SCMMixingChenTodreas::computeBulkMixingParameters() const
{
  const Real pitch = _subchannel_mesh.getPitch();
  const Real pin_diameter = _subchannel_mesh.getPinDiameter();
  const Real p_over_d = pitch / pin_diameter;

  const Real wire_lead_length = _tri_sch_mesh->getWireLeadLength();
  const Real wire_diameter = _tri_sch_mesh->getWireDiameter();
  const unsigned int Nr = _tri_sch_mesh->getNumOfRings();

  const Real bulk_Re = _scm_problem.getBulkReynoldsNumber();

  // The bulk Reynolds number is computed from the solution during the solve, so its
  // applicability range (see constructor) can only be checked here rather than at construction
  if (_mixing_model == "1986")
  {
    if (bulk_Re < 400.0 || bulk_Re > 1.0e6)
      flagSolutionWarning("Bulk Reynolds number (Reb) outside the 1986 Cheng-Todreas mixing "
                          "correlation data range.");
  }
  else if (bulk_Re < 10.0 || bulk_Re > 3.0e5)
    flagSolutionWarning("Bulk Reynolds number (Reb) outside the Pacio-Chen-Todreas mixing "
                        "correlation data range.");

  const Real theta = std::acos(
      wire_lead_length / std::sqrt(Utility::pow<2>(wire_lead_length) +
                                   Utility::pow<2>(libMesh::pi * (pin_diameter + wire_diameter))));

  const Real ReL = 320.0 * std::pow(10.0, p_over_d - 1.0);

  const Real ReT = 10000.0 * std::pow(10.0, 0.7 * (p_over_d - 1.0));

  //
  // Original Cheng-Todreas (1986) mixing correlation.
  //
  // This remains active for center-center gaps even when Pacio is selected,
  // while center-edge gaps and edge-corner gaps are replaced by the Pacio treatment.
  // When Pacio is not selected: beta is defined for center-center, center-edge gaps.
  // In edge-edge, edge-corner beta = zero.
  {
    // projected area of wire on center subchannel
    const Real Ar1 = libMesh::pi * (pin_diameter + wire_diameter) * wire_diameter / 6.0;

    // bare center-subchannel flow area
    const Real A1prime = (std::sqrt(3.0) / 4.0) * Utility::pow<2>(pitch) -
                         libMesh::pi * Utility::pow<2>(pin_diameter) / 8.0;

    Real CmL_constant;
    Real CmT_constant;

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

    Real Cm;

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

    _beta_1986 = Cm * std::sqrt(Ar1 / A1prime) * std::tan(theta);
  }

  // Sweep flow always uses the original Cheng-Todreas (1986) correlation.
  {
    // distance from pin surface to duct
    const Real dpgap = _tri_sch_mesh->getDuctToPinGap();

    // Edge pitch parameter defined as pin diameter plus distance to duct wall
    const Real w = pin_diameter + dpgap;

    const Real Ar2 = libMesh::pi * (pin_diameter + wire_diameter) * wire_diameter / 4.0;

    const Real A2prime =
        pitch * (w - pin_diameter / 2.0) - libMesh::pi * Utility::pow<2>(pin_diameter) / 8.0;

    Real CsL_constant;
    Real CsT_constant;

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

    Real Cs;
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

    // Sweep-flow coefficient used only by the peripheral enthalpy calculation.
    _beta_sweep = Cs * std::sqrt(Ar2 / A2prime) * std::tan(theta);
  }
}

void
SCMMixingChenTodreas::computeBlockMixingParameters(const unsigned int first_node,
                                                   const unsigned int last_node) const
{
  if (_mixing_model != "Pacio")
    return;

  if (_beta_pacio_center_edge.size() < last_node + 1)
  {
    _beta_pacio_center_edge.resize(last_node + 1);
    _beta_pacio_edge_corner.resize(last_node + 1);
  }

  const Real pitch = _subchannel_mesh.getPitch();
  const Real pin_diameter = _subchannel_mesh.getPinDiameter();
  const Real wire_diameter = _tri_sch_mesh->getWireDiameter();
  const Real dpgap = _tri_sch_mesh->getDuctToPinGap();
  const Real bulk_V = _scm_problem.getBulkVelocity();

  // Pacio et al. (2022) Eq. (30) multiplies beta by the contact perimeter between all subchannels
  // of two types, Eqs. (A.24) and (A.25), while SCM multiplies beta by the width of each gap.
  // Scale beta by the ratio of the contact perimeter per gap to the gap width, so that the sum
  // over the gaps of SCM gives the crossflow of Pacio:
  // - center-edge: Pi12 = 6 n (P - D - Dw / 6) over the 6 n pin-pin gaps of width P - D;
  // - edge-corner: Pi23 = [6 (W - D) 2 - Dw / 6] / 2 over the 12 pin-duct gaps of width W - D.
  const Real perimeter_ratio_center_edge =
      (pitch - pin_diameter - wire_diameter / 6.0) / (pitch - pin_diameter);
  const Real perimeter_ratio_edge_corner = (0.5 * dpgap - wire_diameter / 144.0) / dpgap;

  for (unsigned int iz = first_node; iz < last_node + 1; iz++)
  {
    // Flow split of each subchannel type lumped over the axial cell, as in the PCTD model,
    // where the flow split is defined per subchannel type: sum of the mass flow rates over sum of
    // the density times flow area, averaged over the inlet and outlet of the cell
    Real mdot_sum[3] = {0.0, 0.0, 0.0};
    Real rhoS_sum[3] = {0.0, 0.0, 0.0};
    for (const auto i_ch : make_range(_subchannel_mesh.getNumOfChannels()))
    {
      const Node * const node_in = _subchannel_mesh.getChannelNode(i_ch, iz - 1);
      const Node * const node_out = _subchannel_mesh.getChannelNode(i_ch, iz);
      const auto type = _subchannel_mesh.getSubchannelType(i_ch);
      const unsigned int i_type =
          type == EChannelType::CENTER ? 0 : (type == EChannelType::EDGE ? 1 : 2);
      mdot_sum[i_type] += 0.5 * (_mdot_soln(node_in) + _mdot_soln(node_out));
      rhoS_sum[i_type] += 0.25 * (_rho_soln(node_in) + _rho_soln(node_out)) *
                          (_S_soln(node_in) + _S_soln(node_out));
    }
    const Real X_center = mdot_sum[0] / rhoS_sum[0] / bulk_V;
    const Real X_edge = mdot_sum[1] / rhoS_sum[1] / bulk_V;
    const Real X_corner = mdot_sum[2] / rhoS_sum[2] / bulk_V;

    _beta_pacio_center_edge[iz] =
        computePacioMixingParameter(X_center, X_edge, perimeter_ratio_center_edge);
    _beta_pacio_edge_corner[iz] =
        computePacioMixingParameter(X_edge, X_corner, perimeter_ratio_edge_corner);
  }
}

Real
SCMMixingChenTodreas::computePacioMixingParameter(const Real Xi,
                                                  const Real Xj,
                                                  const Real perimeter_ratio) const
{
  mooseAssert(_mixing_model == "Pacio", "The Pacio mixing parameter requires mixing_model = Pacio");

  const Real pitch = _subchannel_mesh.getPitch();
  const Real pin_diameter = _subchannel_mesh.getPinDiameter();
  const Real wire_lead_length = _tri_sch_mesh->getWireLeadLength();
  const Real wire_diameter = _tri_sch_mesh->getWireDiameter();
  const Real bulk_Re = _scm_problem.getBulkReynoldsNumber();

  const Real theta = std::acos(
      wire_lead_length / std::sqrt(Utility::pow<2>(wire_lead_length) +
                                   Utility::pow<2>(libMesh::pi * (pin_diameter + wire_diameter))));

  const Real ReL = 700.0;
  const Real ReT = 10000.0;

  constexpr Real flow_split_exponent = 2.0 - 0.18;

  // Flow-split term of the mixing parameter, Pacio et al. (2022) Eq. (31).
  Real fraction;
  if (MooseUtils::absoluteFuzzyEqual(Xi, Xj))
  {
    const Real Xavg = 0.5 * (Xi + Xj);

    if (Xavg < 0.0)
      mooseError("The Pacio mixing correlation does not support negative flow splits "
                 "when evaluating the fractional flow-split exponent.");

    // The flow-split term diverges as X^(-m) at zero flow; the crossflow is zero there because
    // SCM multiplies beta by the average mass flux of the gap, so any finite value works
    if (MooseUtils::absoluteFuzzyEqual(Xavg, 0.0))
      fraction = 0.0;
    else
      fraction = 0.5 * flow_split_exponent * std::pow(Xavg, flow_split_exponent - 2.0);
  }
  else
  {
    if (Xi < 0.0 || Xj < 0.0)
      mooseError("The Pacio mixing correlation does not support negative flow splits "
                 "when evaluating the fractional flow-split exponent.");

    fraction = (std::pow(Xi, flow_split_exponent) - std::pow(Xj, flow_split_exponent)) /
               (Utility::pow<2>(Xi) - Utility::pow<2>(Xj));
  }

  // Laminar and turbulent wire mixing coefficients, Pacio et al. (2022) Table 5
  const Real WmL = 0.0;
  const Real WmT = 8.8;

  // Mixing parameter without the geometric factors, Pacio et al. (2022) Eqs. (31) and (33):
  // WmT fraction / Reb^m in the turbulent regime and WmL / Reb in the laminar regime
  const Real Re_eff = std::max(bulk_Re, 1.0);
  const Real Cm_L = WmL / Re_eff;
  const Real Cm_T = WmT * fraction / std::pow(Re_eff, 0.18);

  Real Cm;

  if (bulk_Re < ReL)
    Cm = Cm_L;
  else if (bulk_Re > ReT)
    Cm = Cm_T;
  else
  {
    const Real psi = std::log(bulk_Re / ReL) / std::log(ReT / ReL);

    // Same transition interpolation as the Pacio-Chen-Todreas friction factor, Pacio et al. (2022)
    // Eq. (35), with gamma and lambda of Table 5
    const Real gamma = 0.362;
    const Real lambda = 6.7;

    Cm = Cm_L * std::pow(1.0 - psi, gamma) * (1.0 - std::pow(psi, lambda)) +
         Cm_T * std::pow(psi, gamma);
  }

  // Pacio uses the edge-subchannel projected wire area and bare flow area.
  const Real dpgap = _tri_sch_mesh->getDuctToPinGap();
  const Real w = pin_diameter + dpgap;

  const Real Ar2 = libMesh::pi * (pin_diameter + wire_diameter) * wire_diameter / 4.0;

  const Real A2prime =
      pitch * (w - pin_diameter / 2.0) - libMesh::pi * Utility::pow<2>(pin_diameter) / 8.0;

  return Cm * std::sqrt(Ar2 / A2prime) * std::tan(theta) * perimeter_ratio;
}

Real
SCMMixingChenTodreas::computeMixingParameter(const unsigned int i_gap, const unsigned int iz) const
{
  const auto chans = _subchannel_mesh.getGapChannels(i_gap);
  const auto subch_type_i = _subchannel_mesh.getSubchannelType(chans.first);
  const auto subch_type_j = _subchannel_mesh.getSubchannelType(chans.second);

  // Pacio applicability
  const bool center_edge =
      (subch_type_i == EChannelType::CENTER && subch_type_j == EChannelType::EDGE) ||
      (subch_type_i == EChannelType::EDGE && subch_type_j == EChannelType::CENTER);

  const bool edge_corner =
      (subch_type_i == EChannelType::EDGE && subch_type_j == EChannelType::CORNER) ||
      (subch_type_i == EChannelType::CORNER && subch_type_j == EChannelType::EDGE);

  // Pacio defines the mixing treatment across center-edge and edge-corner gaps.
  // For all other applicable gaps, we retain the original Cheng-Todreas (1986) model.
  if (_mixing_model == "Pacio" && center_edge)
    return _beta_pacio_center_edge[iz];
  else if (_mixing_model == "Pacio" && edge_corner)
    return _beta_pacio_edge_corner[iz];
  else if (subch_type_i == EChannelType::CENTER || subch_type_j == EChannelType::CENTER)
    return _beta_1986;
  else
    return 0.0;
}

Real
SCMMixingChenTodreas::computeSweepFlowMixingParameter(const unsigned int i_gap,
                                                      const unsigned int /* iz */) const
{
  const auto chans = _subchannel_mesh.getGapChannels(i_gap);
  const auto subch_type_i = _subchannel_mesh.getSubchannelType(chans.first);
  const auto subch_type_j = _subchannel_mesh.getSubchannelType(chans.second);

  if ((subch_type_i == EChannelType::CORNER || subch_type_i == EChannelType::EDGE) &&
      (subch_type_j == EChannelType::CORNER || subch_type_j == EChannelType::EDGE))
    return _beta_sweep;
  else
    return 0.0;
}
