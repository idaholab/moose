//* This file is part of the MOOSE framework
//* https://mooseframework.inl.gov
//*
//* All rights reserved, see COPYRIGHT for full restrictions
//* https://github.com/idaholab/moose/blob/master/COPYRIGHT
//*
//* Licensed under LGPL 2.1, please see LICENSE for details
//* https://www.gnu.org/licenses/lgpl-2.1.html

#include "SCMFrictionUpgradedChengTodreas.h"

registerMooseObject("SubChannelApp", SCMFrictionUpgradedChengTodreas);

InputParameters
SCMFrictionUpgradedChengTodreas::validParams()
{
  InputParameters params = SCMFrictionClosureBase::validParams();
  params.addClassDescription("Class that computes the axial friction factor using the upgraded "
                             "Cheng Todreas correlations.");
  return params;
}

SCMFrictionUpgradedChengTodreas::SCMFrictionUpgradedChengTodreas(const InputParameters & parameters)
  : SCMFrictionClosureBase(parameters),
    _is_tri_lattice(dynamic_cast<const TriSubChannelMesh *>(&_subchannel_mesh) != nullptr),
    _tri_sch_mesh(dynamic_cast<const TriSubChannelMesh *>(&_subchannel_mesh)),
    _quad_sch_mesh(dynamic_cast<const QuadSubChannelMesh *>(&_subchannel_mesh)),
    _has_wire_wrap(_is_tri_lattice && _tri_sch_mesh->getWireDiameter() != 0.0 &&
                   _tri_sch_mesh->getWireLeadLength() != 0.0)
{
  if (_is_tri_lattice &&
      ((_tri_sch_mesh->getWireDiameter() != 0.0) != (_tri_sch_mesh->getWireLeadLength() != 0.0)))
    mooseError("Wire-wrapped bundle friction requires both wire diameter and wire lead length. "
               "Set both to zero for a bare pin bundle.");

  if (_is_tri_lattice)
  {
    const auto pitch = _subchannel_mesh.getPitch();
    const auto pin_diameter = _subchannel_mesh.getPinDiameter();
    const auto p_over_d = pitch / pin_diameter;
    const auto wire_lead_to_diameter = _tri_sch_mesh->getWireLeadLength() / pin_diameter;
    const unsigned int Nr = _tri_sch_mesh->getNumOfRings();
    const unsigned int num_pins = 1 + 3 * Nr * (Nr - 1);
    const auto Reb = _scm_problem.getBulkReynoldsNumber();

    // The upgraded Cheng-Todreas detailed triangular friction factor correlation is based on
    // data spanning 1.0 <= P/D <= 1.42, 4 <= H/D <= 52, 7 <= Npin <= 217,
    // and 50 <= Re <= 1e6.
    if (p_over_d < 1.0 || p_over_d > 1.42)
      flagSolutionWarning("Pitch-over-pin diameter ratio (P/D) outside the upgraded "
                          "Cheng-Todreas friction correlation data range.");
    if (_has_wire_wrap && (wire_lead_to_diameter < 8.0 || wire_lead_to_diameter > 52.0))
      flagSolutionWarning("Wire lead length-over-pin diameter ratio (H/D) outside the upgraded "
                          "Cheng-Todreas friction correlation data range.");
    if (num_pins < 7 || num_pins > 271)
      flagSolutionWarning("Number of pins outside the upgraded Cheng-Todreas friction correlation "
                          "data range.");
    if (Reb < 50.0 || Reb > 1.0e6)
      flagSolutionWarning("Bulk Reynolds number (Reb) outside the upgraded Cheng-Todreas friction "
                          "correlation data range.");
  }
}

Real
SCMFrictionUpgradedChengTodreas::computeFrictionFactor(const FrictionStruct & friction_args) const
{
  if (_is_tri_lattice)
    return computeTriLatticeFrictionFactor(friction_args);
  else
    return computeQuadLatticeFrictionFactor(friction_args);
}

