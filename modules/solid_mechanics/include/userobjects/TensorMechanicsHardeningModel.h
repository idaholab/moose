//* This file is part of the MOOSE framework
//* https://www.mooseframework.org
//*
//* All rights reserved, see COPYRIGHT for full restrictions
//* https://github.com/idaholab/moose/blob/master/COPYRIGHT
//*
//* Licensed under LGPL 2.1, please see LICENSE for details
//* https://www.gnu.org/licenses/lgpl-2.1.html

#pragma once

#include "GeneralUserObject.h"

/**
 * Hardening Model base class.  The derived classes will provide
 * a value and a derivative of that value with respect to a
 * single internal parameter.
 *
 * The virtual functions written below must be
 * over-ridden in derived classes to provide actual values
 */
class TensorMechanicsHardeningModel : public GeneralUserObject
{
public:
  static InputParameters validParams();

  TensorMechanicsHardeningModel(const InputParameters & parameters);

  void initialize();
  void execute();
  void finalize();

  /* provides the value of the hardening parameter at given internal parameter
   * @param intnl the value of the internal parameter at which to evaluate the hardening parameter
   * @return the value of the hardening parameter
   */
  virtual Real value(Real intnl) const;

  /* provides d(hardening parameter)/d(internal parameter)
   * @param intnl the value of the internal parameter at which to evaluate the derivative
   * @return the value of the hardening parameter
   */
  virtual Real derivative(Real intnl) const;

  /* A name for this hardening model.  Plasticity models can use
   * this name to enable certain optimizations
   */
  virtual std::string modelName() const = 0;
};
