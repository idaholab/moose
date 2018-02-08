//* This file is part of the MOOSE framework
//* https://www.mooseframework.org
//*
//* All rights reserved, see COPYRIGHT for full restrictions
//* https://github.com/idaholab/moose/blob/master/COPYRIGHT
//*
//* Licensed under LGPL 2.1, please see LICENSE for details
//* https://www.gnu.org/licenses/lgpl-2.1.html

#ifndef NODALNORMALSCORNER_H
#define NODALNORMALSCORNER_H

#include "SideUserObject.h"

class NodalNormalsCorner;
class NodalNormalsUserObject;

template <>
InputParameters validParams<NodalNormalsCorner>();

/**
 * Collect all contributions from boundary nodes that are in corners.
 *
 * @see NodalNormalsUserObject
 */
class NodalNormalsCorner : public SideUserObject
{
public:
  NodalNormalsCorner(const InputParameters & parameters);

  virtual void initialize() override;
  virtual void finalize() override;
  virtual void execute() override;
  virtual void threadJoin(const UserObject & uo) override;

protected:
  /// true if the boundary has corners, otherwise false
  bool _has_corners;
  /// The node set ID containing the corner nodes
  BoundaryID _corner_boundary_id;
  /// The user object that stores the nodal normals
  const NodalNormalsUserObject & _nodal_normals_uo;
};

#endif /* NODALNORMALSCORNER_H */
