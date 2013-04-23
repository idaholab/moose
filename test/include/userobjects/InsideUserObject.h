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

#ifndef INSIDEUSEROBJECT_H
#define INSIDEUSEROBJECT_H

#include "InternalSideUserObject.h"

class InsideUserObject;

template<>
InputParameters validParams<InsideUserObject>();

/**
 *
 */
class InsideUserObject : public InternalSideUserObject
{
public:
  InsideUserObject(const std::string & name, InputParameters parameters);
  virtual ~InsideUserObject();

  virtual void initialize();
  virtual void execute();
  virtual void destroy();
  virtual void finalize();
  virtual void threadJoin(const UserObject & uo);

  Real getValue() const { return _value; }

protected:
  VariableValue & _u;
  VariableValue & _u_neighbor;

  Real _value;
};

#endif /* INSIDEUSEROBJECT_H */
