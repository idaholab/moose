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

#ifndef NUMINTERNALSIDES_H
#define NUMINTERNALSIDES_H

// MOOSE includes
#include "InternalSidePostprocessor.h"

// Forward declerations
class NumInternalSides;

template<>
InputParameters validParams<NumInternalSides>();

/**
 * An object for testing the block restricted behavior of InternalSideUserObject, it
 * simply counts the number of sides
 */
class NumInternalSides : public InternalSidePostprocessor
{
public:
  NumInternalSides(const std::string & name, InputParameters parameters);
  virtual ~NumInternalSides();
  virtual void execute();
  virtual void threadJoin(const UserObject & uo);
  virtual void finalize();
  virtual void initialize();
  virtual PostprocessorValue getValue();
  const unsigned int & count() const { return _count; }

private:
  unsigned int _count;
};

#endif //NUMINTERNALSIDES_H
