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

#include "InitialCondition.h"

class QuadSubChannelMesh;

/**
 * An abstract class for ICs for quadrilateral subchannels
 */
class QuadSubChannelBaseIC : public InitialCondition
{
public:
  QuadSubChannelBaseIC(const InputParameters & params);

protected:
  /**
   * Check that `mesh` is QuadSubChannelMesh and if not, report an error.
   */
  QuadSubChannelMesh & getMesh(MooseMesh & mesh);

  QuadSubChannelMesh & _mesh;

public:
  static InputParameters validParams();
};
