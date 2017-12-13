//* This file is part of the MOOSE framework
//* https://www.mooseframework.org
//*
//* All rights reserved, see COPYRIGHT for full restrictions
//* https://github.com/idaholab/moose/blob/master/COPYRIGHT
//*
//* Licensed under LGPL 2.1, please see LICENSE for details
//* https://www.gnu.org/licenses/lgpl-2.1.html

#ifndef NODALNORMALSBOUNDARYNODES_H
#define NODALNORMALSBOUNDARYNODES_H

#include "ElementUserObject.h"

#include "libmesh/fe_type.h"

// Forward declarations
class NodalNormalsBoundaryNodes;
class NodalNormalsUserObject;

template <>
InputParameters validParams<NodalNormalsBoundaryNodes>();

/**
 * Collect all contributions to nodal normals from boundary nodes and feed them into
 * NodalNormalsUserObject
 *
 * @see NodalNormalsUserObject
 */
class NodalNormalsBoundaryNodes : public ElementUserObject
{
public:
  NodalNormalsBoundaryNodes(const InputParameters & parameters);

  virtual void initialize() override;
  virtual void finalize() override;
  virtual void execute() override;
  virtual void threadJoin(const UserObject & uo) override;

protected:
  /// FE type that defines the grad of a test function
  FEType _fe_type;
  /// The boundary ID where we compute the nodal normals
  BoundaryID _boundary_id;
  /// true if the boundary has corners, otherwise false
  bool _has_corners;
  /// The node set ID containing the corner nodes
  BoundaryID _corner_boundary_id;
  /// The gradient of the test function
  const VariablePhiGradient & _grad_phi;
  /// The user object that stores the nodal normals
  const NodalNormalsUserObject & _nodal_normals_uo;
};

#endif /* NODALNORMALSBOUNDARYNODES_H */
