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

#ifndef ADDSIDESETSFROMBOUNDINGBOX_H
#define ADDSIDESETSFROMBOUNDINGBOX_H

// MOOSE includes
#include "MooseEnum.h"
#include "MeshModifier.h"

// Forward declerations
class AddSideSetsFromBoundingBox;

template <>
InputParameters validParams<AddSideSetsFromBoundingBox>();

namespace libMesh
{
class BoundingBox;
}

/**
 * MeshModifier for defining a Subdomain inside or outside of a bounding box
 */
class AddSideSetsFromBoundingBox : public MeshModifier
{
public:
  /**
   * Class constructor
   * @param parameters The input parameters
   */
  AddSideSetsFromBoundingBox(const InputParameters & parameters);

  virtual void modify() override;

private:
  /// ID location (inside of outside of box)
  MooseEnum _location;

  /// Block ID to assign to the region
  SubdomainID _block_id;

  /// boundary ID to select
  std::vector<BoundaryName> _boundary_id_old;

  /// boundary ID to assign
  boundary_id_type _boundary_id_new;

  /// Bounding box for testing element centroids against
  BoundingBox _bounding_box;

  /// Flag to determine if the provided boundaries need to overlap
  const bool _boundary_id_overlap;
};

#endif // ADDSIDESETSFROMBOUNDINGBOX_H
