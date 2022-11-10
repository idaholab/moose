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
#include "TriSubChannelBaseIC.h"

class TriInterWrapperMesh;

/**
 * An abstract class for ICs for hexagonal fuel assemblies
 */
class TriInterWrapperBaseIC : public InitialCondition
{
public:
  TriInterWrapperBaseIC(const InputParameters & params);

protected:
  /**
   * Check that `mesh` is TriInterWrapperMesh and if not, report an error.
   */
  TriInterWrapperMesh & getMesh(MooseMesh & mesh);

  TriInterWrapperMesh & _mesh;

public:
  static InputParameters validParams();
};
