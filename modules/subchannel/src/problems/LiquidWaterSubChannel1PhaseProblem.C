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
  return params;
}

LiquidWaterSubChannel1PhaseProblem::LiquidWaterSubChannel1PhaseProblem(
    const InputParameters & params)
  : SubChannel1PhaseProblem(params), _subchannel_mesh(dynamic_cast<QuadSubChannelMesh &>(_mesh))
{
}

double
LiquidWaterSubChannel1PhaseProblem::computeFrictionFactor(double Re)
{
  double a, b;
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

double
LiquidWaterSubChannel1PhaseProblem::computeFrictionFactor(double Re, int i_ch)
{
  double a, b1, b2, C, n, ratio;
  auto pitch = _subchannel_mesh.getPitch();
  auto rod_diameter = _subchannel_mesh.getRodDiameter();
  auto gap = _subchannel_mesh.getGap();
  auto w = (rod_diameter / 2.0) + (pitch / 2.0) + gap;
  auto p_over_d = pitch / rod_diameter;
  auto subch_type = _subchannel_mesh.getSubchannelType(i_ch);

  if (Re < 2000.0)
  {
    n = 1.0;
    if (subch_type == EChannelType::CORNER)
    {
      ratio = w / rod_diameter;
      if ((1.0 <= p_over_d) && (p_over_d <= 1.1))
      {
        a = 28.62, b1 = 715.9, b2 = -2807;
      }
      else if ((1.1 < p_over_d) && (p_over_d <= 1.5))
      {
        a = 58.83, b1 = 160.7, b2 = -203.5;
      }
      else
        mooseError(" Geometric parameters beyond scope of friction factor model ");
    }
    else if (subch_type == EChannelType::EDGE)
    {
      ratio = w / rod_diameter;
      if ((1.0 <= p_over_d) && (p_over_d <= 1.1))
      {
        a = 26.18, b1 = 554.5, b2 = -1480;
      }
      else if ((1.1 < p_over_d) && (p_over_d <= 1.5))
      {
        a = 44.40, b1 = 256.7, b2 = -267.6;
      }
      else
        mooseError(" Geometric parameters beyond scope of friction factor model ");
    }
    else
    {
      ratio = p_over_d;
      if ((1.0 <= p_over_d) && (p_over_d <= 1.1))
      {
        a = 26.37, b1 = 374.2, b2 = -493.9;
      }
      else if ((1.1 < p_over_d) && (p_over_d <= 1.5))
      {
        a = 35.55, b1 = 263.7, b2 = -190.2;
      }
      else
        mooseError(" Geometric parameters beyond scope of friction factor model ");
    }
  }
  else
  {
    n = 0.18;
    if (subch_type == EChannelType::CORNER)
    {
      ratio = w / rod_diameter;
      if ((1.0 <= p_over_d) && (p_over_d <= 1.1))
      {
        a = 0.09755, b1 = 1.127, b2 = -6.304;
      }
      else if ((1.1 < p_over_d) && (p_over_d <= 1.5))
      {
        a = 0.1452, b1 = 0.02681, b2 = -0.03411;
      }
      else
        mooseError(" Geometric parameters beyond scope of friction factor model ");
    }
    else if (subch_type == EChannelType::EDGE)
    {
      ratio = w / rod_diameter;
      if ((1.0 <= p_over_d) && (p_over_d <= 1.1))
      {
        a = 0.09377, b1 = 0.8732, b2 = -3.341;
      }
      else if ((1.1 < p_over_d) && (p_over_d <= 1.5))
      {
        a = 0.1430, b1 = 0.04199, b2 = -0.04428;
      }
      else
        mooseError(" Geometric parameters beyond scope of friction factor model ");
    }
    else
    {
      ratio = p_over_d;
      if ((1.0 <= p_over_d) && (p_over_d <= 1.1))
      {
        a = 0.09423, b1 = 0.5806, b2 = -1.239;
      }
      else if ((1.1 < p_over_d) && (p_over_d <= 1.5))
      {
        a = 0.1339, b1 = 0.09059, b2 = -0.09926;
      }
      else
        mooseError(" Geometric parameters beyond scope of friction factor model ");
    }
  }
  C = a + b1 * (ratio - 1) + b2 * (ratio - 1) * (ratio - 1);
  return C * std::pow(Re, -n);
}
