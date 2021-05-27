//* This file is part of the MOOSE framework
//* https://www.mooseframework.org
//*
//* All rights reserved, see COPYRIGHT for full restrictions
//* https://github.com/idaholab/moose/blob/master/COPYRIGHT
//*
//* Licensed under LGPL 2.1, please see LICENSE for details
//* https://www.gnu.org/licenses/lgpl-2.1.html

#include "RayTracingObject.h"

#include "TraceRay.h"

InputParameters
RayTracingObject::validParams()
{
  auto params = MooseObject::validParams();
  params += SetupInterface::validParams();
  params += TransientInterface::validParams();
  params += MeshChangedInterface::validParams();

  params.addParam<UserObjectName>("study",
                                  "The RayTracingStudy associated with this object. If none "
                                  "provided, this will default to the one study that exists.");
  params.addParam<std::vector<std::string>>(
      "rays",
      "The name of the Rays associated with this object; only used if Ray registration is enabled "
      "within the study. If no Rays are supplied, this object will be applied to all Rays.");

  params.addPrivateParam<RayTracingStudy *>("_ray_tracing_study");

  // Execute flags don't make sense here because the RayTracingStudy calls this object
  params.suppressParameter<ExecFlagEnum>("execute_on");

  return params;
}

RayTracingObject::RayTracingObject(const InputParameters & params)
  : MooseObject(params),
    SetupInterface(this),
    FunctionInterface(this),
    PostprocessorInterface(this),
    VectorPostprocessorInterface(this),
    MeshChangedInterface(params),
    TransientInterface(this),
    UserObjectInterface(this),
    DependencyResolverInterface(),
    _tid(params.get<THREAD_ID>("_tid")),
    _fe_problem(*params.getCheckedPointerParam<FEProblemBase *>("_fe_problem_base")),
    _study(*params.getCheckedPointerParam<RayTracingStudy *>("_ray_tracing_study")),
    _trace_ray(const_cast<const RayTracingStudy &>(_study).traceRay(_tid)),
    _nl(_fe_problem.getNonlinearSystemBase()),
    _aux(_fe_problem.getAuxiliarySystem()),
    _mesh(_fe_problem.mesh()),
    _current_elem(_trace_ray.currentElem()),
    _current_subdomain_id(_trace_ray.currentSubdomainID()),
    _current_intersected_side(_trace_ray.currentIntersectedSide()),
    _current_intersected_extrema(_trace_ray.currentIntersectedExtrema()),
    _current_ray(_trace_ray.currentRay())
{
  // Supplies only itself
  _supplied_names.insert(name());
}
