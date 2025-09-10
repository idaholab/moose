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

template <typename DirichletBC>
class KokkosDirichletBC : public Moose::Kokkos::DirichletBCBase<DirichletBC>
{
  usingKokkosDirichletBCBaseMembers(DirichletBC);

public:
  static InputParameters validParams();

  KokkosDirichletBC(const InputParameters & parameters);

  KOKKOS_FUNCTION Real computeValue(const ContiguousNodeID /* node */) const { return _value; }

protected:
  const Moose::Kokkos::Scalar<const Real> _value;
};

template <typename DirichletBC>
InputParameters
KokkosDirichletBC<DirichletBC>::validParams()
{
  InputParameters params = Moose::Kokkos::DirichletBCBase<DirichletBC>::validParams();
  params.addRequiredParam<Real>("value", "Value of the BC");
  params.declareControllable("value");
  return params;
}

template <typename DirichletBC>
KokkosDirichletBC<DirichletBC>::KokkosDirichletBC(const InputParameters & parameters)
  : Moose::Kokkos::DirichletBCBase<DirichletBC>(parameters),
    _value(this->template getParam<Real>("value"))
{
}

class KokkosDirichletBCWrapper final : public KokkosDirichletBC<KokkosDirichletBCWrapper>
{
public:
  static InputParameters validParams();

  KokkosDirichletBCWrapper(const InputParameters & parameters);
};