Real
SCMFrictionUpgradedChengTodreas::computeTriLatticeFrictionFactor(
    const FrictionStruct & friction_args) const
{
  const auto Re = friction_args.Re;
  // Limit the Reynolds number used in the friction-factor correlation to avoid
  // singular behavior at zero flow.
  const Real Re_eff = std::max(Re, 1.0);
  const auto i_ch = friction_args.i_ch;
  const auto S = friction_args.S;
  const auto w_perim = friction_args.w_perim;
  const auto Dh_i = 4.0 * S / w_perim;
  // Bare fuel Pins coefficients, and friction factor
  Real k1L, k2L, k3L, CfL;
  Real k1T, k2T, k3T, CfT;
  // Transient range parameters
  Real CbL1, CbL2, CbT1, CbT2;
  // wire sweep coefficient parameters
  Real a, b;
  // wire Drag coefficient parameters
  Real CwT1, CwT2, CwT3, CwT4;
  // Ratio of laminar over turbulent drag and sweep coefficients
  Real CwL1, CwL2;
  // interpolation exponent
  Real lambda;
  // transition smoothing coefficient
  Real gamma;
  const Real & pitch = _subchannel_mesh.getPitch();
  const Real & pin_diameter = _subchannel_mesh.getPinDiameter();
  const Real & wire_lead_length = _tri_sch_mesh->getWireLeadLength();
  const Real & wire_diameter = _tri_sch_mesh->getWireDiameter();
  const auto gap = _tri_sch_mesh->getDuctToPinGap();
  const auto p_over_d = pitch / pin_diameter;
  const auto w_over_d = (pin_diameter + gap) / pin_diameter;
  const auto subch_type = _subchannel_mesh.getSubchannelType(i_ch);
  CbL1 = 320;
  CbL2 = 1.0;
  CbT1 = 10000;
  CbT2 = 0.7;
  const auto ReL = std::pow(10, CbL2 * (p_over_d - 1)) * CbL1;
  const auto ReT = std::pow(10, CbT2 * (p_over_d - 1)) * CbT1;
  const auto Reb = _scm_problem.getBulkReynoldsNumber();
  const auto psi = std::log(Reb / ReL) / std::log(ReT / ReL);

  // Find the coefficients of bare Pin bundle friction factor
  // correlations for turbulent and laminar flow regimes. Todreas & Kazimi, Nuclear Systems
  // second edition, Volume 1, Chapter 9.6
  if (subch_type == EChannelType::CENTER)
  {
    if (p_over_d < 1.1)
    {
      k1L = 26.0;
      k2L = 888.2;
      k3L = -3334.0;
      k1T = 0.09378;
      k2T = 1.398;
      k3T = -8.664;
    }
    else
    {
      k1L = 62.97;
      k2L = 216.9;
      k3L = -190.2;
      k1T = 0.1458;
      k2T = 0.03632;
      k3T = -0.03333;
    }
    // laminar flow friction factor for bare Pin bundle - Center subchannel
    CfL = k1L + k2L * (p_over_d - 1) + k3L * Utility::pow<2>((p_over_d - 1));
    // turbulent flow friction factor for bare Pin bundle - Center subchannel
    CfT = k1T + k2T * (p_over_d - 1) + k3T * Utility::pow<2>((p_over_d - 1));
  }
  else if (subch_type == EChannelType::EDGE)
  {
    if (w_over_d < 1.1)
    {
      k1L = 26.18;
      k2L = 554.5;
      k3L = -1480.0;
      k1T = 0.09377;
      k2T = 0.8732;
      k3T = -3.341;
    }
    else
    {
      k1L = 44.4;
      k2L = 256.7;
      k3L = -267.6;
      k1T = 0.1430;
      k2T = 0.04199;
      k3T = -0.04428;
    }
    // laminar flow friction factor for bare Pin bundle - Edge subchannel
    CfL = k1L + k2L * (w_over_d - 1) + k3L * Utility::pow<2>((w_over_d - 1));
    // turbulent flow friction factor for bare Pin bundle - Edge subchannel
    CfT = k1T + k2T * (w_over_d - 1) + k3T * Utility::pow<2>((w_over_d - 1));
  }
  else
  {
    if (w_over_d < 1.1)
    {
      k1L = 26.98;
      k2L = 1636.0;
      k3L = -10050.0;
      k1T = 0.1004;
      k2T = 1.625;
      k3T = -11.85;
    }
    else
    {
      k1L = 87.26;
      k2L = 38.59;
      k3L = -55.12;
      k1T = 0.1499;
      k2T = 0.006706;
      k3T = -0.009567;
    }
    // laminar flow friction factor for bare Pin bundle - Corner subchannel
    CfL = k1L + k2L * (w_over_d - 1) + k3L * Utility::pow<2>((w_over_d - 1));
    // turbulent flow friction factor for bare Pin bundle - Corner subchannel
    CfT = k1T + k2T * (w_over_d - 1) + k3T * Utility::pow<2>((w_over_d - 1));
  }

  // Find the coefficients of wire-wrapped Pin bundle friction factor
  // correlations for turbulent and laminar flow regimes. Todreas & Kazimi, Nuclear Systems
  // Volume 1 Chapter 9-6 also Chen and Todreas (2018).
  if (_has_wire_wrap)
  {
    const auto theta =
        std::acos(wire_lead_length /
                  std::sqrt(Utility::pow<2>(wire_lead_length) +
                            Utility::pow<2>(libMesh::pi * (pin_diameter + wire_diameter))));
    CwT1 = 19.56;
    CwT2 = -98.71;
    CwT3 = 303.47;
    CwT4 = -0.541;
    CwL1 = 1.4;
    CwL2 = 1.0;
    // Drag coefficient
    const auto WdT = (CwT1 + CwT2 * (wire_diameter / pin_diameter) +
                      CwT3 * Utility::pow<2>((wire_diameter / pin_diameter))) *
                     std::pow((wire_lead_length / pin_diameter), CwT4);
    const auto WdL = CwL1 * WdT;
    a = -11;
    b = 19;
    // Sweep coefficient
    const auto WsT = a * std::log10(wire_lead_length / pin_diameter) + b;
    const auto WsL = CwL2 * WsT;
    Real ar = 0.0;
    Real a_p = 0.0;

    if (subch_type == EChannelType::CENTER)
    {
      // wetted perimeter for center subchannel and bare Pin bundle
      const Real pw_p = libMesh::pi * pin_diameter / 2.0;
      // wire projected area - center subchannel wire-wrapped bundle
      ar = libMesh::pi * (pin_diameter + wire_diameter) * wire_diameter / 6.0;
      // bare Pin bundle center subchannel flow area (normal area + wire area)
      a_p = S + libMesh::pi * Utility::pow<2>(wire_diameter) / 8.0 / std::cos(theta);
      // turbulent friction factor equation constant - Center subchannel
      CfT *= (pw_p / w_perim);
      CfT += WdT * (3.0 * ar / a_p) * (Dh_i / wire_lead_length) *
             std::pow((Dh_i / wire_diameter), 0.18);
      // laminar friction factor equation constant - Center subchannel
      CfL *= (pw_p / w_perim);
      CfL += WdL * (3.0 * ar / a_p) * (Dh_i / wire_lead_length) * (Dh_i / wire_diameter);
    }
    else if (subch_type == EChannelType::EDGE)
    {
      // wire projected area - edge subchannel wire-wrapped bundle
      ar = libMesh::pi * (pin_diameter + wire_diameter) * wire_diameter / 4.0;
      // bare Pin bundle edge subchannel flow area (normal area + wire area)
      a_p = S + libMesh::pi * Utility::pow<2>(wire_diameter) / 8.0 / std::cos(theta);
      // turbulent friction factor equation constant - Edge subchannel
      const Real turbulent_wire_correction =
          1 + WsT * (ar / a_p) * Utility::pow<2>(std::tan(theta));
      if (!std::isfinite(turbulent_wire_correction) || turbulent_wire_correction < 0.0)
        mooseError("The exponentiated term in the Cheng-Todreas turbulent wire correction must be "
                   "non-negative and finite for an edge subchannel. Computed ",
                   turbulent_wire_correction,
                   ".");
      CfT *= std::pow(turbulent_wire_correction, 1.41);
      // laminar friction factor equation constant - Edge subchannel
      CfL *= (1 + WsL * (ar / a_p) * Utility::pow<2>(std::tan(theta)));
    }
    else
    {
      // wire projected area - corner subchannel wire-wrapped bundle
      ar = libMesh::pi * (pin_diameter + wire_diameter) * wire_diameter / 6.0;
      // bare Pin bundle corner subchannel flow area (normal area + wire area)
      a_p = S + libMesh::pi * Utility::pow<2>(wire_diameter) / 24.0 / std::cos(theta);
      // turbulent friction factor equation constant - Corner subchannel
      const Real turbulent_wire_correction =
          1 + WsT * (ar / a_p) * Utility::pow<2>(std::tan(theta));
      if (!std::isfinite(turbulent_wire_correction) || turbulent_wire_correction < 0.0)
        mooseError("The exponentiated term in the Cheng-Todreas turbulent wire correction must be "
                   "non-negative and finite for a corner subchannel. Computed ",
                   turbulent_wire_correction,
                   ".");
      CfT *= std::pow(turbulent_wire_correction, 1.41);
      // laminar friction factor equation constant - Corner subchannel
      CfL *= (1 + WsL * (ar / a_p) * Utility::pow<2>(std::tan(theta)));
    }
  }
  // laminar friction factor and turbulent friction factor coefficients
  const Real mL = -1.0;
  const Real mT = -0.18;
  auto fL = CfL * std::pow(Re_eff, mL);
  auto fT = CfT * std::pow(Re_eff, mT);
  gamma = 1.0 / 3.0;
  lambda = 7.0;
  if (Reb < ReL)
  {
    // laminar flow
    return fL;
  }
  else if (Reb > ReT)
  {
    // turbulent flow
    return fT;
  }
  else
  {
    // Transition regime is selected by bulk Reynolds number, same for all channels.
    return fL * std::pow((1 - psi), gamma) * (1 - std::pow(psi, lambda)) +
           fT * std::pow(psi, gamma);
  }
}

