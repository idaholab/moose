//* This file is part of the MOOSE framework
//* https://www.mooseframework.org
//*
//* All rights reserved, see COPYRIGHT for full restrictions
//* https://github.com/idaholab/moose/blob/master/COPYRIGHT
//*
//* Licensed under LGPL 2.1, please see LICENSE for details
//* https://www.gnu.org/licenses/lgpl-2.1.html

#pragma once

#include "SidePostprocessor.h"
#include "MooseVariableInterface.h"

class ReflectionCoefficient : public SidePostprocessor, public MooseVariableInterface<Real>
{
public:
  static InputParameters validParams();

  ReflectionCoefficient(const InputParameters & parameters);

  virtual void initialize() override;
  virtual void execute() override;
  virtual PostprocessorValue getValue() override;
  virtual void threadJoin(const UserObject & y) override;
  virtual void finalize() override;

protected:
  virtual Real computeReflection();

  unsigned int _qp;

  const VariableValue & _coupled_real;

  const VariableValue & _coupled_imag;

  Real _theta;

  Real _length;

  Real _k;

  Real _incoming_mag;

  Real _reflection_coefficient;
};
