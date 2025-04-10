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

#pragma once

#include "TriSubChannelBaseIC.h"
#include "SubChannelMesh.h"

class TriSubChannelMesh;

/**
 * This class calculates the area of the triangular, edge, and corner subchannels for hexagonal fuel
 * assemblies in the Marvel microreactor design.
 */
class MarvelTriFlowAreaIC : public TriSubChannelBaseIC
{
public:
  MarvelTriFlowAreaIC(const InputParameters & params);
  Real value(const Point & p) override;

public:
  static InputParameters validParams();

protected:
  const SubChannelMesh & _subchannel_mesh;
};