Real
SCMFrictionUpgradedChengTodreas::computeQuadLatticeFrictionFactor(
    const FrictionStruct & friction_args) const
{
  const auto Re = friction_args.Re;
  // Limit the Reynolds number used in the friction-factor correlation to avoid
  // singular behavior k1T zero flow.
  const Real Re_eff = std::max(Re, 1.0);
  const auto i_ch = friction_args.i_ch;
  /// Todreas-Kazimi NUCLEAR SYSTEMS, second edition, Volume 1, 2011
  Real k1L, k2L, k3L, CfL;
  Real k1T, k2T, k3T, CfT;
  const auto pitch = _subchannel_mesh.getPitch();
  const auto pin_diameter = _subchannel_mesh.getPinDiameter();
  // This gap is a constant value for the whole assembly. Might want to make it
  // subchannel specific in the future if we have duct deformation.
  const auto side_gap = _quad_sch_mesh->getSideGap();
  const auto w = (pin_diameter / 2.0) + (pitch / 2.0) + side_gap;
  const auto p_over_d = pitch / pin_diameter;
  const auto w_over_d = w / pin_diameter;
  const auto ReL = std::pow(10, (p_over_d - 1)) * 320.0;
  const auto ReT = std::pow(10, 0.7 * (p_over_d - 1)) * 1.0E+4;
  const auto Reb = _scm_problem.getBulkReynoldsNumber();
  const auto psi = std::log(Reb / ReL) / std::log(ReT / ReL);
  const auto subch_type = _subchannel_mesh.getSubchannelType(i_ch);
  // interpolation exponent
  Real lambda;
  // transition smoothing coefficient
  Real gamma;

  // Find the coefficients of bare Pin bundle friction factor
  // correlations for turbulent and laminar flow regimes. Todreas & Kazimi, Nuclear Systems Volume
  // 1
  if (subch_type == EChannelType::CENTER)
  {
    if (p_over_d < 1.1)
    {
      k1L = 26.37;
      k2L = 374.2;
      k3L = -493.9;
      k1T = 0.09423;
      k2T = 0.5806;
      k3T = -1.239;
    }
    else
    {
      k1L = 35.55;
      k2L = 263.7;
      k3L = -190.2;
      k1T = 0.1339;
      k2T = 0.09059;
      k3T = -0.09926;
    }
    // laminar flow friction factor for bare Pin bundle - Center subchannel
    CfL = k1L + k2L * (p_over_d - 1) + k3L * Utility::pow<2>((p_over_d - 1));
    // turbulent flow friction factor for bare Pin bundle - Center subchannel
    CfT = k1T + k2T * (p_over_d - 1) + k3T * Utility::pow<2>((p_over_d - 1));
  }
  else if (subch_type == EChannelType::EDGE)
  {
    if (p_over_d < 1.1)
    {
      k1L = 26.18;
      k2L = 554.5;
      k3L = -1480;
      k1T = 0.09377;
      k2T = 0.8732;
      k3T = -3.341;
    }
    else
    {
      k1L = 44.40;
      k2L = 256.7;
      k3L = -267.6;
      k1T = 0.1430;
      k2T = 0.04199;
      k3T = -0.04428;
    }
    // laminar flow friction factor for bare Pin bundle - Edge subchannel
    CfL = k1L + k2L * (w_over_d - 1) + k3L * Utility::pow<2>((w_over_d - 1));
    // turbulent flow friction factor for bare Pin bundle - Edge subchannel
    CfT = k1T + k2T * (w_over_d - 1) + k3T * Utility::pow<2>((w_over_d - 1));
  }
  else
  {
    if (p_over_d < 1.1)
    {
      k1L = 28.62;
      k2L = 715.9;
      k3L = -2807;
      k1T = 0.09755;
      k2T = 1.127;
      k3T = -6.304;
    }
    else
    {
      k1L = 58.83;
      k2L = 160.7;
      k3L = -203.5;
      k1T = 0.1452;
      k2T = 0.02681;
      k3T = -0.03411;
    }
    // laminar flow friction factor for bare Pin bundle - Corner subchannel
    CfL = k1L + k2L * (w_over_d - 1) + k3L * Utility::pow<2>((w_over_d - 1));
    // turbulent flow friction factor for bare Pin bundle - Corner subchannel
    CfT = k1T + k2T * (w_over_d - 1) + k3T * Utility::pow<2>((w_over_d - 1));
  }
  // laminar friction factor and turbulent friction factor coefficients
  const Real mL = -1.0;
  const Real mT = -0.18;
  auto fL = CfL * std::pow(Re_eff, mL);
  auto fT = CfT * std::pow(Re_eff, mT);
  gamma = 1.0 / 3.0;
  lambda = 7.0;
  if (Reb < ReL)
  {
    // laminar flow
    return fL;
  }
  else if (Reb > ReT)
  {
    // turbulent flow
    return fT;
  }
  else
  {
    // Transition regime is selected by bulk Reynolds number, same for all channels.
    return fL * std::pow((1 - psi), gamma) * (1 - std::pow(psi, lambda)) +
           fT * std::pow(psi, gamma);
  }
}
