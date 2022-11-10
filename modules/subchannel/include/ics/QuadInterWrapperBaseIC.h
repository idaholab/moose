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
#include "QuadSubChannelBaseIC.h"

class QuadInterWrapperMesh;

/**
 * An abstract class for ICs for quadrilateral subchannels
 */
class QuadInterWrapperBaseIC : public InitialCondition
{
public:
  QuadInterWrapperBaseIC(const InputParameters & params);

protected:
  /**
   * Check that `mesh` is QuadSubChannelMesh and if not, report an error.
   */
  QuadInterWrapperMesh & getMesh(MooseMesh & mesh);

  QuadInterWrapperMesh & _mesh;

public:
  static InputParameters validParams();
};
