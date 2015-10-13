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

#include "NodalKernel.h"
#include "Problem.h"
#include "SubProblem.h"
#include "SystemBase.h"
#include "MooseVariable.h"

template<>
InputParameters validParams<NodalKernel>()
{
  InputParameters params = validParams<MooseObject>();
  params += validParams<TransientInterface>();
  params += validParams<BlockRestrictable>();
  params += validParams<RandomInterface>();

  params.addRequiredParam<NonlinearVariableName>("variable", "The name of the variable that this boundary condition applies to");

  params.addParam<std::vector<AuxVariableName> >("save_in", "The name of auxiliary variables to save this BC's residual contributions to.  Everything about that variable must match everything about this variable (the type, what blocks it's on, etc.)");

  params.addParam<std::vector<AuxVariableName> >("diag_save_in", "The name of auxiliary variables to save this BC's diagonal jacobian contributions to.  Everything about that variable must match everything about this variable (the type, what blocks it's on, etc.)");


  params.addParam<bool>("use_displaced_mesh", false, "Whether or not this object should use the displaced mesh for computation.  Note that in the case this is true but no displacements are provided in the Mesh block the undisplaced mesh will still be used.");
  params.addParamNamesToGroup("use_displaced_mesh", "Advanced");

  params.registerBase("NodalKernel");

  return params;
}

NodalKernel::NodalKernel(const InputParameters & parameters) :
    MooseObject(parameters),
    BlockRestrictable(parameters),
    SetupInterface(parameters),
    FunctionInterface(parameters),
    UserObjectInterface(parameters),
    TransientInterface(parameters, "bcs"),
    PostprocessorInterface(parameters),
    GeometricSearchInterface(parameters),
    Restartable(parameters, "BCs"),
    ZeroInterface(parameters),
    MeshChangedInterface(parameters),
    RandomInterface(parameters, *parameters.get<FEProblem *>("_fe_problem"), parameters.get<THREAD_ID>("_tid"), true),
    CoupleableMooseVariableDependencyIntermediateInterface(parameters, true),
    _subproblem(*parameters.get<SubProblem *>("_subproblem")),
    _fe_problem(*parameters.get<FEProblem *>("_fe_problem")),
    _sys(*parameters.get<SystemBase *>("_sys")),
    _tid(parameters.get<THREAD_ID>("_tid")),
    _assembly(_subproblem.assembly(_tid)),
    _var(_sys.getVariable(_tid, parameters.get<NonlinearVariableName>("variable"))),
    _mesh(_subproblem.mesh()),
    _current_node(_var.node()),
    _u(_var.nodalSln()),
    _u_dot(_var.nodalSlnDot()),
    _save_in_strings(parameters.get<std::vector<AuxVariableName> >("save_in")),
    _diag_save_in_strings(parameters.get<std::vector<AuxVariableName> >("diag_save_in"))

{
  _save_in.resize(_save_in_strings.size());
  _diag_save_in.resize(_diag_save_in_strings.size());

  for (unsigned int i=0; i<_save_in_strings.size(); i++)
  {
    MooseVariable * var = &_subproblem.getVariable(_tid, _save_in_strings[i]);

    if (var->feType() != _var.feType())
      mooseError("Error in " + name() + ". When saving residual values in an Auxiliary variable the AuxVariable must be the same type as the nonlinear variable the object is acting on.");

    _save_in[i] = var;
    var->sys().addVariableToZeroOnResidual(_save_in_strings[i]);
    addMooseVariableDependency(var);
  }

  _has_save_in = _save_in.size() > 0;

  for (unsigned int i=0; i<_diag_save_in_strings.size(); i++)
  {
    MooseVariable * var = &_subproblem.getVariable(_tid, _diag_save_in_strings[i]);

    if (var->feType() != _var.feType())
      mooseError("Error in " + name() + ". When saving diagonal Jacobian values in an Auxiliary variable the AuxVariable must be the same type as the nonlinear variable the object is acting on.");

    _diag_save_in[i] = var;
    var->sys().addVariableToZeroOnJacobian(_diag_save_in_strings[i]);
    addMooseVariableDependency(var);
  }

  _has_diag_save_in = _diag_save_in.size() > 0;
}

MooseVariable &
NodalKernel::variable()
{
  return _var;
}

SubProblem &
NodalKernel::subProblem()
{
  return _subproblem;
}

void
NodalKernel::computeResidual(NumericVector<Number> & residual)
{
  if (_var.isNodalDefined())
  {
    dof_id_type & dof_idx = _var.nodalDofIndex();
    _qp = 0;
    Real res = computeQpResidual();
    residual.add(dof_idx, res);

    if (_has_save_in)
    {
      Threads::spin_mutex::scoped_lock lock(Threads::spin_mtx);
      for (unsigned int i=0; i<_save_in.size(); i++)
        _save_in[i]->sys().solution().add(_save_in[i]->nodalDofIndex(), res);
    }
  }
}

void
NodalKernel::computeJacobian()
{/*
  // We call the user's computeQpJacobian() function and store the
  // results in the _assembly object. We can't store them directly in
  // the element stiffness matrix, as they will only be inserted after
  // all the assembly is done.
  if (_var.isNodalDefined())
  {
    _qp = 0;
    Real cached_val = computeQpJacobian();
    dof_id_type cached_row = _var.nodalDofIndex();

    // Cache the user's computeQpJacobian() value for later use.
    _fe_problem.assembly(0).cacheNodalBCJacobianEntry(cached_row, cached_row, cached_val);

    if (_has_diag_save_in)
    {
      Threads::spin_mutex::scoped_lock lock(Threads::spin_mtx);
      for (unsigned int i=0; i<_diag_save_in.size(); i++)
        _diag_save_in[i]->sys().solution().set(_diag_save_in[i]->nodalDofIndex(), cached_val);
    }
  }
 */
}

void
NodalKernel::computeOffDiagJacobian(unsigned int jvar)
{
  /*
  if (jvar == _var.number())
    computeJacobian();
  else
  {
    _qp = 0;
    Real cached_val = computeQpOffDiagJacobian(jvar);
    dof_id_type cached_row = _var.nodalDofIndex();
    // Note: this only works for Lagrange variables...
    dof_id_type cached_col = _current_node->dof_number(_sys.number(), jvar, 0);

    // Cache the user's computeQpJacobian() value for later use.
    _fe_problem.assembly(0).cacheNodalBCJacobianEntry(cached_row, cached_col, cached_val);
  }
  */
}


Real
NodalKernel::computeQpJacobian()
{
  return 0.;
}

Real
NodalKernel::computeQpOffDiagJacobian(unsigned int /*jvar*/)
{
  return 0.;
}
