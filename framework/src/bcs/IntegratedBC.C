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

#include "IntegratedBC.h"
#include "SubProblem.h"
#include "SystemBase.h"
#include "MooseVariable.h"


template<>
InputParameters validParams<IntegratedBC>()
{
  InputParameters params = validParams<BoundaryCondition>();
  params += validParams<RandomInterface>();

  params.addParam<std::vector<AuxVariableName> >("save_in", "The name of auxiliary variables to save this Kernel's residual contributions to.  Everything about that variable must match everything about this variable (the type, what blocks it's on, etc.)");
  params.addParam<std::vector<AuxVariableName> >("diag_save_in", "The name of auxiliary variables to save this Kernel's diagonal jacobian contributions to.  Everything about that variable must match everything about this variable (the type, what blocks it's on, etc.)");

  params.addParamNamesToGroup("diag_save_in save_in", "Advanced");

  return params;
}

IntegratedBC::IntegratedBC(const std::string & name, InputParameters parameters) :
    BoundaryCondition(name, parameters),
    RandomInterface(name, parameters, _fe_problem, _tid, false),
    CoupleableMooseVariableDependencyIntermediateInterface(parameters, false),
    MaterialPropertyInterface(parameters),
    _current_elem(_assembly.elem()),
    _current_elem_volume(_assembly.elemVolume()),
    _current_side(_assembly.side()),
    _current_side_elem(_assembly.sideElem()),
    _current_side_volume(_assembly.sideElemVolume()),

    _normals(_var.normals()),

    _qrule(_assembly.qRuleFace()),
    _q_point(_assembly.qPointsFace()),
    _JxW(_assembly.JxWFace()),
    _coord(_assembly.coordTransformation()),

    _phi(_assembly.phiFace()),
    _grad_phi(_assembly.gradPhiFace()),

    _test(_var.phiFace()),
    _grad_test(_var.gradPhiFace()),

    _u(_is_implicit ? _var.sln() : _var.slnOld()),
    _grad_u(_is_implicit ? _var.gradSln() : _var.gradSlnOld()),

    _save_in_strings(parameters.get<std::vector<AuxVariableName> >("save_in")),
    _diag_save_in_strings(parameters.get<std::vector<AuxVariableName> >("diag_save_in"))
{
  _save_in.resize(_save_in_strings.size());
  _diag_save_in.resize(_diag_save_in_strings.size());

  for(unsigned int i=0; i<_save_in_strings.size(); i++)
  {
    MooseVariable * var = &_subproblem.getVariable(_tid, _save_in_strings[i]);

    if (var->feType() != _var.feType())
      mooseError("Error in " + _name + ". When saving residual values in an Auxiliary variable the AuxVariable must be the same type as the nonlinear variable the object is acting on.");

    _save_in[i] = var;
    var->sys().addVariableToZeroOnResidual(_save_in_strings[i]);
    addMooseVariableDependency(var);
  }

  _has_save_in = _save_in.size() > 0;

  for(unsigned int i=0; i<_diag_save_in_strings.size(); i++)
  {
    MooseVariable * var = &_subproblem.getVariable(_tid, _diag_save_in_strings[i]);

    if (var->feType() != _var.feType())
      mooseError("Error in " + _name + ". When saving diagonal Jacobian values in an Auxiliary variable the AuxVariable must be the same type as the nonlinear variable the object is acting on.");

    _diag_save_in[i] = var;
    var->sys().addVariableToZeroOnJacobian(_diag_save_in_strings[i]);
    addMooseVariableDependency(var);
  }

  _has_diag_save_in = _diag_save_in.size() > 0;
}

IntegratedBC::~IntegratedBC()
{
}

void
IntegratedBC::computeResidual()
{
  DenseVector<Number> & re = _assembly.residualBlock(_var.index());
  _local_re.resize(re.size());
  _local_re.zero();

  for (_qp = 0; _qp < _qrule->n_points(); _qp++)
    for (_i = 0; _i < _test.size(); _i++)
      _local_re(_i) += _JxW[_qp]*_coord[_qp]*computeQpResidual();

  re += _local_re;

  if (_has_save_in)
  {
    Threads::spin_mutex::scoped_lock lock(Threads::spin_mtx);
    for(unsigned int i=0; i<_save_in.size(); i++)
      _save_in[i]->sys().solution().add_vector(_local_re, _save_in[i]->dofIndices());
  }
}

void
IntegratedBC::computeJacobian()
{
  DenseMatrix<Number> & ke = _assembly.jacobianBlock(_var.index(), _var.index());
  _local_ke.resize(ke.m(), ke.n());
  _local_ke.zero();

  for (_qp = 0; _qp < _qrule->n_points(); _qp++)
    for (_i = 0; _i < _test.size(); _i++)
      for (_j = 0; _j < _phi.size(); _j++)
        _local_ke(_i, _j) += _JxW[_qp]*_coord[_qp]*computeQpJacobian();

  ke += _local_ke;

  if (_has_diag_save_in)
  {
    unsigned int rows = ke.m();
    DenseVector<Number> diag(rows);
    for(unsigned int i=0; i<rows; i++)
      diag(i) = _local_ke(i,i);

    Threads::spin_mutex::scoped_lock lock(Threads::spin_mtx);
    for(unsigned int i=0; i<_diag_save_in.size(); i++)
      _diag_save_in[i]->sys().solution().add_vector(diag, _diag_save_in[i]->dofIndices());
  }
}

void
IntegratedBC::computeJacobianBlock(unsigned int jvar)
{
//  Moose::perf_log.push("computeJacobianBlock()","IntegratedBC");

  DenseMatrix<Number> & ke = _assembly.jacobianBlock(_var.index(), jvar);

  for (_qp=0; _qp<_qrule->n_points(); _qp++)
    for (_i=0; _i<_test.size(); _i++)
      for (_j=0; _j<_phi.size(); _j++)
      {
        if (_var.index() == jvar)
          ke(_i,_j) += _JxW[_qp]*_coord[_qp]*computeQpJacobian();
        else
          ke(_i,_j) += _JxW[_qp]*_coord[_qp]*computeQpOffDiagJacobian(jvar);
      }

//  Moose::perf_log.pop("computeJacobianBlock()","IntegratedBC");
}

void
IntegratedBC::computeJacobianBlockScalar(unsigned int jvar)
{
  DenseMatrix<Number> & ke = _assembly.jacobianBlock(_var.index(), jvar);

  MooseVariableScalar & jv = _sys.getScalarVariable(_tid, jvar);
  for (_qp = 0; _qp < _qrule->n_points(); _qp++)
    for (_i = 0; _i < _test.size(); _i++)
      for (_j = 0; _j < jv.order(); _j++)
        ke(_i, _j) += _JxW[_qp] * _coord[_qp] * computeQpOffDiagJacobian(jvar);
}

Real
IntegratedBC::computeQpJacobian()
{
  return 0;
}

Real
IntegratedBC::computeQpOffDiagJacobian(unsigned int /*jvar*/)
{
  return 0;
}
