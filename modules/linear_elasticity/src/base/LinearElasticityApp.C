/****************************************************************/
/* MOOSE - Multiphysics Object Oriented Simulation Environment  */
/*                                                              */
/*          All contents are licensed under LGPL V2.1           */
/*             See LICENSE for full restrictions                */
/****************************************************************/
#include "LinearElasticityApp.h"
#include "Moose.h"
#include "AppFactory.h"
#include "MooseSyntax.h"

#include "LinearElasticityMaterial.h"
#include "SolidMechX.h"
#include "SolidMechY.h"
#include "SolidMechZ.h"
#include "SolidMechTempCoupleX.h"
#include "SolidMechTempCoupleY.h"
#include "SolidMechTempCoupleZ.h"

// Initialize static member variables
bool LinearElasticityApp::_registered_objects = false;
bool LinearElasticityApp::_associated_syntax = false;

template<>
InputParameters validParams<LinearElasticityApp>()
{
  InputParameters params = validParams<MooseApp>();
  params.set<bool>("use_legacy_uo_initialization") = false;
  params.set<bool>("use_legacy_uo_aux_computation") = false;
  return params;
}

LinearElasticityApp::LinearElasticityApp(const InputParameters & parameters) :
    MooseApp(parameters)
{
  Moose::registerObjects(_factory);
  LinearElasticityApp::registerObjects(_factory);

  Moose::associateSyntax(_syntax, _action_factory);
  LinearElasticityApp::associateSyntax(_syntax, _action_factory);
}

LinearElasticityApp::~LinearElasticityApp()
{
}

// External entry point for dynamic application loading
extern "C" void LinearElasticityApp__registerApps() { LinearElasticityApp::registerApps(); }
void
LinearElasticityApp::registerApps()
{
  registerApp(LinearElasticityApp);
}

// External entry point for dynamic object registration
extern "C" void LinearElasticityApp__registerObjects(Factory & factory) { LinearElasticityApp::registerObjects(factory); }
void
LinearElasticityApp::registerObjects(Factory & factory)
{
  if (_registered_objects)
    return;
  _registered_objects = true;

  registerMaterial(LinearElasticityMaterial);
  registerKernel(SolidMechX);
  registerKernel(SolidMechY);
  registerKernel(SolidMechZ);
  registerKernel(SolidMechTempCoupleX);
  registerKernel(SolidMechTempCoupleY);
  registerKernel(SolidMechTempCoupleZ);
}

// External entry point for dynamic syntax association
extern "C" void LinearElasticityApp__associateSyntax(Syntax & syntax, ActionFactory & action_factory) { LinearElasticityApp::associateSyntax(syntax, action_factory); }
void
LinearElasticityApp::associateSyntax(Syntax & /*syntax*/, ActionFactory & /*action_factory*/)
{
  if (_associated_syntax)
    return;
  _associated_syntax = true;

}
