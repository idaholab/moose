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
class GPUDirichletBC : public GPUDirichletBCBase<DirichletBC>
{
  usingGPUDirichletBCBaseMembers(DirichletBC);

public:
  static InputParameters validParams()
  {
    InputParameters params = GPUDirichletBCBase<DirichletBC>::validParams();
    params.addRequiredParam<Real>("value", "Value of the BC");
    params.declareControllable("value");
    return params;
  }

  GPUDirichletBC(const InputParameters & parameters)
    : GPUDirichletBCBase<DirichletBC>(parameters), _value(this->template getParam<Real>("value"))
  {
  }

  KOKKOS_FUNCTION Real computeValue(const dof_id_type /* node */) const { return _value; }

protected:
  GPUScalar<const Real> _value;
};

class GPUDirichletBCKernel final : public GPUDirichletBC<GPUDirichletBCKernel>
{
public:
  static InputParameters validParams();

  GPUDirichletBCKernel(const InputParameters & parameters);
};

#define GPUDirichletBCMembers(T)                                                                   \
  usingGPUDirichletBCBaseMembers(T);                                                               \
                                                                                                   \
protected:                                                                                         \
  using GPUDirichletBC<T>::_value;
