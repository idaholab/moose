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

#ifndef SPATIALUSEROBJECTAUX_H
#define SPATIALUSEROBJECTAUX_H

#include "AuxKernel.h"

//Forward Declarations
class SpatialUserObjectAux;
class UserObject;

template<>
InputParameters validParams<SpatialUserObjectAux>();

/**
 * Function auxiliary value
 */
class SpatialUserObjectAux : public AuxKernel
{
public:
  /**
   * Factory constructor, takes parameters so that all derived classes can be built using the same
   * constructor.
   */
  SpatialUserObjectAux(const std::string & name, InputParameters parameters);

  virtual ~SpatialUserObjectAux() {}

protected:
  virtual Real computeValue();

  /// UserObject to be queried for a value
  const UserObject & _user_object;
};

#endif // SPATIALUSEROBJECTAUX_H
