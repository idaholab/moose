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

class KokkosDirichletBC : public Moose::Kokkos::DirichletBCBase
{
public:
  static InputParameters validParams();

  KokkosDirichletBC(const InputParameters & parameters);

  KOKKOS_FUNCTION Real computeValue(const unsigned int /* qp */, AssemblyDatum & /* datum */) const
  {
    return _value;
  }

protected:
  const Moose::Kokkos::Scalar<const Real> _value;
};
