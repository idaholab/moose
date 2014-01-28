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

#ifndef TESTBOUNDARYRESTRICTABLEASSERT_H
#define TESTBOUNDARYRESTRICTABLEASSERT_H

// MOOSE includes
#include "SideUserObject.h"

// Forward declerations
class TestBoundaryRestrictableAssert;

template<>
InputParameters validParams<TestBoundaryRestrictableAssert>();

/**
 * An empty user object used for testing an assert in BoundaryRestrictable
 */
class TestBoundaryRestrictableAssert : public SideUserObject
{
public:

  TestBoundaryRestrictableAssert(const std::string & name, InputParameters parameters);
  virtual ~TestBoundaryRestrictableAssert(){}
  virtual void execute(){}
  virtual void threadJoin(const UserObject & /*uo*/){}
  virtual void initialize(){}
  virtual void finalize(){}

};

#endif //TESTBOUNDARYRESTRICTABLEASSERT_H
