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

#ifndef MUTABLECOEFFICIENTSFUNCTIONINTERFACE_H
#define MUTABLECOEFFICIENTSFUNCTIONINTERFACE_H

// MOOSE includes
#include "FunctionInterface.h"

// Module lincludes
#include "MemoizedFunctionInterface.h"
#include "MutableCoefficientsInterface.h"

// Forward declarations
class MutableCoefficientsFunctionInterface;

template <>
InputParameters validParams<MutableCoefficientsFunctionInterface>();

/// Interface for a type of functions using coefficients that may be changed before or after a solve
class MutableCoefficientsFunctionInterface : public MemoizedFunctionInterface,
                                             protected FunctionInterface,
                                             public MutableCoefficientsInterface
{
public:
  MutableCoefficientsFunctionInterface(const InputParameters & parameters);

protected:
  // Override from MemoizedFunctionInterface
  virtual void coefficientsChanged() override;
};

#endif // MUTABLECOEFFICIENTSFUNCTIONINTERFACE_H
