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

#ifndef ADDALLSIDESETS_H
#define ADDALLSIDESETS_H

#include "AddSideSetsBase.h"
#include "MooseTypes.h"

class AddAllSideSetsByNormals;

template<>
InputParameters validParams<AddAllSideSetsByNormals>();

/**
 * This class will add sidesets to the entire mesh based on unique normals.  This
 * information will be made available through the MooseMesh interface.
 * Note: This algorithm will not work well with meshes containing curved faces.
 * Several sidesets will be created in that case.  Use sensibly!
 */
class AddAllSideSetsByNormals : public AddSideSetsBase
{
public:
  AddAllSideSetsByNormals(const std::string & name, InputParameters parameters);

  virtual ~AddAllSideSetsByNormals();

  virtual void modify();

protected:
  BoundaryID getNextBoundaryID();

  /**
   * A pointer to the Mesh's boundary set, this datastructure will be modified
   * through this modifier.
   */
  std::set<BoundaryID> *_mesh_boundary_ids;
};

#endif /* ADDALLSIDESETS_H */
