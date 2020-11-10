//* This file is part of the MOOSE framework
//* https://www.mooseframework.org
//*
//* All rights reserved, see COPYRIGHT for full restrictions
//* https://github.com/idaholab/moose/blob/master/COPYRIGHT
//*
//* Licensed under LGPL 2.1, please see LICENSE for details
//* https://www.gnu.org/licenses/lgpl-2.1.html

#pragma once

#include "ScalarKernel.h"
#include "RankTwoTensorForward.h"
#include "RankFourTensorForward.h"

// Forward Declarations
class GlobalStrainUserObjectInterface;

class GlobalStrain : public ScalarKernel
{
public:
  static InputParameters validParams();

  GlobalStrain(const InputParameters & parameters);

  virtual void reinit() override {}
  virtual void computeResidual() override;
  virtual void computeJacobian() override;
  virtual void computeOffDiagJacobian(unsigned int) override {}

protected:
  virtual void assignComponentIndices(Order var_order);

  const GlobalStrainUserObjectInterface & _pst;
  const RankTwoTensor & _pst_residual;
  const RankFourTensor & _pst_jacobian;
  const VectorValue<bool> & _periodic_dir;

  std::vector<std::pair<unsigned int, unsigned int>> _components;
  const unsigned int _dim;
};
