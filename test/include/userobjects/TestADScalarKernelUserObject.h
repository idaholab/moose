//* This file is part of the MOOSE framework
//* https://www.mooseframework.org
//*
//* All rights reserved, see COPYRIGHT for full restrictions
//* https://github.com/idaholab/moose/blob/master/COPYRIGHT
//*
//* Licensed under LGPL 2.1, please see LICENSE for details
//* https://www.gnu.org/licenses/lgpl-2.1.html

#pragma once

#include "ElementUserObject.h"
#include "MooseVariableInterface.h"

/**
 * User object used for testing ADScalarKernel
 */
class TestADScalarKernelUserObject : public ElementUserObject, public MooseVariableInterface<Real>
{
public:
  static InputParameters validParams();

  TestADScalarKernelUserObject(const InputParameters & parameters);

  virtual void initialize();
  virtual void execute();
  virtual void threadJoin(const UserObject & y);
  virtual void finalize();
  virtual ADReal getValue() const;

protected:
  /// Holds the solution at current quadrature points
  const ADVariableValue & _u;
  /// AD version of JxW
  const MooseArray<ADReal> & _ad_JxW;
  /// AD version of coord
  const MooseArray<ADReal> & _ad_coord;

  /// Integral
  ADReal _integral_value;
};
