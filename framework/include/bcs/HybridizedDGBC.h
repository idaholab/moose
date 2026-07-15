//* This file is part of the MOOSE framework
//* https://mooseframework.inl.gov
//*
//* All rights reserved, see COPYRIGHT for full restrictions
//* https://github.com/idaholab/moose/blob/master/COPYRIGHT
//*
//* Licensed under LGPL 2.1, please see LICENSE for details
//* https://www.gnu.org/licenses/lgpl-2.1.html

#pragma once

#include "ADIntegratedBC.h"

class HybridizedDGAssemblyHelper;

/**
 * Base boundary condition for two-field hybridized DG discretizations assembled with automatic
 * differentiation.
 */
class HybridizedDGBC : public ADIntegratedBC
{
public:
  static InputParameters validParams();

  HybridizedDGBC(const InputParameters & parameters);

  virtual void computeResidual() override;
  virtual void computeJacobian() override;
  virtual void computeOffDiagJacobian(unsigned int jvar) override;
  virtual void computeResidualAndJacobian() override;
  virtual void jacobianSetup() override;
  virtual const std::unordered_set<unsigned int> & getMatPropDependencies() const override;
  virtual bool getMaterialPropertyCalled() const override;

protected:
  virtual void compute() = 0;

  virtual HybridizedDGAssemblyHelper & hybridizedDGHelper() = 0;
  const HybridizedDGAssemblyHelper & hybridizedDGHelper() const;

  virtual ADReal computeQpResidual() override { mooseError("this will never be called"); }

  const Elem * _cached_elem;
  unsigned int _cached_side;
};

inline const HybridizedDGAssemblyHelper &
HybridizedDGBC::hybridizedDGHelper() const
{
  return const_cast<HybridizedDGBC *>(this)->hybridizedDGHelper();
}
