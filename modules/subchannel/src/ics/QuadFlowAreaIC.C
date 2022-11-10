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

#include "QuadFlowAreaIC.h"

registerMooseObject("SubChannelApp", QuadFlowAreaIC);

InputParameters
QuadFlowAreaIC::validParams()
{
  InputParameters params = QuadSubChannelBaseIC::validParams();
  params.addClassDescription("Computes subchannel flow area in the square lattice arrangement");
  return params;
}

QuadFlowAreaIC::QuadFlowAreaIC(const InputParameters & params)
  : QuadSubChannelBaseIC(params), _subchannel_mesh(dynamic_cast<SubChannelMesh &>(_mesh))
{
}

Real
QuadFlowAreaIC::value(const Point & p)
{
  auto pitch = _mesh.getPitch();
  auto rod_diameter = _mesh.getRodDiameter();
  auto gap = _mesh.getGap();
  auto z_blockage = _mesh.getZBlockage();
  auto index_blockage = _mesh.getIndexBlockage();
  auto reduction_blockage = _mesh.getReductionBlockage();

  // Compute the flow area for a standard channel (one that touches 4 pins).
  auto rod_area = 0.25 * M_PI * rod_diameter * rod_diameter;
  auto standard_area = pitch * pitch - rod_area;

  auto i = _mesh.getSubchannelIndexFromPoint(p);
  auto subch_type = _mesh.getSubchannelType(i);

  auto index = 0;
  for (const auto & i_blockage : index_blockage)
  {
    if (i == i_blockage && (p(2) >= z_blockage.front() && p(2) <= z_blockage.back()))
    {
      if (subch_type == EChannelType::CORNER)
        return reduction_blockage[index] * (standard_area * 0.25 + pitch * gap + gap * gap);
      else if (subch_type == EChannelType::EDGE)
        return reduction_blockage[index] * (standard_area * 0.5 + pitch * gap);
      else
        return reduction_blockage[index] * standard_area;
    }
    index++;
  }

  if (subch_type == EChannelType::CORNER)
    return standard_area * 0.25 + pitch * gap + gap * gap;
  else if (subch_type == EChannelType::EDGE)
    return standard_area * 0.5 + pitch * gap;
  else
    return standard_area;
}
