//* This file is part of the MOOSE framework
//* https://www.mooseframework.org
//*
//* All rights reserved, see COPYRIGHT for full restrictions
//* https://github.com/idaholab/moose/blob/master/COPYRIGHT
//*
//* Licensed under LGPL 2.1, please see LICENSE for details
//* https://www.gnu.org/licenses/lgpl-2.1.html

#ifndef ORIENTEDSUBDOMAINBOUNDINGBOX_H
#define ORIENTEDSUBDOMAINBOUNDINGBOX_H

// MOOSE includes
#include "MooseEnum.h"
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
  const MooseEnum _location;

  /// Block ID to assign to the region
  const SubdomainID _block_id;
};

#endif // ORIENTEDSUBDOMAINBOUNDINGBOX_H
