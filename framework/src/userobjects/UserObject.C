//* This file is part of the MOOSE framework
//* https://www.mooseframework.org
//*
//* All rights reserved, see COPYRIGHT for full restrictions
//* https://github.com/idaholab/moose/blob/master/COPYRIGHT
//*
//* Licensed under LGPL 2.1, please see LICENSE for details
//* https://www.gnu.org/licenses/lgpl-2.1.html

#include "UserObject.h"
#include "SubProblem.h"
#include "Assembly.h"
#include "NonlinearSystemBase.h"

#include "libmesh/sparse_matrix.h"

InputParameters
UserObject::validParams()
{
  InputParameters params = MooseObject::validParams();
  params += ReporterInterface::validParams();

  // Add the SetupInterface parameter, 'execute_on', and set it to a default of 'timestep_end'
  params += SetupInterface::validParams();
  params.set<ExecFlagEnum>("execute_on", true) = EXEC_TIMESTEP_END;

  params.addParam<bool>("use_displaced_mesh",
                        false,
                        "Whether or not this object should use the "
                        "displaced mesh for computation.  Note that "
                        "in the case this is true but no "
                        "displacements are provided in the Mesh block "
                        "the undisplaced mesh will still be used.");

  // Execution parameters
  params.addParam<bool>("allow_duplicate_execution_on_initial",
                        false,
                        "In the case where this UserObject is depended upon by an initial "
                        "condition, allow it to be executed twice during the initial setup (once "
                        "before the IC and again after mesh adaptivity (if applicable).");
  params.declareControllable("enable");

  params.addParam<bool>("force_preaux", false, "Forces the UserObject to be executed in PREAUX");
  params.addParam<bool>("force_postaux", false, "Forces the UserObject to be executed in POSTAUX");
  params.addParam<bool>(
      "force_preic", false, "Forces the UserObject to be executed in PREIC during initial setup");
  params.addParam<int>(
      "execution_order_group",
      0,
      "Execution order groups are executed in increasing order (e.g., the lowest "
      "number is executed first). Note that negative group numbers may be used to execute groups "
      "before the default (0) group. Please refer to the user object documentation "
      "for ordering of user object execution within a group.");

  params.registerBase("UserObject");
  params.registerSystemAttributeName("UserObject");

  params.addParamNamesToGroup("use_displaced_mesh allow_duplicate_execution_on_initial "
                              "force_preaux force_postaux force_preic execution_order_group",
                              "Advanced");
  return params;
}

UserObject::UserObject(const InputParameters & parameters)
  : MooseObject(parameters),
    SetupInterface(this),
    FunctionInterface(this),
    UserObjectInterface(this),
    PostprocessorInterface(this),
    VectorPostprocessorInterface(this),
    ReporterInterface(this),
    DistributionInterface(this),
    SamplerInterface(this),
    Restartable(this, "UserObjects"),
    MeshMetaDataInterface(this),
    MeshChangedInterface(parameters),
    PerfGraphInterface(this),
    _subproblem(*getCheckedPointerParam<SubProblem *>("_subproblem")),
    _fe_problem(*getCheckedPointerParam<FEProblemBase *>("_fe_problem_base")),
    _sys(*getCheckedPointerParam<SystemBase *>("_sys")),
    _tid(parameters.get<THREAD_ID>("_tid")),
    _assembly(_subproblem.assembly(_tid, 0)),
    _coord_sys(_assembly.coordSystem()),
    _duplicate_initial_execution(getParam<bool>("allow_duplicate_execution_on_initial"))
{
  // Check the pre/post aux flag
  if (getParam<bool>("force_preaux") && getParam<bool>("force_postaux"))
    paramError("force_preaux",
               "A user object may be specified as executing before or after "
               "AuxKernels, not both.");

  mooseAssert(_sys.varKind() == Moose::VAR_SOLVER,
              "We expect the system to conceptually be nonlinear or linear.");

  _supplied_uo.insert(name());
}

std::set<UserObjectName>
UserObject::getDependObjects() const
{
  std::set<UserObjectName> all;
  for (auto & v : _depend_uo)
  {
    all.insert(v);
    auto & uo = UserObjectInterface::getUserObjectBaseByName(v);

    // Add dependencies of other objects, but don't allow it to call itself. This can happen
    // through the PostprocessorInterface if a Postprocessor calls getPostprocessorValueByName
    // with it's own name. This happens in the Receiver, which could use the FEProblem version of
    // the get method, but this is a fix that prevents an infinite loop occurring by accident for
    // future objects.
    if (uo.name() != name())
    {
      auto uos = uo.getDependObjects();
      for (auto & t : uos)
        all.insert(t);
    }
  }
  return all;
}

void
UserObject::addUserObjectDependencyHelper(const UserObject & uo) const
{
  _depend_uo.insert(uo.name());
}

void
UserObject::addPostprocessorDependencyHelper(const PostprocessorName & name) const
{
  _depend_uo.insert(name);
}

void
UserObject::addVectorPostprocessorDependencyHelper(const VectorPostprocessorName & name) const
{
  _depend_uo.insert(name);
}

void
UserObject::addReporterDependencyHelper(const ReporterName & reporter_name)
{
  _depend_uo.insert(reporter_name.getObjectName());
}

void
UserObject::setPrimaryThreadCopy(UserObject * primary)
{
  if (!_primary_thread_copy && primary != this)
    _primary_thread_copy = primary;
}

unsigned int
UserObject::systemNumber() const
{
  return _sys.number();
}
