//* This file is part of the MOOSE framework
//* https://www.mooseframework.org
//*
//* All rights reserved, see COPYRIGHT for full restrictions
//* https://github.com/idaholab/moose/blob/master/COPYRIGHT
//*
//* Licensed under LGPL 2.1, please see LICENSE for details
//* https://www.gnu.org/licenses/lgpl-2.1.html

#pragma once

#include "GPUDirichletBCBase.h"

template <typename DirichletBC>
class KokkosDirichletBC : public Moose::Kokkos::DirichletBCBase<DirichletBC>
{
  usingKokkosDirichletBCBaseMembers(DirichletBC);

public:
  static InputParameters validParams()
  {
    InputParameters params = Moose::Kokkos::DirichletBCBase<DirichletBC>::validParams();
    params.addRequiredParam<Real>("value", "Value of the BC");
    params.declareControllable("value");
    return params;
  }

  KokkosDirichletBC(const InputParameters & parameters)
    : Moose::Kokkos::DirichletBCBase<DirichletBC>(parameters),
      _value(this->template getParam<Real>("value"))
  {
  }

  KOKKOS_FUNCTION Real computeValue(const dof_id_type /* node */) const { return _value; }

protected:
  Moose::Kokkos::Scalar<const Real> _value;
};

class KokkosDirichletBCKernel final : public KokkosDirichletBC<KokkosDirichletBCKernel>
{
public:
  static InputParameters validParams();

  KokkosDirichletBCKernel(const InputParameters & parameters);
};

#define usingKokkosDirichletBCMembers(T)                                                           \
  usingKokkosDirichletBCBaseMembers(T);                                                            \
                                                                                                   \
protected:                                                                                         \
  using KokkosDirichletBC<T>::_value;
