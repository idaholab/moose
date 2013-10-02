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

#ifndef POINTERSTOREERROR_H
#define POINTERSTOREERROR_H

#include "GeneralUserObject.h"

class PointerStoreError;

template<>
InputParameters validParams<PointerStoreError>();

class ReallyDumb
{
public:
  int _i;
};

class PointerStoreError : public GeneralUserObject
{
public:
  PointerStoreError(const std::string & name, InputParameters params);
  virtual ~PointerStoreError();

  virtual void initialSetup();
  virtual void timestepSetup();

  virtual void initialize() {};
  virtual void execute();
  virtual void finalize() {};

protected:
  ReallyDumb * & _pointer_data;
};


#endif /* POINTERSTOREERROR_H */
