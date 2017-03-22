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

#include "KernelBase.h"
#include "Assembly.h"
#include "MooseVariable.h"
#include "Problem.h"
#include "SubProblem.h"
#include "SystemBase.h"
#include "NonlinearSystem.h"

// libmesh includes
#include "libmesh/threads.h"

template <>
InputParameters
validParams<KernelBase>()
{
  InputParameters params = validParams<MooseObject>();
  params += validParams<TransientInterface>();
  params += validParams<BlockRestrictable>();
  params += validParams<RandomInterface>();
  params += validParams<MeshChangedInterface>();
  params += validParams<MaterialPropertyInterface>();

  params.addRequiredParam<NonlinearVariableName>(
      "variable", "The name of the variable that this Kernel operates on");
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
  params.addParam<bool>(
      "eigen_kernel", false, "Whether or not this kernel will be used as an eigen kernel");
  params.addParam<bool>("use_displaced_mesh",
                        false,
                        "Whether or not this object should use the "
                        "displaced mesh for computation. Note that in "
                        "the case this is true but no displacements "
                        "are provided in the Mesh block the "
                        "undisplaced mesh will still be used.");
  params.addParamNamesToGroup("use_displaced_mesh", "Advanced");

  params.addParamNamesToGroup("diag_save_in save_in", "Advanced");

  params.declareControllable("enable");
  return params;
}

KernelBase::KernelBase(const InputParameters & parameters)
  : MooseObject(parameters),
    BlockRestrictable(parameters),
    SetupInterface(this),
    CoupleableMooseVariableDependencyIntermediateInterface(this, false),
    FunctionInterface(this),
    UserObjectInterface(this),
    TransientInterface(this),
    PostprocessorInterface(this),
    VectorPostprocessorInterface(this),
    MaterialPropertyInterface(this, blockIDs()),
    RandomInterface(parameters,
                    *parameters.get<FEProblemBase *>("_fe_problem_base"),
                    parameters.get<THREAD_ID>("_tid"),
                    false),
    GeometricSearchInterface(this),
    Restartable(parameters, "Kernels"),
    ZeroInterface(parameters),
    MeshChangedInterface(parameters),
    _subproblem(*parameters.get<SubProblem *>("_subproblem")),
    _fe_problem(*parameters.get<FEProblemBase *>("_fe_problem_base")),
    _sys(*parameters.get<SystemBase *>("_sys")),
    _tid(parameters.get<THREAD_ID>("_tid")),
    _assembly(_subproblem.assembly(_tid)),
    _var(_sys.getVariable(_tid, parameters.get<NonlinearVariableName>("variable"))),
    _mesh(_subproblem.mesh()),
    _current_elem(_var.currentElem()),
    _current_elem_volume(_assembly.elemVolume()),
    _q_point(_assembly.qPoints()),
    _qrule(_assembly.qRule()),
    _JxW(_assembly.JxW()),
    _coord(_assembly.coordTransformation()),

    _test(_var.phi()),
    _grad_test(_var.gradPhi()),

    _phi(_assembly.phi()),
    _grad_phi(_assembly.gradPhi()),

    _save_in_strings(parameters.get<std::vector<AuxVariableName>>("save_in")),
    _diag_save_in_strings(parameters.get<std::vector<AuxVariableName>>("diag_save_in")),

    _eigen_kernel(getParam<bool>("eigen_kernel"))
{
  _save_in.resize(_save_in_strings.size());
  _diag_save_in.resize(_diag_save_in_strings.size());

  for (unsigned int i = 0; i < _save_in_strings.size(); i++)
  {
    MooseVariable * var = &_subproblem.getVariable(_tid, _save_in_strings[i]);

    if (_fe_problem.getNonlinearSystemBase().hasVariable(_save_in_strings[i]))
      mooseError("Trying to use solution variable " + _save_in_strings[i] +
                 " as a save_in variable in " + name());

    if (var->feType() != _var.feType())
      mooseError("Error in " + name() + ". When saving residual values in an Auxiliary variable "
                                        "the AuxVariable must be the same type as the nonlinear "
                                        "variable the object is acting on.");

    _save_in[i] = var;
    var->sys().addVariableToZeroOnResidual(_save_in_strings[i]);
    addMooseVariableDependency(var);
  }

  _has_save_in = _save_in.size() > 0;

  for (unsigned int i = 0; i < _diag_save_in_strings.size(); i++)
  {
    MooseVariable * var = &_subproblem.getVariable(_tid, _diag_save_in_strings[i]);

    if (_fe_problem.getNonlinearSystemBase().hasVariable(_diag_save_in_strings[i]))
      mooseError("Trying to use solution variable " + _diag_save_in_strings[i] +
                 " as a diag_save_in variable in " + name());

    if (var->feType() != _var.feType())
      mooseError("Error in " + name() + ". When saving diagonal Jacobian values in an Auxiliary "
                                        "variable the AuxVariable must be the same type as the "
                                        "nonlinear variable the object is acting on.");

    _diag_save_in[i] = var;
    var->sys().addVariableToZeroOnJacobian(_diag_save_in_strings[i]);
    addMooseVariableDependency(var);
  }

  _has_diag_save_in = _diag_save_in.size() > 0;
}

KernelBase::~KernelBase() {}

MooseVariable &
KernelBase::variable()
{
  return _var;
}

SubProblem &
KernelBase::subProblem()
{
  return _subproblem;
}
