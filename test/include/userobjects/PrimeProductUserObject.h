//* This file is part of the MOOSE framework
//* https://www.mooseframework.org
//*
//* All rights reserved, see COPYRIGHT for full restrictions
//* https://github.com/idaholab/moose/blob/master/COPYRIGHT
//*
//* Licensed under LGPL 2.1, please see LICENSE for details
//* https://www.gnu.org/licenses/lgpl-2.1.html

#pragma once

#include "ThreadedGeneralUserObject.h"

/**
 * Computes a product of prime numbers. Each threads picks an n-th prime number where n is equal to
 * its thread ID. Then when threads join the product of all prime numbers is computed, which is the
 * final value provided by this user object.
 */
class PrimeProductUserObject : public ThreadedGeneralUserObject
{
public:
  static InputParameters validParams();

  PrimeProductUserObject(const InputParameters & params);

  virtual void initialize() override;
  virtual void execute() override;
  virtual void finalize() override;

  virtual void threadJoin(const UserObject &) override;

  unsigned int getProduct() const { return _product; }

protected:
  unsigned int factorial(unsigned int n) const;
  unsigned int prime(unsigned int n) const;

  /// Prime number of this instance
  unsigned int _prime;
  /// Product for all prime numbers
  unsigned int _product;
};
