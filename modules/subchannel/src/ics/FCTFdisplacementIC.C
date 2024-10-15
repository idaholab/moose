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

#include "FCTFdisplacementIC.h"
#include "TriSubChannelMesh.h"

registerMooseObject("SubChannelApp", FCTFdisplacementIC);

InputParameters
FCTFdisplacementIC::validParams()
{
  InputParameters params = TriSubChannelBaseIC::validParams();
  params.addClassDescription(
      "This class calculates the displacement of the duct for the areva FCTF");
  return params;
}

FCTFdisplacementIC::FCTFdisplacementIC(const InputParameters & params)
  : TriSubChannelBaseIC(params), _subchannel_mesh(dynamic_cast<SubChannelMesh &>(_mesh))
{
}

Real
FCTFdisplacementIC::value(const Point & p)
{
  auto L = _mesh.getHeatedLength();
  auto LIN = _mesh.getHeatedLengthEntry();
  auto P = _mesh.getPitch();
  auto Dmax = 0.001064;
  auto Side = 0.05291;
  auto i = _mesh.getSubchannelIndexFromPoint(p);
  auto subch_type = _mesh.getSubchannelType(i);
  auto x = p(0);
  auto y = p(1);
  auto z = p(2);

  if (subch_type == EChannelType::EDGE)
  {
    if ((y > 2.0 * sqrt(3) * P) && ((LIN + L) >= z) && z >= LIN) // TOP
    {
      return ((Dmax / 2.0) * cos(x * pi / (Side / 2.0)) + Dmax / 2) * sin(((z - LIN) / (L)) * pi);
    }
    else if ((y < -2.0 * sqrt(3) * P) && ((LIN + L) >= z) && z >= LIN) // BOTTOM
    {
      return ((Dmax / 2.0) * cos(x * pi / (Side / 2.0)) + Dmax / 2) * sin(((z - LIN) / (L)) * pi);
    }
    else if (y > sqrt(3) * x + sqrt(3) * 4.0 * P && ((LIN + L) >= z) && z >= LIN) // TOP LEFT
    {
      auto xprime = x * cos(pi / 3.0) + y * sin(pi / 3.0);
      return ((Dmax / 2.0) * cos(xprime * pi / (Side / 2.0)) + Dmax / 2) *
             sin(((z - LIN) / (L)) * pi);
    }
    else if (y < -sqrt(3) * x - sqrt(3) * 4.0 * P && ((LIN + L) >= z) && z >= LIN) // BOTTOM LEFT
    {
      auto xprime = x * cos(2.0 * pi / 3.0) + y * sin(2.0 * pi / 3.0);
      return ((Dmax / 2.0) * cos(xprime * pi / (Side / 2.0)) + Dmax / 2) *
             sin(((z - LIN) / (L)) * pi);
    }
    else if (y < sqrt(3) * x - sqrt(3) * 4.0 * P && ((LIN + L) >= z) && z >= LIN) // BOTTOM RIGHT
    {
      auto xprime = x * cos(4.0 * pi / 3.0) + y * sin(4.0 * pi / 3.0);
      return ((Dmax / 2.0) * cos(xprime * pi / (Side / 2.0)) + Dmax / 2) *
             sin(((z - LIN) / (L)) * pi);
    }
    else if (y > -sqrt(3) * x + sqrt(3) * 4.0 * P && ((LIN + L) >= z) && z >= LIN) // TOP RIGHT
    {
      auto xprime = x * cos(5.0 * pi / 3.0) + y * sin(5.0 * pi / 3.0);
      return ((Dmax / 2.0) * cos(xprime * pi / (Side / 2.0)) + Dmax / 2) *
             sin(((z - LIN) / (L)) * pi);
    }
    else
    {
      return 0.0;
    }
  }
  else if (subch_type == EChannelType::CORNER)
  {
    auto xprime = P + Side / 4.0;
    return ((Dmax / 2.0) * cos(xprime * pi / (Side / 2.0)) + Dmax / 2) *
           sin(((z - LIN) / (L)) * pi);
  }
  else
  {
    return 0.0;
  }
}
