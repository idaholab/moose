//* This file is part of the MOOSE framework
//* https://mooseframework.inl.gov
//*
//* All rights reserved, see COPYRIGHT for full restrictions
//* https://github.com/idaholab/moose/blob/master/COPYRIGHT
//*
//* Licensed under LGPL 2.1, please see LICENSE for details
//* https://www.gnu.org/licenses/lgpl-2.1.html

#include "SCMFrictionUpdatedChengTodreas.h"

registerMooseObject("SubChannelApp", SCMFrictionUpdatedChengTodreas);

InputParameters
SCMFrictionUpdatedChengTodreas::validParams()
{
  InputParameters params = SCMFrictionClosureBase::validParams();
  params.addClassDescription("Class that computes the axial friction factor using the updated "
                             "Cheng Todreas correlations.");
  return params;
}

SCMFrictionUpdatedChengTodreas::SCMFrictionUpdatedChengTodreas(const InputParameters & parameters)
  : SCMFrictionClosureBase(parameters),
    _is_tri_lattice(dynamic_cast<const TriSubChannelMesh *>(&_subchannel_mesh) != nullptr),
    _tri_sch_mesh(dynamic_cast<const TriSubChannelMesh *>(&_subchannel_mesh)),
    _quad_sch_mesh(dynamic_cast<const QuadSubChannelMesh *>(&_subchannel_mesh))
{
}

Real
SCMFrictionUpdatedChengTodreas::computeFrictionFactor(const FrictionStruct & friction_args) const
{
  if (_is_tri_lattice)
    return computeTriLatticeFrictionFactor(friction_args);
  else
    return computeQuadLatticeFrictionFactor(friction_args);
}

Real
SCMFrictionUpdatedChengTodreas::computeTriLatticeFrictionFactor(
    const FrictionStruct & friction_args) const
{
  const auto Re = friction_args.Re;
  const auto i_ch = friction_args.i_ch;
  const auto S = friction_args.S;
  const auto w_perim = friction_args.w_perim;
  const auto Dh_i = 4.0 * S / w_perim;
  Real aL, b1L, b2L, cL;
  Real aT, b1T, b2T, cT;
  const Real & pitch = _subchannel_mesh.getPitch();
  const Real & pin_diameter = _subchannel_mesh.getPinDiameter();
  const Real & wire_lead_length = _tri_sch_mesh->getWireLeadLength();
  const Real & wire_diameter = _tri_sch_mesh->getWireDiameter();
  const auto p_over_d = pitch / pin_diameter;
  const auto subch_type = _subchannel_mesh.getSubchannelType(i_ch);
  // This gap is a constant value for the whole assembly. Might want to make it
  // subchannel specific in the future if we have duct deformation.
  const auto gap = _tri_sch_mesh->getDuctToPinGap();
  const auto w_over_d = (pin_diameter + gap) / pin_diameter;
  const auto ReL = std::pow(10, (p_over_d - 1)) * 320.0;
  const auto ReT = std::pow(10, 0.7 * (p_over_d - 1)) * 1.0E+4;
  const auto psi = std::log(Re / ReL) / std::log(ReT / ReL);
  const auto theta = std::acos(
      wire_lead_length / std::sqrt(Utility::pow<2>(wire_lead_length) +
                                   Utility::pow<2>(libMesh::pi * (pin_diameter + wire_diameter))));
  const auto wd_t = (19.56 - 98.71 * (wire_diameter / pin_diameter) +
                     303.47 * Utility::pow<2>((wire_diameter / pin_diameter))) *
                    std::pow((wire_lead_length / pin_diameter), -0.541);
  const auto wd_l = 1.4 * wd_t;
  const auto ws_t = -11.0 * std::log(wire_lead_length / pin_diameter) + 19.0;
  const auto ws_l = ws_t;
  Real pw_p = 0.0;
  Real ar = 0.0;
  Real a_p = 0.0;

  // Find the coefficients of bare Pin bundle friction factor
  // correlations for turbulent and laminar flow regimes. Todreas & Kazimi, Nuclear Systems
  // second edition, Volume 1, Chapter 9.6
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
    // laminar flow friction factor for bare Pin bundle - Center subchannel
    cL = aL + b1L * (p_over_d - 1) + b2L * Utility::pow<2>((p_over_d - 1));
    // turbulent flow friction factor for bare Pin bundle - Center subchannel
    cT = aT + b1T * (p_over_d - 1) + b2T * Utility::pow<2>((p_over_d - 1));
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
    // laminar flow friction factor for bare Pin bundle - Edge subchannel
    cL = aL + b1L * (w_over_d - 1) + b2L * Utility::pow<2>((w_over_d - 1));
    // turbulent flow friction factor for bare Pin bundle - Edge subchannel
    cT = aT + b1T * (w_over_d - 1) + b2T * Utility::pow<2>((w_over_d - 1));
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
      b2T = -0.009567;
    }
    // laminar flow friction factor for bare Pin bundle - Corner subchannel
    cL = aL + b1L * (w_over_d - 1) + b2L * Utility::pow<2>((w_over_d - 1));
    // turbulent flow friction factor for bare Pin bundle - Corner subchannel
    cT = aT + b1T * (w_over_d - 1) + b2T * Utility::pow<2>((w_over_d - 1));
  }

  // Find the coefficients of wire-wrapped Pin bundle friction factor
  // correlations for turbulent and laminar flow regimes. Todreas & Kazimi, Nuclear Systems
  // Volume 1 Chapter 9-6 also Chen and Todreas (2018).
  if ((wire_diameter != 0.0) && (wire_lead_length != 0.0))
  {
    if (subch_type == EChannelType::CENTER)
    {
      // wetted perimeter for center subchannel and bare Pin bundle
      pw_p = libMesh::pi * pin_diameter / 2.0;
      // wire projected area - center subchannel wire-wrapped bundle
      ar = libMesh::pi * (pin_diameter + wire_diameter) * wire_diameter / 6.0;
      // bare Pin bundle center subchannel flow area (normal area + wire area)
      a_p = S + libMesh::pi * Utility::pow<2>(wire_diameter) / 8.0 / std::cos(theta);
      // turbulent friction factor equation constant - Center subchannel
      cT *= (pw_p / w_perim);
      cT += wd_t * (3.0 * ar / a_p) * (Dh_i / wire_lead_length) *
            std::pow((Dh_i / wire_diameter), 0.18);
      // laminar friction factor equation constant - Center subchannel
      cL *= (pw_p / w_perim);
      cL += wd_l * (3.0 * ar / a_p) * (Dh_i / wire_lead_length) * (Dh_i / wire_diameter);
    }
    else if (subch_type == EChannelType::EDGE)
    {
      // wire projected area - edge subchannel wire-wrapped bundle
      ar = libMesh::pi * (pin_diameter + wire_diameter) * wire_diameter / 4.0;
      // bare Pin bundle edge subchannel flow area (normal area + wire area)
      a_p = S + libMesh::pi * Utility::pow<2>(wire_diameter) / 8.0 / std::cos(theta);
      // turbulent friction factor equation constant - Edge subchannel
      cT *= std::pow((1 + ws_t * (ar / a_p) * Utility::pow<2>(std::tan(theta))), 1.41);
      // laminar friction factor equation constant - Edge subchannel
      cL *= (1 + ws_l * (ar / a_p) * Utility::pow<2>(std::tan(theta)));
    }
    else
    {
      // wire projected area - corner subchannel wire-wrapped bundle
      ar = libMesh::pi * (pin_diameter + wire_diameter) * wire_diameter / 6.0;
      // bare Pin bundle corner subchannel flow area (normal area + wire area)
      a_p = S + libMesh::pi * Utility::pow<2>(wire_diameter) / 24.0 / std::cos(theta);
      // turbulent friction factor equation constant - Corner subchannel
      cT *= std::pow((1 + ws_t * (ar / a_p) * Utility::pow<2>(std::tan(theta))), 1.41);
      // laminar friction factor equation constant - Corner subchannel
      cL *= (1 + ws_l * (ar / a_p) * Utility::pow<2>(std::tan(theta)));
    }
  }
  // laminar friction factor and turbulent friction factor coefficients
  const Real bL = -1.0;
  const Real bT = -0.18;
  auto fL = cL * std::pow(Re, bL);
  auto fT = cT * std::pow(Re, bT);

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
    return fL * std::pow((1 - psi), 1.0 / 3.0) * (1 - std::pow(psi, 7)) +
           fT * std::pow(psi, 1.0 / 3.0);
  }
}

