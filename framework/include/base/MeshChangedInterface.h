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

#ifndef MESHCHANGEDINTERFACE_H
#define MESHCHANGEDINTERFACE_H

#include "InputParameters.h"
#include "MooseEnum.h"

// Forward declarations
class FEProblemBase;
class InputParameters;
class MeshChangedInterface;

template <typename T>
InputParameters validParams();

template <>
InputParameters validParams<MeshChangedInterface>();

/**
 * Interface for notifications that the mesh has changed.
 */
class MeshChangedInterface
{
public:
  MeshChangedInterface(const InputParameters & params);
  virtual ~MeshChangedInterface() = default;

  /**
   * Called on this object when the mesh changes
   */
  virtual void meshChanged() {}

protected:
  /// Reference to FEProblemBase instance
  FEProblemBase & _mci_feproblem;
};

#endif /* MESHCHANGEDINTERFACE_H */
