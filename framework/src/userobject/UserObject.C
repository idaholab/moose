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

#include "libmesh/sparse_matrix.h"

defineLegacyParams(UserObject);

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
  params.addParam<bool>("allow_duplicate_execution_on_initial",
                        false,
                        "In the case where this UserObject is depended upon by an initial "
                        "condition, allow it to be executed twice during the initial setup (once "
                        "before the IC and again after mesh adaptivity (if applicable).");
  params.declareControllable("enable");

  params.registerBase("UserObject");
  params.registerSystemAttributeName("UserObject");

  params.addParamNamesToGroup("use_displaced_mesh allow_duplicate_execution_on_initial",
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
    ScalarCoupleable(this),
    PerfGraphInterface(this),
    _subproblem(*getCheckedPointerParam<SubProblem *>("_subproblem")),
    _fe_problem(*getCheckedPointerParam<FEProblemBase *>("_fe_problem_base")),
    _tid(parameters.get<THREAD_ID>("_tid")),
    _assembly(_subproblem.assembly(_tid)),
    _coord_sys(_assembly.coordSystem()),
    _duplicate_initial_execution(getParam<bool>("allow_duplicate_execution_on_initial"))
{
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

const UserObject &
UserObject::getUserObjectBase(const std::string & param_name) const
{
  const auto & uo = UserObjectInterface::getUserObjectBase(param_name);
  _depend_uo.insert(uo.name());
  return uo;
}

const UserObject &
UserObject::getUserObjectBaseByName(const UserObjectName & object_name) const
{
  const auto & uo = UserObjectInterface::getUserObjectBaseByName(object_name);
  _depend_uo.insert(object_name);
  return uo;
}

const PostprocessorValue &
UserObject::getPostprocessorValue(const std::string & name, unsigned int index /* = 0 */) const
{
  if (!isDefaultPostprocessorValue(name, index)) // if default, no dependencies to add
    _depend_uo.insert(getPostprocessorName(name, index));
  return PostprocessorInterface::getPostprocessorValue(name, index);
}

const PostprocessorValue &
UserObject::getPostprocessorValueByName(const PostprocessorName & name) const
{
  _depend_uo.insert(name);
  return PostprocessorInterface::getPostprocessorValueByName(name);
}

const VectorPostprocessorValue &
UserObject::getVectorPostprocessorValue(const std::string & name,
                                        const std::string & vector_name) const
{
  _depend_uo.insert(_pars.get<VectorPostprocessorName>(name));
  return VectorPostprocessorInterface::getVectorPostprocessorValue(name, vector_name);
}

const VectorPostprocessorValue &
UserObject::getVectorPostprocessorValueByName(const VectorPostprocessorName & name,
                                              const std::string & vector_name) const
{
  _depend_uo.insert(name);
  return VectorPostprocessorInterface::getVectorPostprocessorValueByName(name, vector_name);
}

const VectorPostprocessorValue &
UserObject::getVectorPostprocessorValue(const std::string & name,
                                        const std::string & vector_name,
                                        bool needs_broadcast) const
{
  _depend_uo.insert(_pars.get<VectorPostprocessorName>(name));
  return VectorPostprocessorInterface::getVectorPostprocessorValue(
      name, vector_name, needs_broadcast);
}

const VectorPostprocessorValue &
UserObject::getVectorPostprocessorValueByName(const VectorPostprocessorName & name,
                                              const std::string & vector_name,
                                              bool needs_broadcast) const
{
  _depend_uo.insert(name);
  return VectorPostprocessorInterface::getVectorPostprocessorValueByName(
      name, vector_name, needs_broadcast);
}

const ScatterVectorPostprocessorValue &
UserObject::getScatterVectorPostprocessorValue(const std::string & name,
                                               const std::string & vector_name) const
{
  _depend_uo.insert(_pars.get<VectorPostprocessorName>(name));
  return VectorPostprocessorInterface::getScatterVectorPostprocessorValue(name, vector_name);
}

const ScatterVectorPostprocessorValue &
UserObject::getScatterVectorPostprocessorValueByName(const std::string & name,
                                                     const std::string & vector_name) const
{
  _depend_uo.insert(name);
  return VectorPostprocessorInterface::getScatterVectorPostprocessorValueByName(name, vector_name);
}

void
UserObject::setPrimaryThreadCopy(UserObject * primary)
{
  if (!_primary_thread_copy && primary != this)
    _primary_thread_copy = primary;
}
