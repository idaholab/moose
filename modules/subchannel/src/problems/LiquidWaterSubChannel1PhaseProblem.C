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
  params.addParam<bool>("default_friction_model",
                        true,
                        "Boolean to define which friction model to use (Only for quad use)");
  return params;
}

LiquidWaterSubChannel1PhaseProblem::LiquidWaterSubChannel1PhaseProblem(
    const InputParameters & params)
  : SubChannel1PhaseProblem(params),
    _subchannel_mesh(dynamic_cast<QuadSubChannelMesh &>(_mesh)),
    _default_friction_model(getParam<bool>("default_friction_model"))
{
}

double
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
