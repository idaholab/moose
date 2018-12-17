//* This file is part of the MOOSE framework
//* https://www.mooseframework.org
//*
//* All rights reserved, see COPYRIGHT for full restrictions
//* https://github.com/idaholab/moose/blob/master/COPYRIGHT
//*
//* Licensed under LGPL 2.1, please see LICENSE for details
//* https://www.gnu.org/licenses/lgpl-2.1.html

#ifndef GLOBALSTRAIN_H
#define GLOBALSTRAIN_H

#include "ScalarKernel.h"

// Forward Declarations
class GlobalStrain;
class GlobalStrainUserObjectInterface;
template <typename>
class RankTwoTensorTempl;
typedef RankTwoTensorTempl<Real> RankTwoTensor;
template <typename>
class RankFourTensorTempl;
typedef RankFourTensorTempl<Real> RankFourTensor;

template <>
InputParameters validParams<GlobalStrain>();

class GlobalStrain : public ScalarKernel
{
public:
  GlobalStrain(const InputParameters & parameters);

  virtual void reinit(){};
  virtual void computeResidual();
  virtual void computeJacobian();
  virtual void computeOffDiagJacobian(unsigned int /*jvar*/);

protected:
  virtual void assignComponentIndices(Order var_order);

  const GlobalStrainUserObjectInterface & _pst;
  const RankTwoTensor & _pst_residual;
  const RankFourTensor & _pst_jacobian;
  const VectorValue<bool> & _periodic_dir;

  std::vector<std::pair<unsigned int, unsigned int>> _components;
  const unsigned int _dim;
};
#endif // GLOBALSTRAIN_H
