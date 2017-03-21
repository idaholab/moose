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

#ifndef NODALNORMALSPREPROCESSOR_H
#define NODALNORMALSPREPROCESSOR_H

// MOOSE includes
#include "ElementUserObject.h"
#include "BoundaryRestrictable.h"

// libMesh includes
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
class NodalNormalsPreprocessor : public ElementUserObject, public BoundaryRestrictable
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
   * must
   * execute on each element (see ComputeUserObjectsThread::onElement).
   *
   * The MooseObjectWarehouseBase object that stores the objects uses this method to determine
   * whether
   * the object should be stored as boundary or block. Since this object needs to execute on
   * elements, it must
   * be stored as a block object, overloading this method to always return false has such effect.
   */
  virtual bool boundaryRestricted() override { return false; }

protected:
  AuxiliarySystem & _aux;
  FEType _fe_type;
  bool _has_corners;
  BoundaryID _corner_boundary_id;

  const VariablePhiGradient & _grad_phi;
};

#endif /* NODALNORMALSPREPROCESSOR_H */
