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

#include "SCMQuadFlowAreaIC.h"

registerMooseObject("SubChannelApp", SCMQuadFlowAreaIC);

InputParameters
SCMQuadFlowAreaIC::validParams()
{
  InputParameters params = QuadSubChannelBaseIC::validParams();
  params.addClassDescription(
      "Computes subchannel flow area in the square lattice subchannel arrangement");
  return params;
}

SCMQuadFlowAreaIC::SCMQuadFlowAreaIC(const InputParameters & params)
  : QuadSubChannelBaseIC(params), _subchannel_mesh(dynamic_cast<SubChannelMesh &>(_mesh))
{
}

Real
SCMQuadFlowAreaIC::value(const Point & p)
{
  Real standard_area, rod_area, additional_area;
  auto pitch = _mesh.getPitch();
  auto pin_diameter = _mesh.getPinDiameter();
  auto gap = _mesh.getGap();
  auto z_blockage = _mesh.getZBlockage();
  auto index_blockage = _mesh.getIndexBlockage();
  auto reduction_blockage = _mesh.getReductionBlockage();
  auto i = _mesh.getSubchannelIndexFromPoint(p);
  auto subch_type = _mesh.getSubchannelType(i);

  if (subch_type == EChannelType::CORNER)
  {
    standard_area = 0.25 * pitch * pitch;
    rod_area = 0.25 * 0.25 * M_PI * pin_diameter * pin_diameter;
    additional_area = pitch * gap + gap * gap;
  }
  else if (subch_type == EChannelType::EDGE)
  {
    standard_area = 0.5 * pitch * pitch;
    rod_area = 0.5 * 0.25 * M_PI * pin_diameter * pin_diameter;
    additional_area = pitch * gap;
  }
  else
  {
    standard_area = pitch * pitch;
    rod_area = 0.25 * M_PI * pin_diameter * pin_diameter;
    additional_area = 0.0;
  }

  /// Calculate subchannel area
  auto subchannel_area = standard_area + additional_area - rod_area;

  /// Apply area reduction on subchannels affected by blockage
  auto index = 0;
  for (const auto & i_blockage : index_blockage)
  {
    if (i == i_blockage && (p(2) >= z_blockage.front() && p(2) <= z_blockage.back()))
    {
      return reduction_blockage[index] * subchannel_area;
    }
    index++;
  }

  return subchannel_area;
}
