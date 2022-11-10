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

class TriSubChannelMesh;

/**
 * An abstract class for ICs for hexagonal fuel assemblies
 */
class TriSubChannelBaseIC : public InitialCondition
{
public:
  TriSubChannelBaseIC(const InputParameters & params);

protected:
  /**
   * Check that `mesh` is TriSubChannelMesh and if not, report an error.
   */
  TriSubChannelMesh & getMesh(MooseMesh & mesh);

  TriSubChannelMesh & _mesh;

public:
  static InputParameters validParams();
};
