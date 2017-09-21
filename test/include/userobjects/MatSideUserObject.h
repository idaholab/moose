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

#ifndef MatSideUserOBJECT_H
#define MatSideUserOBJECT_H

#include "SideUserObject.h"

// Forward Declarations
class MatSideUserObject;

template <>
InputParameters validParams<MatSideUserObject>();

/*
 * This is for testing error message only. It does nothing.
 */
class MatSideUserObject : public SideUserObject
{
public:
  MatSideUserObject(const InputParameters & parameters);
  virtual void initialize() override {}
  virtual void execute() override {}
  virtual void finalize() override {}
  virtual void threadJoin(const UserObject &) override {}

protected:
  const MaterialProperty<Real> & _mat_prop;
};

#endif
