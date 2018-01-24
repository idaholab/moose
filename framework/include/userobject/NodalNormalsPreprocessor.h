//* This file is part of the MOOSE framework
//* https://www.mooseframework.org
//*
//* All rights reserved, see COPYRIGHT for full restrictions
//* https://github.com/idaholab/moose/blob/master/COPYRIGHT
//*
//* Licensed under LGPL 2.1, please see LICENSE for details
//* https://www.gnu.org/licenses/lgpl-2.1.html

#ifndef NODALNORMALSPREPROCESSOR_H
#define NODALNORMALSPREPROCESSOR_H

// MOOSE includes
#include "ElementUserObject.h"
#include "BoundaryRestrictable.h"

#include "libmesh/fe_type.h"

// Forward declarations
class NodalNormalsPreprocessor;
class AuxiliarySystem;

template <>
InputParameters validParams<NodalNormalsPreprocessor>();

/**
 * An ElementUserObject that prepares MOOSE for computing nodal
 * normals.
 */
class NodalNormalsPreprocessor : public ElementUserObject
{
public:
  NodalNormalsPreprocessor(const InputParameters & parameters);

  virtual void initialize() override;
  virtual void finalize() override;
  virtual void execute() override;
  virtual void threadJoin(const UserObject & uo) override;

  /**
   * Forces object to be stored as a block object.
   *
   * This object inherits from BoundaryRestrictable to utilize the "boundary" parameter and other
   * methods that come with this interface class. However, this object is an ElementUserObject and
   * must execute on each element (see ComputeUserObjectsThread::onElement).
   *
   * The MooseObjectWarehouseBase object that stores the objects uses this method to determine
   * whether the object should be stored as boundary or block. Since this object needs to execute on
   * elements, it must be stored as a block object, overloading this method to always return false
   * has such effect.
   */

protected:
  AuxiliarySystem & _aux;
  FEType _fe_type;
  bool _has_corners;
  std::vector<BoundaryID> _boundaries;
  BoundaryID _corner_boundary_id;

  const VariablePhiGradient & _grad_phi;
};

#endif /* NODALNORMALSPREPROCESSOR_H */
