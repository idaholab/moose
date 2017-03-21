/****************************************************************/
/*               DO NOT MODIFY THIS HEADER                      */
/* MOOSE - Multiphysics Object Oriented Simulation Environment  */
/*                                                              */
/*           (c) 2010 Battelle Energy Alliance, LLC             */
/*                   ALL RIGHTS RESERVED                        */
/*                                                              */
/*          Prepared by Battelle Energy Alliance, LLC           */
/*            Under Contract No. DE-AC07-05ID14517              */
/*            With the U. S. Department of Energy               */
/*                                                              */
/*            See COPYRIGHT for full restrictions               */
/****************************************************************/

#ifndef ORIENTEDSUBDOMAINBOUNDINGBOX_H
#define ORIENTEDSUBDOMAINBOUNDINGBOX_H

// MOOSE includes
#include "MeshModifier.h"
#include "OrientedBoxInterface.h"

// Forward declerations
class OrientedSubdomainBoundingBox;

template <>
InputParameters validParams<OrientedSubdomainBoundingBox>();

/**
 * MeshModifier for defining a Subdomain inside or outside of a bounding box with arbitrary
 * orientation
 */
class OrientedSubdomainBoundingBox : public MeshModifier, public OrientedBoxInterface
{
public:
  /**
   * Class constructor
   * @param parameters The parameters object holding data for the class to use.
   */
  OrientedSubdomainBoundingBox(const InputParameters & parameters);

private:
  virtual void modify() override;

  /// ID location (inside of outside of box)
  MooseEnum _location;

  /// Block ID to assign to the region
  SubdomainID _block_id;
};

#endif // ORIENTEDSUBDOMAINBOUNDINGBOX_H
