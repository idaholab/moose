#include "ExampleApp.h"
#include "Moose.h"
#include "AppFactory.h"

// Example Includes
#include "BlockAverageDiffusionMaterial.h"
#include "BlockAverageValue.h"
#include "ExampleDiffusion.h"

template<>
InputParameters validParams<ExampleApp>()
{
  InputParameters params = validParams<MooseApp>();
  return params;
}

ExampleApp::ExampleApp(const std::string & name, InputParameters parameters) :
    MooseApp(name, parameters)
{
  srand(libMesh::processor_id());

  Moose::registerObjects(_factory);
  ExampleApp::registerObjects(_factory);

  Moose::associateSyntax(_syntax, _action_factory);
  ExampleApp::associateSyntax(_syntax, _action_factory);
}

ExampleApp::~ExampleApp()
{
}



void
ExampleApp::registerApps()
{
  registerApp(ExampleApp);
}

void
ExampleApp::registerObjects(Factory & factory)
{
  registerMaterial(BlockAverageDiffusionMaterial);
  registerKernel(ExampleDiffusion);

  // This is how to register a UserObject
  registerUserObject(BlockAverageValue);
}

void
ExampleApp::associateSyntax(Syntax & /*syntax*/, ActionFactory & /*action_factory*/)
{
}
