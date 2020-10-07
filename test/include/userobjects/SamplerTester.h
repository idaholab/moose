//* This file is part of the MOOSE framework
//* https://www.mooseframework.org
//*
//* All rights reserved, see COPYRIGHT for full restrictions
//* https://github.com/idaholab/moose/blob/master/COPYRIGHT
//*
//* Licensed under LGPL 2.1, please see LICENSE for details
//* https://www.gnu.org/licenses/lgpl-2.1.html

#pragma once

#include "GeneralPostprocessor.h"
#include "SamplerInterface.h"
#include "Sampler.h"

/**
 * UserObject for testing Sampler object threaded and parallel behavior, it should be used for
 * anything else, see TestSampler object.
 */
class SamplerTester : public GeneralPostprocessor
{
public:
  static InputParameters validParams();

  SamplerTester(const InputParameters & parameters);

protected:
  void assertEqual(const Real & value, const Real & gold);
  void assertEqual(const DenseMatrix<Real> & value, const DenseMatrix<Real> & gold);

  virtual void execute() final;
  virtual void initialize() final;
  virtual void finalize() final;
  virtual Real getValue() final;
  virtual void threadJoin(const UserObject & uo) final;
  Sampler & _sampler;
  const MooseEnum & _test_type;
  DenseMatrix<Real> _samples;
};
