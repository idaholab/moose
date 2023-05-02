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

#include "TriInterWrapperWettedPerimIC.h"
#include "TriInterWrapperMesh.h"

registerMooseObject("SubChannelApp", TriInterWrapperWettedPerimIC);

InputParameters
TriInterWrapperWettedPerimIC::validParams()
{
  InputParameters params = TriSubChannelBaseIC::validParams();
  params.addClassDescription(
      "Computes wetted perimeter of inter-wrapper cells in a triangualar subchannel lattice");
  return params;
}

TriInterWrapperWettedPerimIC::TriInterWrapperWettedPerimIC(const InputParameters & params)
  : TriInterWrapperBaseIC(params)
{
}

Real
TriInterWrapperWettedPerimIC::value(const Point & p)
{
  auto flat_to_flat = _mesh.getSideX();
  auto gap = _mesh.getDuctToRodGap();
  auto element_side = flat_to_flat * std::tan(libMesh::pi / 6.0);

  auto i = _mesh.getSubchannelIndexFromPoint(p);
  // given the channel number, i, it computes the flow area of the subchannel
  // based on the subchannel type: CENTER, EDGE or CORNER.
  auto subch_type = _mesh.getSubchannelType(i);
  if (subch_type == EChannelType::CENTER)
  {
    return 3.0 * element_side;
  }
  else if (subch_type == EChannelType::EDGE)
  {
    return 3.0 * element_side + 2.0 * gap * std::tan(libMesh::pi / 6.0);
  }
  else
  {
    return 2.0 * element_side + 2.0 * gap * std::tan(libMesh::pi / 6.0);
  }
}
