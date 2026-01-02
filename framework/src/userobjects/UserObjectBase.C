//* This file is part of the MOOSE framework
//* https://mooseframework.inl.gov
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
UserObjectBase::validParams()
{
  InputParameters params = MooseObject::validParams();
  params += ReporterInterface::validParams();

  // Add the SetupInterface parameter, 'execute_on', and set it to a default of 'timestep_end'
  params += SetupInterface::validParams();
  params.set<ExecFlagEnum>("execute_on", true) = EXEC_TIMESTEP_END;
  ExecFlagEnum & exec_enum = params.set<ExecFlagEnum>("execute_on", true);
  exec_enum.addAvailableFlags(EXEC_TRANSFER);

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

  params.addParamNamesToGroup("execute_on force_preaux force_postaux force_preic "
                              "allow_duplicate_execution_on_initial execution_order_group",
                              "Execution scheduling");
  params.addParamNamesToGroup("use_displaced_mesh", "Advanced");

  params.registerBase("UserObject");

  return params;
}

UserObjectBase::UserObjectBase(const InputParameters & parameters)
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
    MeshDisplacedInterface(parameters),
    PerfGraphInterface(this),
    _tid(parameters.get<THREAD_ID>("_tid")),
    _subproblem(*getCheckedPointerParam<SubProblem *>("_subproblem")),
    _fe_problem(*getCheckedPointerParam<FEProblemBase *>("_fe_problem_base")),
    _sys(*getCheckedPointerParam<SystemBase *>("_sys")),
    _assembly(_subproblem.assembly(_tid, 0)),
    _duplicate_initial_execution(getParam<bool>("allow_duplicate_execution_on_initial"))
{
  // Check the pre/post aux flag
  if (getParam<bool>("force_preaux") && getParam<bool>("force_postaux"))
    paramError("force_preaux",
               "A user object may be specified as executing before or after "
               "AuxKernels, not both.");

  _supplied_uo.insert(name());
}

#ifdef MOOSE_KOKKOS_ENABLED
UserObjectBase::UserObjectBase(const UserObjectBase & object,
                               const Moose::Kokkos::FunctorCopy & key)
  : MooseObject(object, key),
    SetupInterface(object, key),
    FunctionInterface(object, key),
    UserObjectInterface(object, key),
    PostprocessorInterface(object, key),
    VectorPostprocessorInterface(object, key),
    ReporterInterface(object, key),
    DistributionInterface(object, key),
    SamplerInterface(object, key),
    Restartable(object, key),
    MeshMetaDataInterface(object, key),
    MeshChangedInterface(object, key),
    MeshDisplacedInterface(object, key),
    PerfGraphInterface(object, key),
    _tid(object._tid),
    _subproblem(object._subproblem),
    _fe_problem(object._fe_problem),
    _sys(object._sys),
    _assembly(object._assembly),
    _duplicate_initial_execution(object._duplicate_initial_execution)
{
}
#endif

std::set<UserObjectName>
UserObjectBase::getDependObjects() const
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
UserObjectBase::addUserObjectDependencyHelper(const UserObject & uo) const
{
  _depend_uo.insert(uo.name());
}

void
UserObjectBase::addPostprocessorDependencyHelper(const PostprocessorName & name) const
{
  _depend_uo.insert(name);
}

void
UserObjectBase::addVectorPostprocessorDependencyHelper(const VectorPostprocessorName & name) const
{
  _depend_uo.insert(name);
}

void
UserObjectBase::addReporterDependencyHelper(const ReporterName & reporter_name)
{
  _depend_uo.insert(reporter_name.getObjectName());
}
