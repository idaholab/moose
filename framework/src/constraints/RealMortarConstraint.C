//* This file is part of the MOOSE framework
//* https://www.mooseframework.org
//*
//* All rights reserved, see COPYRIGHT for full restrictions
//* https://github.com/idaholab/moose/blob/master/COPYRIGHT
//*
//* Licensed under LGPL 2.1, please see LICENSE for details
//* https://www.gnu.org/licenses/lgpl-2.1.html

#include "RealMortarConstraint.h"

// MOOSE includes
#include "Assembly.h"
#include "FEProblem.h"
#include "MooseVariable.h"
#include "NearestNodeLocator.h"
#include "PenetrationLocator.h"

#include "libmesh/quadrature.h"

template <>
InputParameters
validParams<RealMortarConstraint>()
{
  InputParameters params = validParams<Constraint>();
  params.addRequiredParam<unsigned>("master_id", "The id of the master boundary sideset.");
  params.addRequiredParam<unsigned>("slave_id", "The id of the slave boundary sideset.");
  params.registerRelationshipManagers("AugmentSparsityOnInterface");
  params.addRequiredParam<VariableName>("master_variable", "Variable on master surface");
  params.addParam<VariableName>("slave_variable", "Variable on master surface");
  return params;
}

RealMortarConstraint::RealMortarConstraint(const InputParameters & parameters)
  : Constraint(parameters),
    CoupleableMooseVariableDependencyIntermediateInterface(this, true),
    MooseVariableInterface<Real>(this,
                                 true,
                                 "variable",
                                 Moose::VarKindType::VAR_NONLINEAR,
                                 Moose::VarFieldType::VAR_FIELD_STANDARD),
    _fe_problem(*getCheckedPointerParam<FEProblemBase *>("_fe_problem_base")),
    _dim(_mesh.dimension()),

    _q_point(_assembly.qPoints()),
    _qrule(_assembly.qRule()),
    _JxW(_assembly.JxW()),
    _coord(_assembly.coordTransformation()),
    _current_elem(_assembly.elem()),

    _master_var(_subproblem.getStandardVariable(_tid, getParam<VariableName>("master_variable"))),
    _slave_var(
        isParamValid("slave_variable")
            ? _subproblem.getStandardVariable(_tid, getParam<VariableName>("slave_variable"))
            : _subproblem.getStandardVariable(_tid, getParam<VariableName>("master_variable"))),
    _lambda(_var.sln()),

    _test_master(_master_var.phi()),
    _grad_test_master(_master_var.gradPhi()),
    _phi_master(_master_var.phi()),

    _test_slave(_slave_var.phi()),
    _grad_test_slave(_slave_var.gradPhi()),
    _phi_slave(_slave_var.phi()),
    _slave_id(getParam<unsigned>("slave_id")),
    _master_id(getParam<unsigned>("master_id")),
    _amg(_fe_problem.getMortarInterface(std::make_pair(_slave_id, _master_id)))
{
}

void
RealMortarConstraint::computeResidual()
{
  DenseVector<Number> & re = _assembly.residualBlock(_var.number());
  for (_qp = 0; _qp < _qrule->n_points(); _qp++)
    for (_i = 0; _i < _test.size(); _i++)
      re(_i) += _JxW_lm[_qp] * _coord[_qp] * computeQpResidual();
}

void
RealMortarConstraint::computeJacobian()
{
  _phi = _assembly.getFE(_var.feType(), _dim - 1)->get_phi(); // yes we need to do a copy here
  std::vector<std::vector<Real>> phi_master;
  std::vector<std::vector<Real>> phi_slave;

  DenseMatrix<Number> & Kee = _assembly.jacobianBlock(_var.number(), _var.number());

  for (_qp = 0; _qp < _qrule->n_points(); _qp++)
    for (_i = 0; _i < _test.size(); _i++)
      for (_j = 0; _j < _phi.size(); _j++)
        Kee(_i, _j) += _JxW_lm[_qp] * _coord[_qp] * computeQpJacobian();
}

Real
RealMortarConstraint::computeQpJacobian()
{
  return 0.;
}
