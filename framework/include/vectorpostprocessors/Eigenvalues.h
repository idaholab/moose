//* This file is part of the MOOSE framework
//* https://www.mooseframework.org
//*
//* All rights reserved, see COPYRIGHT for full restrictions
//* https://github.com/idaholab/moose/blob/master/COPYRIGHT
//*
//* Licensed under LGPL 2.1, please see LICENSE for details
//* https://www.gnu.org/licenses/lgpl-2.1.html

#ifndef EIGENVALUES_H
#define EIGENVALUES_H

#include "GeneralVectorPostprocessor.h"

// Forward Declarations
class Eigenvalues;
class NonlinearEigenSystem;

template <>
InputParameters validParams<Eigenvalues>();

class Eigenvalues : public GeneralVectorPostprocessor
{
public:
  Eigenvalues(const InputParameters & parameters);

  virtual void initialize() override;
  virtual void execute() override;

protected:
  VectorPostprocessorValue & _eigen_values_real;
  VectorPostprocessorValue & _eigen_values_imag;
  NonlinearEigenSystem * _nl_eigen;
};

#endif // EIGENVALUES_H
