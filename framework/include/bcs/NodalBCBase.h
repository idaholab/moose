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

#ifndef NODALBCBASE_H
#define NODALBCBASE_H

#include "BoundaryCondition.h"
#include "RandomInterface.h"
#include "CoupleableMooseVariableDependencyIntermediateInterface.h"

// Forward declarations
class NodalBCBase;

// libMesh forward declarations
namespace libMesh
{
template <typename T> class NumericVector;
}

template<>
InputParameters validParams<NodalBCBase>();

/**
 * Base class for deriving any boundary condition that works at nodes
 */
class NodalBCBase :
  public BoundaryCondition,
  public RandomInterface,
  public CoupleableMooseVariableDependencyIntermediateInterface
{
public:
  NodalBCBase(const InputParameters & parameters);

  virtual void computeResidual(NumericVector<Number> & residual) = 0;
  virtual void computeJacobian() = 0;
  virtual void computeOffDiagJacobian(unsigned int jvar) = 0;
};

#endif /* NODALBCBASE_H */
