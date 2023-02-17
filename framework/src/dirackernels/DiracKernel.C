//* This file is part of the MOOSE framework
//* https://www.mooseframework.org
//*
//* All rights reserved, see COPYRIGHT for full restrictions
//* https://github.com/idaholab/moose/blob/master/COPYRIGHT
//*
//* Licensed under LGPL 2.1, please see LICENSE for details
//* https://www.gnu.org/licenses/lgpl-2.1.html

// Moose includes
#include "DiracKernel.h"
#include "Assembly.h"
#include "DiracKernelBase.h"
#include "MooseError.h"
#include "SystemBase.h"
#include "Problem.h"
#include "MooseMesh.h"

#include "libmesh/libmesh_common.h"
#include "libmesh/quadrature.h"

template <typename T>
InputParameters
DiracKernelTempl<T>::validParams()
{
  InputParameters params = DiracKernelBase::validParams();
  if (std::is_same<T, Real>::value)
    params.registerBase("DiracKernel");
  else if (std::is_same<T, RealVectorValue>::value)
    params.registerBase("VectorDiracKernel");
  else
    ::mooseError("unsupported DiracKernelTempl specialization");
  return params;
}

template <typename T>
DiracKernelTempl<T>::DiracKernelTempl(const InputParameters & parameters)
  : DiracKernelBase(parameters),
    MooseVariableInterface<T>(this,
                              false,
                              "variable",
                              Moose::VarKindType::VAR_NONLINEAR,
                              std::is_same<T, Real>::value ? Moose::VarFieldType::VAR_FIELD_STANDARD
                                                           : Moose::VarFieldType::VAR_FIELD_VECTOR),
    _var(this->mooseVariableField()),
    _phi(_assembly.phi(_var)),
    _grad_phi(_assembly.gradPhi(_var)),
    _test(_var.phi()),
    _grad_test(_var.gradPhi()),
    _u(_var.sln()),
    _grad_u(_var.gradSln()),
    _drop_duplicate_points(parameters.get<bool>("drop_duplicate_points")),
    _point_not_found_behavior(
        parameters.get<MooseEnum>("point_not_found_behavior").getEnum<PointNotFoundBehavior>())
{
  addMooseVariableDependency(&this->mooseVariableField());

  // Stateful material properties are not allowed on DiracKernels
  statefulPropertiesAllowed(false);
}

template <typename T>
void
DiracKernelTempl<T>::computeResidual()
{
  prepareVectorTag(_assembly, _var.number());

  const std::vector<unsigned int> * multiplicities =
      _drop_duplicate_points ? NULL : &_local_dirac_kernel_info.getPoints()[_current_elem].second;
  unsigned int local_qp = 0;
  Real multiplicity = 1.0;

  for (_qp = 0; _qp < _qrule->n_points(); _qp++)
  {
    _current_point = _physical_point[_qp];
    if (isActiveAtPoint(_current_elem, _current_point))
    {
      if (!_drop_duplicate_points)
        multiplicity = (*multiplicities)[local_qp++];

      for (_i = 0; _i < _test.size(); _i++)
        _local_re(_i) += multiplicity * computeQpResidual();
    }
  }

  accumulateTaggedLocalResidual();
}

template <typename T>
void
DiracKernelTempl<T>::computeJacobian()
{
  prepareMatrixTag(_assembly, _var.number(), _var.number());

  const std::vector<unsigned int> * multiplicities =
      _drop_duplicate_points ? NULL : &_local_dirac_kernel_info.getPoints()[_current_elem].second;
  unsigned int local_qp = 0;
  Real multiplicity = 1.0;

  for (_qp = 0; _qp < _qrule->n_points(); _qp++)
  {
    _current_point = _physical_point[_qp];
    if (isActiveAtPoint(_current_elem, _current_point))
    {
      if (!_drop_duplicate_points)
        multiplicity = (*multiplicities)[local_qp++];

      for (_i = 0; _i < _test.size(); _i++)
        for (_j = 0; _j < _phi.size(); _j++)
          _local_ke(_i, _j) += multiplicity * computeQpJacobian();
    }
  }

  accumulateTaggedLocalMatrix();
}

template <typename T>
void
DiracKernelTempl<T>::computeOffDiagJacobian(const unsigned int jvar_num)
{
  if (jvar_num == _var.number())
  {
    computeJacobian();
  }
  else
  {
    prepareMatrixTag(_assembly, _var.number(), jvar_num);

    const std::vector<unsigned int> * multiplicities =
        _drop_duplicate_points ? NULL : &_local_dirac_kernel_info.getPoints()[_current_elem].second;
    unsigned int local_qp = 0;
    Real multiplicity = 1.0;

    for (_qp = 0; _qp < _qrule->n_points(); _qp++)
    {
      _current_point = _physical_point[_qp];
      if (isActiveAtPoint(_current_elem, _current_point))
      {
        if (!_drop_duplicate_points)
          multiplicity = (*multiplicities)[local_qp++];

        for (_i = 0; _i < _test.size(); _i++)
          for (_j = 0; _j < _phi.size(); _j++)
            _local_ke(_i, _j) += multiplicity * computeQpOffDiagJacobian(jvar_num);
      }
    }

    accumulateTaggedLocalMatrix();
  }
}

template <typename T>
Real
DiracKernelTempl<T>::computeQpJacobian()
{
  return 0;
}

template <typename T>
Real
DiracKernelTempl<T>::computeQpOffDiagJacobian(unsigned int /*jvar*/)
{
  return 0;
}

// Explicitly instantiates the two versions of the DiracKernelTempl class
template class DiracKernelTempl<Real>;
template class DiracKernelTempl<RealVectorValue>;
