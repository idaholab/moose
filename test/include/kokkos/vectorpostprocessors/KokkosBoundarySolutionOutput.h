//* This file is part of the MOOSE framework
//* https://mooseframework.inl.gov
//*
//* All rights reserved, see COPYRIGHT for full restrictions
//* https://github.com/idaholab/moose/blob/master/COPYRIGHT
//*
//* Licensed under LGPL 2.1, please see LICENSE for details
//* https://www.gnu.org/licenses/lgpl-2.1.html

#pragma once

#include "KokkosSideVectorPostprocessor.h"

#include "MooseMesh.h"

class KokkosBoundarySolutionOutput : public Moose::Kokkos::SideVectorPostprocessor
{
public:
  static InputParameters validParams();

  KokkosBoundarySolutionOutput(const InputParameters & parameters);

  virtual void initialize() override;
  virtual void finalize() override;

  KOKKOS_FUNCTION void execute(Datum & datum) const;

private:
  const MooseMesh & _mesh;

  Moose::Kokkos::VariableValue _var;
  Moose::Kokkos::Array2D<Real> _solution;

  std::vector<Real> & _boundary_id_vec;
  std::vector<Real> & _solution_vec;
};

KOKKOS_FUNCTION inline void
KokkosBoundarySolutionOutput::execute(Datum & datum) const
{
  Real sum = 0;

  for (unsigned int qp = 0; qp < datum.n_qps(); ++qp)
  {
    datum.reinit();
    sum += datum.JxW(qp) * _var(datum, qp);
  }

  _solution(datum.side(), datum.elemID()) = sum;
}
