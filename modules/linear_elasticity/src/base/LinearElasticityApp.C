#include "LinearElasticityApp.h"
#include "Moose.h"
#include "AppFactory.h"

#include "LinearElasticityMaterial.h"
#include "SolidMechX.h"
#include "SolidMechY.h"
#include "SolidMechZ.h"
#include "SolidMechTempCoupleX.h"
#include "SolidMechTempCoupleY.h"
#include "SolidMechTempCoupleZ.h"

template<>
InputParameters validParams<LinearElasticityApp>()
{
  InputParameters params = validParams<MooseApp>();
  return params;
}

LinearElasticityApp::LinearElasticityApp(const std::string & name, InputParameters parameters) :
    MooseApp(name, parameters)
{
  srand(processor_id());

  Moose::registerObjects(_factory);
  LinearElasticityApp::registerObjects(_factory);

  Moose::associateSyntax(_syntax, _action_factory);
  LinearElasticityApp::associateSyntax(_syntax, _action_factory);
}

LinearElasticityApp::~LinearElasticityApp()
{
}

void
LinearElasticityApp::registerApps()
{
  registerApp(LinearElasticityApp);
}

void
LinearElasticityApp::registerObjects(Factory & factory)
{
  registerMaterial(LinearElasticityMaterial);
  registerKernel(SolidMechX);
  registerKernel(SolidMechY);
  registerKernel(SolidMechZ);
  registerKernel(SolidMechTempCoupleX);
  registerKernel(SolidMechTempCoupleY);
  registerKernel(SolidMechTempCoupleZ);
}

void
LinearElasticityApp::associateSyntax(Syntax & /*syntax*/, ActionFactory & /*action_factory*/)
{
}
