//* This file is part of the MOOSE framework
//* https://mooseframework.inl.gov
//*
//* All rights reserved, see COPYRIGHT for full restrictions
//* https://github.com/idaholab/moose/blob/master/COPYRIGHT
//*
//* Licensed under LGPL 2.1, please see LICENSE for details
//* https://www.gnu.org/licenses/lgpl-2.1.html

#include "SubChannelApp.h"
#include "Moose.h"
#include "AppFactory.h"
#include "FluidPropertiesApp.h"
#include "HeatTransferApp.h"
#include "ReactorApp.h"
#include "MooseSyntax.h"
#include "SubChannelSyntax.h"

InputParameters
SubChannelApp::validParams()
{
  InputParameters params = MooseApp::validParams();
  params.set<bool>("use_legacy_initial_residual_evaluation_behavior") = false;
  params.set<bool>("use_legacy_material_output") = false;

  return params;
}

SubChannelApp::SubChannelApp(const InputParameters & parameters) : MooseApp(parameters)
{
  SubChannelApp::registerAll(_factory, _action_factory, _syntax);
}

void
SubChannelApp::registerAll(Factory & f, ActionFactory & af, Syntax & s)
{
  Registry::registerObjectsTo(f, {"SubChannelApp"});
  Registry::registerActionsTo(af, {"SubChannelApp"});

  // Cited by --citations whenever any SubChannel object is used in a simulation
  Registry::addAppCitation("SubChannelApp",
                           "kyriakopoulos2022development",
                           R"(@article{kyriakopoulos2022development,
  title={Development of a Single-Phase, Transient, Subchannel Code, within the MOOSE Multi-Physics Computational Framework},
  author={Kyriakopoulos, Vasileios and Tano, Mauricio E and Ragusa, Jean C},
  journal={Energies},
  volume={15},
  number={11},
  pages={3948},
  year={2022},
  publisher={MDPI}
})");
  Registry::addAppCitation("SubChannelApp",
                           "kyriakopoulos2026numerical",
                           R"(@article{kyriakopoulos2026numerical,
  title={Numerical implementation of the MOOSE subchannel module (SCM) algorithm},
  author={Kyriakopoulos, Vasileios and Tano, Mauricio},
  journal={Nuclear Engineering and Design},
  volume={450},
  pages={114802},
  year={2026},
  publisher={Elsevier}
})");

  /* register custom execute flags, action syntax, etc. here */
  SubChannel::associateSyntax(s, af);

  // Register new syntax for SCMClosures
  auto & syntax = s;
  registerSyntaxTask("AddSCMClosureAction", "SCMClosures/*", "add_scm_closure");
  registerMooseObjectTask("add_scm_closure", SCMClosureBase, /*default task*/ false);

  FluidPropertiesApp::registerAll(f, af, s);
  HeatTransferApp::registerAll(f, af, s);
  ReactorApp::registerAll(f, af, s);
}

void
SubChannelApp::registerApps()
{
  registerApp(SubChannelApp);

  FluidPropertiesApp::registerApps();
  HeatTransferApp::registerApps();
  ReactorApp::registerApps();
}

/***************************************************************************************************
 *********************** Dynamic Library Entry Points - DO NOT MODIFY ******************************
 **************************************************************************************************/
extern "C" void
SubChannelApp__registerAll(Factory & f, ActionFactory & af, Syntax & s)
{
  SubChannelApp::registerAll(f, af, s);
}
extern "C" void
SubChannelApp__registerApps()
{
  SubChannelApp::registerApps();
}
