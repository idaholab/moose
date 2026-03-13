//* This file is part of the MOOSE framework
//* https://mooseframework.inl.gov
//*
//* All rights reserved, see COPYRIGHT for full restrictions
//* https://github.com/idaholab/moose/blob/master/COPYRIGHT
//*
//* Licensed under LGPL 2.1, please see LICENSE for details
//* https://www.gnu.org/licenses/lgpl-2.1.html

#pragma once

#include "KokkosNodalVectorPostprocessor.h"

#include "MooseMesh.h"

class KokkosNodalSolutionOutput : public Moose::Kokkos::NodalVectorPostprocessor
{
public:
  static InputParameters validParams();

  KokkosNodalSolutionOutput(const InputParameters & parameters);

  virtual void initialize() override;
  virtual void finalize() override;

  KOKKOS_FUNCTION void execute(Datum & datum) const;

private:
  MooseMesh & _mesh;

  Moose::Kokkos::VariableValue _var;
  Moose::Kokkos::Array<Real> _solution;

  std::vector<Real> & _block_id_vec;
  std::vector<Real> & _solution_vec;
};

KOKKOS_FUNCTION inline void
KokkosNodalSolutionOutput::execute(Datum & datum) const
{
  _solution[datum.node()] = _var(datum, 0);
}
