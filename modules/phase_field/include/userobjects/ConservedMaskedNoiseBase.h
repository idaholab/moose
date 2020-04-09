//* This file is part of the MOOSE framework
//* https://www.mooseframework.org
//*
//* All rights reserved, see COPYRIGHT for full restrictions
//* https://github.com/idaholab/moose/blob/master/COPYRIGHT
//*
//* Licensed under LGPL 2.1, please see LICENSE for details
//* https://www.gnu.org/licenses/lgpl-2.1.html

#pragma once

#include "ConservedNoiseInterface.h"

#include <unordered_map>

// Forward Declarations

/**
 * This Userobject is the base class of Userobjects that generate one
 * random number per timestep and quadrature point in a way that the integral
 * over all random numbers is zero. It behaves as ConservedNoiseBase but allows
 * the user to specify a multiplicator in the form of a MaterialProperty that is
 * multiplied on each random number, effectively masking the random number field.
 *
 * \see ConservedUniformNoise
 * \see ConservedNormalNoise
 */
class ConservedMaskedNoiseBase : public ConservedNoiseInterface
{
public:
  static InputParameters validParams();

  ConservedMaskedNoiseBase(const InputParameters & parameters);

  virtual ~ConservedMaskedNoiseBase() {}

  virtual void initialize();
  virtual void execute();
  virtual void threadJoin(const UserObject & y);
  virtual void finalize();

  Real getQpValue(dof_id_type element_id, unsigned int qp) const;

protected:
  std::unordered_map<dof_id_type, std::vector<std::pair<Real, Real>>> _random_data;

  const MaterialProperty<Real> & _mask;
};
