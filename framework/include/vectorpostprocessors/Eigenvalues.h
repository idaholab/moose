//* This file is part of the MOOSE framework
//* https://www.mooseframework.org
//*
//* All rights reserved, see COPYRIGHT for full restrictions
//* https://github.com/idaholab/moose/blob/master/COPYRIGHT
//*
//* Licensed under LGPL 2.1, please see LICENSE for details
//* https://www.gnu.org/licenses/lgpl-2.1.html

#pragma once

#include "GeneralVectorPostprocessor.h"

// Forward Declarations
class Eigenvalues;
class NonlinearEigenSystem;

template <>
InputParameters validParams<Eigenvalues>();

class Eigenvalues : public GeneralVectorPostprocessor
{
public:
  static InputParameters validParams();

  Eigenvalues(const InputParameters & parameters);

  virtual void initialize() override {}
  virtual void execute() override;

protected:
  const bool _inverse;
  VectorPostprocessorValue & _eigen_values_real;
  VectorPostprocessorValue & _eigen_values_imag;
  NonlinearEigenSystem * _nl_eigen;
};
