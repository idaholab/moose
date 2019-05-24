//* This file is part of the MOOSE framework
//* https://www.mooseframework.org
//*
//* All rights reserved, see COPYRIGHT for full restrictions
//* https://github.com/idaholab/moose/blob/master/COPYRIGHT
//*
//* Licensed under LGPL 2.1, please see LICENSE for details
//* https://www.gnu.org/licenses/lgpl-2.1.html

#include "InterfaceKernel.h"

// MOOSE includes
#include "Assembly.h"
#include "MooseVariableFE.h"
#include "SystemBase.h"

#include "libmesh/quadrature.h"

template <>
InputParameters
validParams<InterfaceKernel>()
{
  InputParameters params = validParams<InterfaceKernelBase>();
  params.registerBase("InterfaceKernel");
  return params;
}

template <>
InputParameters
validParams<VectorInterfaceKernel>()
{
  InputParameters params = validParams<InterfaceKernelBase>();
  params.registerBase("VectorInterfaceKernel");
  return params;
}

template <typename T>
InterfaceKernelTempl<T>::InterfaceKernelTempl(const InputParameters & parameters)
  : InterfaceKernelBase(parameters),
    NeighborMooseVariableInterface<T>(this,
                                      false,
                                      Moose::VarKindType::VAR_NONLINEAR,
                                      std::is_same<T, Real>::value
                                          ? Moose::VarFieldType::VAR_FIELD_STANDARD
                                          : Moose::VarFieldType::VAR_FIELD_VECTOR),
    _var(*this->mooseVariable()),
    _normals(_assembly.normals()),
    _u(_is_implicit ? _var.sln() : _var.slnOld()),
    _grad_u(_is_implicit ? _var.gradSln() : _var.gradSlnOld()),
    _phi(_assembly.phiFace(_var)),
    _grad_phi(_assembly.gradPhiFace(_var)),
    _test(_var.phiFace()),
    _grad_test(_var.gradPhiFace()),
    _neighbor_var(*getVarHelper<T>("neighbor_var", 0)),
    _neighbor_value(_is_implicit ? _neighbor_var.slnNeighbor() : _neighbor_var.slnOldNeighbor()),
    _grad_neighbor_value(_neighbor_var.gradSlnNeighbor()),
    _phi_neighbor(_assembly.phiFaceNeighbor(_neighbor_var)),
    _grad_phi_neighbor(_assembly.gradPhiFaceNeighbor(_neighbor_var)),
    _test_neighbor(_neighbor_var.phiFaceNeighbor()),
    _grad_test_neighbor(_neighbor_var.gradPhiFaceNeighbor())

{
  addMooseVariableDependency(this->mooseVariable());

  if (!parameters.isParamValid("boundary"))
    mooseError(
        "In order to use an interface kernel, you must specify a boundary where it will live.");

  if (parameters.isParamSetByUser("save_in"))
  {
    if (_save_in_strings.size() != _save_in_var_side.size())
      mooseError("save_in and save_in_var_side must be the same length");
    else
    {
      for (unsigned i = 0; i < _save_in_strings.size(); ++i)
      {
        MooseVariable * var = &_subproblem.getStandardVariable(_tid, _save_in_strings[i]);

        if (_sys.hasVariable(_save_in_strings[i]))
          mooseError("Trying to use solution variable " + _save_in_strings[i] +
                     " as a save_in variable in " + name());

        if (_save_in_var_side[i] == "m")
        {
          if (var->feType() != _var.feType())
            mooseError(
                "Error in " + name() +
                ". There is a mismatch between the fe_type of the save-in Auxiliary variable "
                "and the fe_type of the the master side nonlinear "
                "variable this interface kernel object is acting on.");
          _master_save_in_residual_variables.push_back(var);
        }
        else
        {
          if (var->feType() != _neighbor_var.feType())
            mooseError(
                "Error in " + name() +
                ". There is a mismatch between the fe_type of the save-in Auxiliary variable "
                "and the fe_type of the the slave side nonlinear "
                "variable this interface kernel object is acting on.");
          _slave_save_in_residual_variables.push_back(var);
        }

        var->sys().addVariableToZeroOnResidual(_save_in_strings[i]);
        addMooseVariableDependency(var);
      }
    }
  }

  _has_master_residuals_saved_in = _master_save_in_residual_variables.size() > 0;
  _has_slave_residuals_saved_in = _slave_save_in_residual_variables.size() > 0;

  if (parameters.isParamSetByUser("diag_save_in"))
  {
    if (_diag_save_in_strings.size() != _diag_save_in_var_side.size())
      mooseError("diag_save_in and diag_save_in_var_side must be the same length");
    else
    {
      for (unsigned i = 0; i < _diag_save_in_strings.size(); ++i)
      {
        MooseVariable * var = &_subproblem.getStandardVariable(_tid, _diag_save_in_strings[i]);

        if (_sys.hasVariable(_diag_save_in_strings[i]))
          mooseError("Trying to use solution variable " + _diag_save_in_strings[i] +
                     " as a save_in variable in " + name());

        if (_diag_save_in_var_side[i] == "m")
        {
          if (var->feType() != _var.feType())
            mooseError(
                "Error in " + name() +
                ". There is a mismatch between the fe_type of the save-in Auxiliary variable "
                "and the fe_type of the the master side nonlinear "
                "variable this interface kernel object is acting on.");
          _master_save_in_jacobian_variables.push_back(var);
        }
        else
        {
          if (var->feType() != _neighbor_var.feType())
            mooseError(
                "Error in " + name() +
                ". There is a mismatch between the fe_type of the save-in Auxiliary variable "
                "and the fe_type of the the slave side nonlinear "
                "variable this interface kernel object is acting on.");
          _slave_save_in_jacobian_variables.push_back(var);
        }

        var->sys().addVariableToZeroOnJacobian(_diag_save_in_strings[i]);
        addMooseVariableDependency(var);
      }
    }
  }

  _has_master_jacobians_saved_in = _master_save_in_jacobian_variables.size() > 0;
  _has_slave_jacobians_saved_in = _slave_save_in_jacobian_variables.size() > 0;
}

