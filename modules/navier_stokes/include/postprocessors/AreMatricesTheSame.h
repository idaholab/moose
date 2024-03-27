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

namespace libMesh
{
template <typename>
class PetscMatrix;
}

class AreMatricesTheSame : public GeneralPostprocessor
{
public:
  static InputParameters validParams();

  AreMatricesTheSame(const InputParameters & parameters);

  virtual void initialize() override;
  virtual void execute() override;
  virtual void finalize() override;
  virtual Real getValue() const override;

protected:
  const Real _equiv_tol;
  const std::string & _mat1_name;
  const std::string & _mat2_name;
  std::unique_ptr<SparseMatrix<Number>> _mat1;
  std::unique_ptr<SparseMatrix<Number>> _mat2;
  bool _equiv = true;
};
