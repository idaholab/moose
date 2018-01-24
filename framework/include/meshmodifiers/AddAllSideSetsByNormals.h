//* This file is part of the MOOSE framework
//* https://www.mooseframework.org
//*
//* All rights reserved, see COPYRIGHT for full restrictions
//* https://github.com/idaholab/moose/blob/master/COPYRIGHT
//*
//* Licensed under LGPL 2.1, please see LICENSE for details
//* https://www.gnu.org/licenses/lgpl-2.1.html

#ifndef ADDALLSIDESETS_H
#define ADDALLSIDESETS_H

#include "AddSideSetsBase.h"
#include "MooseTypes.h"

class AddAllSideSetsByNormals;

template <>
InputParameters validParams<AddAllSideSetsByNormals>();

/**
 * This class will add sidesets to the entire mesh based on unique normals.  This
 * information will be made available through the MooseMesh interface.
 * Note: This algorithm may not work well with meshes containing curved faces.
 * Several sidesets may be created in that case.  Use sensibly!
 */
class AddAllSideSetsByNormals : public AddSideSetsBase
{
public:
  AddAllSideSetsByNormals(const InputParameters & parameters);

protected:
  virtual void modify() override;

  BoundaryID getNextBoundaryID();

  /**
   * A pointer to the Mesh's boundary set, this datastructure will be modified
   * through this modifier.
   */
  std::set<BoundaryID> _mesh_boundary_ids;
};

#endif /* ADDALLSIDESETS_H */
