/****************************************************************/
/*               DO NOT MODIFY THIS HEADER                      */
/* MOOSE - Multiphysics Object Oriented Simulation Environment  */
/*                                                              */
/*           (c) 2010 Battelle Energy Alliance, LLC             */
/*                   ALL RIGHTS RESERVED                        */
/*                                                              */
/*          Prepared by Battelle Energy Alliance, LLC           */
/*            Under Contract No. DE-AC07-05ID14517              */
/*            With the U. S. Department of Energy               */
/*                                                              */
/*            See COPYRIGHT for full restrictions               */
/****************************************************************/

#include "InterfaceKernel.h"

// MOOSE includes
#include "Assembly.h"
#include "MooseVariable.h"
#include "SystemBase.h"

#include "libmesh/quadrature.h"

template <>
InputParameters
validParams<InterfaceKernel>()
{
  InputParameters params = validParams<DGKernel>();
  params.addRequiredCoupledVar("neighbor_var", "The variable on the other side of the interface.");
  params.set<std::string>("_moose_base") = "InterfaceKernel";
  params.addParam<std::vector<AuxVariableName>>(
      "save_in",
      "The name of auxiliary variables to save this Kernel's residual contributions to. "
      " Everything about that variable must match everything about this variable (the "
      "type, what blocks it's on, etc.)");
  params.addParam<std::vector<AuxVariableName>>(
      "diag_save_in",
      "The name of auxiliary variables to save this Kernel's diagonal Jacobian "
      "contributions to. Everything about that variable must match everything "
      "about this variable (the type, what blocks it's on, etc.)");

  MultiMooseEnum save_in_var_side("m s");
  params.addParam<MultiMooseEnum>(
      "save_in_var_side",
      save_in_var_side,
      "This parameter must exist if save_in variables are specified and must have the same length "
      "as save_in. This vector specifies whether the corresponding aux_var should save-in "
      "residual contributions from the master ('m') or slave side ('s').");
  params.addParam<MultiMooseEnum>(
      "diag_save_in_var_side",
      save_in_var_side,
      "This parameter must exist if diag_save_in variables are specified and must have the same "
      "length as diag_save_in. This vector specifies whether the corresponding aux_var should "
      "save-in jacobian contributions from the master ('m') or slave side ('s').");

  return params;
}

