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

#ifndef SUBDOMAINBOUNDINGBOX_H
#define SUBDOMAINBOUNDINGBOX_H

// MOOSE includes
#include "MeshModifier.h"

// libmesh includes
#include "libmesh/mesh_tools.h"

// Forward declerations
class SubdomainBoundingBox;

template <>
InputParameters validParams<SubdomainBoundingBox>();

/**
 * MeshModifier for defining a Subdomain inside or outside of a bounding box
 */
class SubdomainBoundingBox : public MeshModifier
{
public:
  /**
   * Class constructor
   * @param parameters The input parameters
   */
  SubdomainBoundingBox(const InputParameters & parameters);

  virtual void modify() override;

private:
  /// ID location (inside of outside of box)
  MooseEnum _location;

  /// Block ID to assign to the region
  SubdomainID _block_id;

  /// Bounding box for testing element centroids against
  MeshTools::BoundingBox _bounding_box;
};

#endif // SUBDOMAINBOUDINGBOX_H