Real
SCMFrictionUpdatedChengTodreas::computeQuadLatticeFrictionFactor(
    const FrictionStruct & friction_args) const
{
  const auto Re = friction_args.Re;
  const auto i_ch = friction_args.i_ch;
  /// Todreas-Kazimi NUCLEAR SYSTEMS, second edition, Volume 1, 2011
  Real aL, b1L, b2L, cL;
  Real aT, b1T, b2T, cT;
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
  const auto psi = std::log(Re / ReL) / std::log(ReT / ReL);
  const auto subch_type = _subchannel_mesh.getSubchannelType(i_ch);

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
    cL = aL + b1L * (p_over_d - 1) + b2L * Utility::pow<2>((p_over_d - 1));
    // turbulent flow friction factor for bare Pin bundle - Center subchannel
    cT = aT + b1T * (p_over_d - 1) + b2T * Utility::pow<2>((p_over_d - 1));
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
    cL = aL + b1L * (w_over_d - 1) + b2L * Utility::pow<2>((w_over_d - 1));
    // turbulent flow friction factor for bare Pin bundle - Edge subchannel
    cT = aT + b1T * (w_over_d - 1) + b2T * Utility::pow<2>((w_over_d - 1));
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
    cL = aL + b1L * (w_over_d - 1) + b2L * Utility::pow<2>((w_over_d - 1));
    // turbulent flow friction factor for bare Pin bundle - Corner subchannel
    cT = aT + b1T * (w_over_d - 1) + b2T * Utility::pow<2>((w_over_d - 1));
  }
  // laminar friction factor and turbulent friction factor coefficients
  const Real bL = -1.0;
  const Real bT = -0.18;
  auto fL = cL * std::pow(Re, bL);
  auto fT = cT * std::pow(Re, bT);

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
    return fL * std::pow((1 - psi), 1.0 / 3.0) * (1 - std::pow(psi, 7)) +
           fT * std::pow(psi, 1.0 / 3.0);
  }
}