InterfaceKernel::InterfaceKernel(const InputParameters & params)
  : DGKernel(params),
    _neighbor_var(*getVar("neighbor_var", 0)),
    _neighbor_value(_neighbor_var.slnNeighbor()),
    _grad_neighbor_value(_neighbor_var.gradSlnNeighbor()),
    _save_in_var_side(params.get<MultiMooseEnum>("save_in_var_side")),
    _save_in_strings(params.get<std::vector<AuxVariableName>>("save_in")),
    _diag_save_in_var_side(params.get<MultiMooseEnum>("diag_save_in_var_side")),
    _diag_save_in_strings(params.get<std::vector<AuxVariableName>>("diag_save_in"))
{
  if (!params.isParamValid("boundary"))
    mooseError(
        "In order to use an interface kernel, you must specify a boundary where it will live.");

  if (params.isParamSetByUser("save_in"))
  {
    if (_save_in_strings.size() != _save_in_var_side.size())
      mooseError("save_in and save_in_var_side must be the same length");
    else
    {
      for (unsigned i = 0; i < _save_in_strings.size(); ++i)
      {
        MooseVariable * var = &_subproblem.getVariable(_tid, _save_in_strings[i]);

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

  if (params.isParamSetByUser("diag_save_in"))
  {
    if (_diag_save_in_strings.size() != _diag_save_in_var_side.size())
      mooseError("diag_save_in and diag_save_in_var_side must be the same length");
    else
    {
      for (unsigned i = 0; i < _diag_save_in_strings.size(); ++i)
      {
        MooseVariable * var = &_subproblem.getVariable(_tid, _diag_save_in_strings[i]);

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

const MooseVariable &
InterfaceKernel::neighborVariable() const
{
  return _neighbor_var;
}

void
InterfaceKernel::computeElemNeighResidual(Moose::DGResidualType type)
{
  bool is_elem;
  if (type == Moose::Element)
    is_elem = true;
  else
    is_elem = false;

  const VariableTestValue & test_space = is_elem ? _test : _test_neighbor;
  DenseVector<Number> & re = is_elem ? _assembly.residualBlock(_var.number())
                                     : _assembly.residualBlockNeighbor(_neighbor_var.number());

  _local_re.resize(re.size());
  _local_re.zero();

  for (_qp = 0; _qp < _qrule->n_points(); _qp++)
    for (_i = 0; _i < test_space.size(); _i++)
      _local_re(_i) += _JxW[_qp] * _coord[_qp] * computeQpResidual(type);

  re += _local_re;

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

void
InterfaceKernel::computeElemNeighJacobian(Moose::DGJacobianType type)
{
  const VariableTestValue & test_space =
      (type == Moose::ElementElement || type == Moose::ElementNeighbor) ? _test : _test_neighbor;
  const VariableTestValue & loc_phi =
      (type == Moose::ElementElement || type == Moose::NeighborElement) ? _phi : _phi_neighbor;
  DenseMatrix<Number> & Kxx =
      type == Moose::ElementElement
          ? _assembly.jacobianBlock(_var.number(), _var.number())
          : type == Moose::ElementNeighbor
                ? _assembly.jacobianBlockNeighbor(
                      Moose::ElementNeighbor, _var.number(), _neighbor_var.number())
                : type == Moose::NeighborElement
                      ? _assembly.jacobianBlockNeighbor(
                            Moose::NeighborElement, _neighbor_var.number(), _var.number())
                      : _assembly.jacobianBlockNeighbor(Moose::NeighborNeighbor,
                                                        _neighbor_var.number(),
                                                        _neighbor_var.number());

  _local_kxx.resize(Kxx.m(), Kxx.n());
  _local_kxx.zero();

  for (_qp = 0; _qp < _qrule->n_points(); _qp++)
    for (_i = 0; _i < test_space.size(); _i++)
      for (_j = 0; _j < loc_phi.size(); _j++)
        _local_kxx(_i, _j) += _JxW[_qp] * _coord[_qp] * computeQpJacobian(type);

  Kxx += _local_kxx;

  if (_has_master_jacobians_saved_in && type == Moose::ElementElement)
  {
    auto rows = _local_kxx.m();
    DenseVector<Number> diag(rows);
    for (decltype(rows) i = 0; i < rows; i++)
      diag(i) = _local_kxx(i, i);

    Threads::spin_mutex::scoped_lock lock(_jacoby_vars_mutex);
    for (const auto & var : _master_save_in_jacobian_variables)
      var->sys().solution().add_vector(diag, var->dofIndices());
  }
  else if (_has_slave_jacobians_saved_in && type == Moose::NeighborNeighbor)
  {
    auto rows = _local_kxx.m();
    DenseVector<Number> diag(rows);
    for (decltype(rows) i = 0; i < rows; i++)
      diag(i) = _local_kxx(i, i);

    Threads::spin_mutex::scoped_lock lock(_jacoby_vars_mutex);
    for (const auto & var : _slave_save_in_jacobian_variables)
      var->sys().solution().add_vector(diag, var->dofIndicesNeighbor());
  }
}

void
InterfaceKernel::computeJacobian()
{
  computeElemNeighJacobian(Moose::ElementElement);
  computeElemNeighJacobian(Moose::NeighborNeighbor);
}

void
InterfaceKernel::computeOffDiagElemNeighJacobian(Moose::DGJacobianType type, unsigned int jvar)
{
  const VariableTestValue & test_space =
      (type == Moose::ElementElement || type == Moose::ElementNeighbor) ? _test : _test_neighbor;
  const VariableTestValue & loc_phi =
      (type == Moose::ElementElement || type == Moose::NeighborElement) ? _phi : _phi_neighbor;
  DenseMatrix<Number> & Kxx =
      type == Moose::ElementElement
          ? _assembly.jacobianBlock(_var.number(), jvar)
          : type == Moose::ElementNeighbor
                ? _assembly.jacobianBlockNeighbor(Moose::ElementNeighbor, _var.number(), jvar)
                : type == Moose::NeighborElement
                      ? _assembly.jacobianBlockNeighbor(
                            Moose::NeighborElement, _neighbor_var.number(), jvar)
                      : _assembly.jacobianBlockNeighbor(
                            Moose::NeighborNeighbor, _neighbor_var.number(), jvar);

  // Prevent calling of Jacobian computation if jvar doesn't lie in the current block
  if ((Kxx.m() == test_space.size()) && (Kxx.n() == loc_phi.size()))
    for (_qp = 0; _qp < _qrule->n_points(); _qp++)
      for (_i = 0; _i < test_space.size(); _i++)
        for (_j = 0; _j < loc_phi.size(); _j++)
          Kxx(_i, _j) += _JxW[_qp] * _coord[_qp] * computeQpOffDiagJacobian(type, jvar);
}

void
InterfaceKernel::computeElementOffDiagJacobian(unsigned int jvar)
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

void
InterfaceKernel::computeNeighborOffDiagJacobian(unsigned int jvar)
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

Real
InterfaceKernel::computeQpOffDiagJacobian(Moose::DGJacobianType /*type*/, unsigned int /*jvar*/)
{
  return 0.;
}
