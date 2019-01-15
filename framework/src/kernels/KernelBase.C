//* This file is part of the MOOSE framework
//* https://www.mooseframework.org
//*
//* All rights reserved, see COPYRIGHT for full restrictions
//* https://github.com/idaholab/moose/blob/master/COPYRIGHT
//*
//* Licensed under LGPL 2.1, please see LICENSE for details
//* https://www.gnu.org/licenses/lgpl-2.1.html

#include "KernelBase.h"
#include "Assembly.h"
#include "MooseVariableFE.h"
#include "Problem.h"
#include "SubProblem.h"
#include "SystemBase.h"
#include "NonlinearSystem.h"

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
  params += validParams<TaggingInterface>();

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

  params.addParam<bool>("use_displaced_mesh",
                        false,
                        "Whether or not this object should use the "
                        "displaced mesh for computation. Note that in "
                        "the case this is true but no displacements "
                        "are provided in the Mesh block the "
                        "undisplaced mesh will still be used.");
  params.addParamNamesToGroup(" diag_save_in save_in use_displaced_mesh", "Advanced");
  params.addCoupledVar("displacements", "The displacements");

  params.declareControllable("enable");
  return params;
}

KernelBase::KernelBase(const InputParameters & parameters)
  : MooseObject(parameters),
    BlockRestrictable(this),
    SetupInterface(this),
    CoupleableMooseVariableDependencyIntermediateInterface(this, false),
    FunctionInterface(this),
    UserObjectInterface(this),
    TransientInterface(this),
    PostprocessorInterface(this),
    VectorPostprocessorInterface(this),
    MaterialPropertyInterface(this, blockIDs(), Moose::EMPTY_BOUNDARY_IDS),
    RandomInterface(parameters,
                    *parameters.getCheckedPointerParam<FEProblemBase *>("_fe_problem_base"),
                    parameters.get<THREAD_ID>("_tid"),
                    false),
    GeometricSearchInterface(this),
    Restartable(this, "Kernels"),
    MeshChangedInterface(parameters),
    TaggingInterface(this),
    _subproblem(*getCheckedPointerParam<SubProblem *>("_subproblem")),
    _fe_problem(*parameters.get<FEProblemBase *>("_fe_problem_base")),
    _sys(*getCheckedPointerParam<SystemBase *>("_sys")),
    _tid(parameters.get<THREAD_ID>("_tid")),
    _assembly(_subproblem.assembly(_tid)),
    _mesh(_subproblem.mesh()),
    _current_elem(_assembly.elem()),
    _current_elem_volume(_assembly.elemVolume()),
    _q_point(_assembly.qPoints()),
    _qrule(_assembly.qRule()),
    _JxW(_assembly.JxW()),
    _coord(_assembly.coordTransformation()),
    _has_save_in(false),
    _save_in_strings(parameters.get<std::vector<AuxVariableName>>("save_in")),
    _has_diag_save_in(false),
    _diag_save_in_strings(parameters.get<std::vector<AuxVariableName>>("diag_save_in"))
{
  auto num_disp = coupledComponents("displacements");
  for (decltype(num_disp) i = 0; i < num_disp; ++i)
    _displacements.push_back(coupled("displacements", i));
}

KernelBase::~KernelBase() {}
