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

#include "TriInterWrapperFlowAreaIC.h"
#include "TriInterWrapperMesh.h"

registerMooseObject("SubChannelApp", TriInterWrapperFlowAreaIC);

InputParameters
TriInterWrapperFlowAreaIC::validParams()
{
  InputParameters params = TriInterWrapperBaseIC::validParams();
  return params;
}

TriInterWrapperFlowAreaIC::TriInterWrapperFlowAreaIC(const InputParameters & params)
  : TriInterWrapperBaseIC(params)
{
}

Real
TriInterWrapperFlowAreaIC::value(const Point & p)
{
  auto pitch = _mesh.getPitch();
  auto flat_to_flat = _mesh.getSideX();
  auto gap = _mesh.getDuctToRodGap();
  auto element_side = flat_to_flat * std::tan(libMesh::pi / 6.0);

  auto i = _mesh.getSubchannelIndexFromPoint(p);
  // given the channel number, i, it computes the flow area of the subchannel
  // based on the subchannel type: CENTER, EDGE or CORNER.
  auto subch_type = _mesh.getSubchannelType(i);
  if (subch_type == EChannelType::CENTER)
  {
    return (pitch - flat_to_flat) * element_side * 3.0 / 2.0;
  }
  else if (subch_type == EChannelType::EDGE)
  {
    return (pitch - flat_to_flat) * element_side * 1.0 / 2.0 +
           (element_side + gap * std::tan(libMesh::pi / 6.0) / 2.0) * gap;
  }
  else
  {
    return (element_side + gap * std::tan(libMesh::pi / 6.0) / 2.0) * gap;
  }
}