template <typename T>
void
InterfaceKernelTempl<T>::computeElemNeighResidual(Moose::DGResidualType type)
{
  bool is_elem;
  if (type == Moose::Element)
    is_elem = true;
  else
    is_elem = false;

  const TemplateVariableTestValue & test_space = is_elem ? _test : _test_neighbor;

  if (is_elem)
    prepareVectorTag(_assembly, _var.number());
  else
    prepareVectorTagNeighbor(_assembly, _neighbor_var.number());

  for (_qp = 0; _qp < _qrule->n_points(); _qp++)
    for (_i = 0; _i < test_space.size(); _i++)
      _local_re(_i) += _JxW[_qp] * _coord[_qp] * computeQpResidual(type);

  accumulateTaggedLocalResidual();

  if (_has_master_residuals_saved_in && is_elem)
  {
    Threads::spin_mutex::scoped_lock lock(_resid_vars_mutex);
    for (const auto & var : _master_save_in_residual_variables)
    {
      var->sys().solution().add_vector(_local_re, var->dofIndices());
    }
  }
  else if (_has_slave_residuals_saved_in && !is_elem)
  {
    Threads::spin_mutex::scoped_lock lock(_resid_vars_mutex);
    for (const auto & var : _slave_save_in_residual_variables)
      var->sys().solution().add_vector(_local_re, var->dofIndicesNeighbor());
  }
}

template <typename T>
void
InterfaceKernelTempl<T>::computeResidual()
{
  // Compute the residual for this element
  computeElemNeighResidual(Moose::Element);

  // Compute the residual for the neighbor
  computeElemNeighResidual(Moose::Neighbor);
}

template <typename T>
void
InterfaceKernelTempl<T>::computeElemNeighJacobian(Moose::DGJacobianType type)
{
  const TemplateVariableTestValue & test_space =
      (type == Moose::ElementElement || type == Moose::ElementNeighbor) ? _test : _test_neighbor;
  const TemplateVariableTestValue & loc_phi =
      (type == Moose::ElementElement || type == Moose::NeighborElement) ? _phi : _phi_neighbor;

  unsigned int ivar, jvar;

  switch (type)
  {
    case Moose::ElementElement:
      ivar = jvar = _var.number();
      break;
    case Moose::ElementNeighbor:
      ivar = _var.number(), jvar = _neighbor_var.number();
      break;
    case Moose::NeighborElement:
      ivar = _neighbor_var.number(), jvar = _var.number();
      break;
    case Moose::NeighborNeighbor:
      ivar = _neighbor_var.number(), jvar = _neighbor_var.number();
      break;
    default:
      mooseError("Unknown DGJacobianType ", type);
  }

  if (type == Moose::ElementElement)
    prepareMatrixTag(_assembly, ivar, jvar);
  else
    prepareMatrixTagNeighbor(_assembly, ivar, jvar, type);

  for (_qp = 0; _qp < _qrule->n_points(); _qp++)
    for (_i = 0; _i < test_space.size(); _i++)
      for (_j = 0; _j < loc_phi.size(); _j++)
        _local_ke(_i, _j) += _JxW[_qp] * _coord[_qp] * computeQpJacobian(type);

  accumulateTaggedLocalMatrix();

  if (_has_master_jacobians_saved_in && type == Moose::ElementElement)
  {
    auto rows = _local_ke.m();
    DenseVector<Number> diag(rows);
    for (decltype(rows) i = 0; i < rows; i++)
      diag(i) = _local_ke(i, i);

    Threads::spin_mutex::scoped_lock lock(_jacoby_vars_mutex);
    for (const auto & var : _master_save_in_jacobian_variables)
      var->sys().solution().add_vector(diag, var->dofIndices());
  }
  else if (_has_slave_jacobians_saved_in && type == Moose::NeighborNeighbor)
  {
    auto rows = _local_ke.m();
    DenseVector<Number> diag(rows);
    for (decltype(rows) i = 0; i < rows; i++)
      diag(i) = _local_ke(i, i);

    Threads::spin_mutex::scoped_lock lock(_jacoby_vars_mutex);
    for (const auto & var : _slave_save_in_jacobian_variables)
      var->sys().solution().add_vector(diag, var->dofIndicesNeighbor());
  }
}

