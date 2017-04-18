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

#ifndef SCALARVARIABLEINDEXPROVIDER_H
#define SCALARVARIABLEINDEXPROVIDER_H

#include "GeneralUserObject.h"

class ScalarVariableIndexProvider;

template <>
InputParameters validParams<ScalarVariableIndexProvider>();

/**
 * Abstract base class for user objects that provide subblock scalar variable index
 */
class ScalarVariableIndexProvider : public GeneralUserObject
{
public:
  ScalarVariableIndexProvider(const InputParameters & params) : GeneralUserObject(params) {}

  /**
   * The index of scalar variable this element is operating on.
   */
  virtual unsigned int getScalarVarIndex(const Elem & /* elem */) const = 0;
};

#endif /* SCALARVARIABLEINDEXPROVIDER_H */
