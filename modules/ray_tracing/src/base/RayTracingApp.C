//* This file is part of the MOOSE framework
//* https://www.mooseframework.org
//*
//* All rights reserved, see COPYRIGHT for full restrictions
//* https://github.com/idaholab/moose/blob/master/COPYRIGHT
//*
//* Licensed under LGPL 2.1, please see LICENSE for details
//* https://www.gnu.org/licenses/lgpl-2.1.html

#include "RayTracingApp.h"
#include "Moose.h"
#include "AppFactory.h"
#include "MooseSyntax.h"

#include "RayTracingAttributes.h"

InputParameters
RayTracingApp::validParams()
{
  InputParameters params = MooseApp::validParams();

  params.set<bool>("use_legacy_material_output") = false;

  return params;
}

registerKnownLabel("RayTracingApp");

RayTracingApp::RayTracingApp(InputParameters parameters) : MooseApp(parameters)
{
  RayTracingApp::registerAll(_factory, _action_factory, _syntax);
}

RayTracingApp::~RayTracingApp() {}

void
RayTracingApp::registerAll(Factory & f, ActionFactory & af, Syntax & syntax)
{
  Registry::registerObjectsTo(f, {"RayTracingApp"});
  Registry::registerActionsTo(af, {"RayTracingApp"});

  f.app().theWarehouse().registerAttribute<AttribRayTracingStudy>("ray_tracing_study", nullptr);

  // Adds [RayKernels] block
  registerSyntaxTask("AddRayKernelAction", "RayKernels/*", "add_ray_kernel");
  registerMooseObjectTask("add_ray_kernel", RayKernel, false);
  addTaskDependency("add_ray_kernel", "add_kernel");

  // Adds [RayBCs] block
  registerSyntaxTask("AddRayBCAction", "RayBCs/*", "add_ray_boundary_condition");
  registerMooseObjectTask("add_ray_boundary_condition", RayBoundaryCondition, false);
  addTaskDependency("add_ray_boundary_condition", "add_kernel");
}

void
RayTracingApp::registerApps()
{
  registerApp(RayTracingApp);
}

void
RayTracingApp::registerObjects(Factory & factory)
{
  mooseDeprecated("use registerAll instead of registerObjects");
  Registry::registerObjectsTo(factory, {"RayTracingApp"});
}

void
RayTracingApp::associateSyntax(Syntax & /*syntax*/, ActionFactory & action_factory)
{
  mooseDeprecated("use registerAll instead of associateSyntax");
  Registry::registerActionsTo(action_factory, {"RayTracingApp"});
}

void
RayTracingApp::registerExecFlags(Factory & /*factory*/)
{
  mooseDeprecated("Do not use registerExecFlags, apps no longer require flag registration");
}

extern "C" void
RayTracingApp__registerAll(Factory & f, ActionFactory & af, Syntax & s)
{
  RayTracingApp::registerAll(f, af, s);
}
extern "C" void
RayTracingApp__registerApps()
{
  RayTracingApp::registerApps();
}
