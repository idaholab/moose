//* This file is part of the MOOSE framework
//* https://www.mooseframework.org
//*
//* All rights reserved, see COPYRIGHT for full restrictions
//* https://github.com/idaholab/moose/blob/master/COPYRIGHT
//*
//* Licensed under LGPL 2.1, please see LICENSE for details
//* https://www.gnu.org/licenses/lgpl-2.1.html

#pragma once

#include "GeneralUserObjectBasePD.h"
#include "PeridynamicsMesh.h"

class RandomizeCriticalValuePD;

template <>
InputParameters validParams<RandomizeCriticalValuePD>();

/**
 * Userobject class to generate randomized critical value used to determine bond status
 * for fracture modeling and simulation
 */
class RandomizeCriticalValuePD : public GeneralUserObjectBasePD
{
public:
  RandomizeCriticalValuePD(const InputParameters & parameters);

  virtual void initialize() override {}
  virtual void execute() override;
  virtual void finalize() override;

protected:
  /// Reference to the auxiliary system
  AuxiliarySystem & _aux;

  /// Solution vector for all aux variables
  NumericVector<Number> & _aux_sln;

  ///@{ Paramters for Gaussian distribution
  const Real _mean;
  const Real _standard_deviation;
  ///@}

  /// Critical elemental aux variable
  MooseVariable * _critical_var;
};