template <typename T>
void
InterfaceKernelTempl<T>::computeJacobian()
{
  computeElemNeighJacobian(Moose::ElementElement);
  computeElemNeighJacobian(Moose::NeighborNeighbor);
}

template <typename T>
void
InterfaceKernelTempl<T>::computeOffDiagElemNeighJacobian(Moose::DGJacobianType type,
                                                         unsigned int jvar)
{
  const TemplateVariableTestValue & test_space =
      (type == Moose::ElementElement || type == Moose::ElementNeighbor) ? _test : _test_neighbor;
  const TemplateVariableTestValue & loc_phi =
      (type == Moose::ElementElement || type == Moose::NeighborElement) ? _phi : _phi_neighbor;

  unsigned int ivar;

  if (type == Moose::ElementElement || type == Moose::ElementNeighbor)
    ivar = _var.number();
  else
    ivar = _neighbor_var.number();

  if (type == Moose::ElementElement)
    prepareMatrixTag(_assembly, ivar, jvar);
  else
    prepareMatrixTagNeighbor(_assembly, ivar, jvar, type);

  // Prevent calling of Jacobian computation if jvar doesn't lie in the current block
  if ((_local_ke.m() == test_space.size()) && (_local_ke.n() == loc_phi.size()))
    for (_qp = 0; _qp < _qrule->n_points(); _qp++)
      for (_i = 0; _i < test_space.size(); _i++)
        for (_j = 0; _j < loc_phi.size(); _j++)
          _local_ke(_i, _j) += _JxW[_qp] * _coord[_qp] * computeQpOffDiagJacobian(type, jvar);

  accumulateTaggedLocalMatrix();
}

template <typename T>
void
InterfaceKernelTempl<T>::computeElementOffDiagJacobian(unsigned int jvar)
{
  bool is_jvar_not_interface_var = true;
  if (jvar == _var.number())
  {
    computeElemNeighJacobian(Moose::ElementElement);
    is_jvar_not_interface_var = false;
  }
  if (jvar == _neighbor_var.number())
  {
    computeElemNeighJacobian(Moose::ElementNeighbor);
    is_jvar_not_interface_var = false;
  }

  if (is_jvar_not_interface_var)
  {
    computeOffDiagElemNeighJacobian(Moose::ElementElement, jvar);
    computeOffDiagElemNeighJacobian(Moose::ElementNeighbor, jvar);
  }
}

template <typename T>
void
InterfaceKernelTempl<T>::computeNeighborOffDiagJacobian(unsigned int jvar)
{
  bool is_jvar_not_interface_var = true;
  if (jvar == _var.number())
  {
    computeElemNeighJacobian(Moose::NeighborElement);
    is_jvar_not_interface_var = false;
  }
  if (jvar == _neighbor_var.number())
  {
    computeElemNeighJacobian(Moose::NeighborNeighbor);
    is_jvar_not_interface_var = false;
  }

  if (is_jvar_not_interface_var)
  {
    computeOffDiagElemNeighJacobian(Moose::NeighborElement, jvar);
    computeOffDiagElemNeighJacobian(Moose::NeighborNeighbor, jvar);
  }
}

// Explicitly instantiates the two versions of the InterfaceKernelTempl class
template class InterfaceKernelTempl<Real>;
template class InterfaceKernelTempl<RealVectorValue>;
