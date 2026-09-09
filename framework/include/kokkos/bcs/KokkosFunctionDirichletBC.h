//* This file is part of the MOOSE framework
//* https://www.mooseframework.org
//*
//* All rights reserved, see COPYRIGHT for full restrictions
//* https://github.com/idaholab/moose/blob/master/COPYRIGHT
//*
//* Licensed under LGPL 2.1, please see LICENSE for details
//* https://www.gnu.org/licenses/lgpl-2.1.html

#pragma once

#include "KokkosDirichletBCBase.h"
#include "KokkosFunction.h"

template <bool is_ad>
class KokkosFunctionDirichletBCTempl : public Moose::Kokkos::DirichletBCBaseTempl<is_ad>
{
public:
  static InputParameters validParams();

  KokkosFunctionDirichletBCTempl(const InputParameters & parameters);

  KOKKOS_FUNCTION Real computeValue(const unsigned int qp, AssemblyDatum & datum) const
  {
    return _function.value(this->_t, datum.q_point(qp));
  }

  virtual Real hostValue(const libMesh::Point & p, Real time) const override
  {
    return _function.hostValue(time, Moose::Kokkos::Real3(p(0), p(1), p(2)));
  }

protected:
  /// Function evaluator: computeValue() reads it through its device dispatch on a mesh node,
  /// hostValue() reads it directly on host at a point libMesh's constraint machinery reports
  /// beyond a mesh node (e.g. a HIERARCHIC edge or face mode)
  const Moose::Kokkos::Function _function;
};

typedef KokkosFunctionDirichletBCTempl<false> KokkosFunctionDirichletBC;
typedef KokkosFunctionDirichletBCTempl<true> KokkosADFunctionDirichletBC;
