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
 * Demonstration of user-data object
 */
class MTUserObject : public GeneralUserObject
{
public:
  static InputParameters validParams();

  MTUserObject(const InputParameters & params);
  virtual ~MTUserObject();

  /**
   * Called before execute() is ever called so that data can be cleared.
   */
  virtual void initialize() {}

  /**
   * Called when this object needs to compute something.
   */
  virtual void execute() {}

  virtual void finalize() {}

  /**
   * A function that does something
   */
  Real doSomething() const;

  /**
   * Get scalar value
   */
  Real getScalar() const { return _scalar; }

  /**
   * Set the scalar value
   * @param scalar
   */
  void setScalar(Real scalar) { _scalar = scalar; }

  virtual void load(std::ifstream & stream);
  virtual void store(std::ofstream & stream);

protected:
  /// A scalar value
  Real _scalar;
  /// A vector value
  const std::vector<Real> & _vector;
  /// Dynamically allocated memory
  Real * _dyn_memory;

protected:
  /// Number of elements to allocate (we do not like magic numbers)
  static const unsigned int NUM = 10;
};
