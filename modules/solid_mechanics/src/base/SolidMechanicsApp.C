//* This file is part of the MOOSE framework
//* https://www.mooseframework.org
//*
//* All rights reserved, see COPYRIGHT for full restrictions
//* https://github.com/idaholab/moose/blob/master/COPYRIGHT
//*
//* Licensed under LGPL 2.1, please see LICENSE for details
//* https://www.gnu.org/licenses/lgpl-2.1.html

#include "SolidMechanicsApp.h"
#include "TensorMechanicsApp.h"
#include "Moose.h"
#include "AppFactory.h"
#include "MooseSyntax.h"

#include "AbaqusCreepMaterial.h"
#include "AbaqusUmatMaterial.h"
#include "CLSHPlasticMaterial.h"
#include "CLSHPlasticModel.h"
#include "CombinedCreepPlasticity.h"
#include "Elastic.h"
#include "ElasticModel.h"
#include "HomogenizationKernel.h"
#include "HomogenizedElasticConstants.h"
#include "IsotropicPlasticity.h"
#include "IsotropicPowerLawHardening.h"
#include "IsotropicTempDepHardening.h"
#include "LinearAnisotropicMaterial.h"
#include "LinearGeneralAnisotropicMaterial.h"
#include "LinearIsotropicMaterial.h"
#include "LinearStrainHardening.h"
#include "MacroElastic.h"
#include "InteractionIntegralSM.h"
#include "MixedModeEquivalentK.h"
#include "MaterialSymmElasticityTensorAux.h"
#include "MaterialTensorAux.h"
#include "PLC_LSH.h"
#include "PowerLawCreep.h"
#include "PowerLawCreepModel.h"
#include "MaterialTensorIntegralSM.h"
#include "LineMaterialSymmTensorSampler.h"
#include "SolidMechanicsAction.h"
#include "SolidMechImplicitEuler.h"
#include "SolidModel.h"
#include "StressDivergence.h"
#include "OutOfPlaneStress.h"
#include "StressDivergenceRZ.h"
#include "StressDivergenceRSpherical.h"
#include "RateDepSmearCrackModel.h"
#include "RateDepSmearIsoCrackModel.h"

template <>
InputParameters
validParams<SolidMechanicsApp>()
{
  InputParameters params = validParams<MooseApp>();
  return params;
}

SolidMechanicsApp::SolidMechanicsApp(const InputParameters & parameters) : MooseApp(parameters)
{
  Moose::registerObjects(_factory);
  SolidMechanicsApp::registerObjectDepends(_factory);
  SolidMechanicsApp::registerObjects(_factory);

  Moose::associateSyntax(_syntax, _action_factory);
  SolidMechanicsApp::associateSyntaxDepends(_syntax, _action_factory);
  SolidMechanicsApp::associateSyntax(_syntax, _action_factory);

  Moose::registerExecFlags(_factory);
  SolidMechanicsApp::registerExecFlags(_factory);
}

SolidMechanicsApp::~SolidMechanicsApp() {}

// External entry point for dynamic application loading
extern "C" void
SolidMechanicsApp__registerApps()
{
  SolidMechanicsApp::registerApps();
}
void
SolidMechanicsApp::registerApps()
{
  registerApp(SolidMechanicsApp);
}

void
SolidMechanicsApp::registerObjectDepends(Factory & factory)
{
  TensorMechanicsApp::registerObjects(factory);
}

// External entry point for dynamic object registration
extern "C" void
SolidMechanicsApp__registerObjects(Factory & factory)
{
  TensorMechanicsApp::registerObjects(factory);
}
void
SolidMechanicsApp::registerObjects(Factory & factory)
{
  registerAux(MaterialSymmElasticityTensorAux);
  registerAux(MaterialTensorAux);

  registerMaterial(AbaqusCreepMaterial);
  registerMaterial(AbaqusUmatMaterial);
  registerMaterial(CLSHPlasticMaterial);
  registerMaterial(CLSHPlasticModel);
  registerMaterial(CombinedCreepPlasticity);
  registerMaterial(Elastic);
  registerMaterial(ElasticModel);
  registerMaterial(IsotropicPlasticity);
  registerMaterial(IsotropicPowerLawHardening);
  registerMaterial(IsotropicTempDepHardening);
  registerMaterial(LinearAnisotropicMaterial);
  registerMaterial(LinearGeneralAnisotropicMaterial);
  registerMaterial(LinearIsotropicMaterial);
  registerMaterial(LinearStrainHardening);
  registerMaterial(MacroElastic);
  registerMaterial(PLC_LSH);
  registerMaterial(PowerLawCreep);
  registerMaterial(PowerLawCreepModel);
  registerMaterial(SolidModel);
  registerMaterial(RateDepSmearCrackModel);
  registerMaterial(RateDepSmearIsoCrackModel);

  registerKernel(HomogenizationKernel);
  registerKernel(SolidMechImplicitEuler);
  registerKernel(StressDivergence);
  registerKernel(OutOfPlaneStress);
  registerKernel(StressDivergenceRZ);
  registerKernel(StressDivergenceRSpherical);

  registerPostprocessor(HomogenizedElasticConstants);
  registerPostprocessor(InteractionIntegralSM);
  registerPostprocessor(MaterialTensorIntegralSM);

  registerVectorPostprocessor(LineMaterialSymmTensorSampler);
}

void
SolidMechanicsApp::associateSyntaxDepends(Syntax & syntax, ActionFactory & action_factory)
{
  TensorMechanicsApp::associateSyntax(syntax, action_factory);
}

// External entry point for dynamic syntax association
extern "C" void
SolidMechanicsApp__associateSyntax(Syntax & syntax, ActionFactory & action_factory)
{
  SolidMechanicsApp::associateSyntax(syntax, action_factory);
}
void
SolidMechanicsApp::associateSyntax(Syntax & syntax, ActionFactory & action_factory)
{
  registerSyntax("SolidMechanicsAction", "SolidMechanics/*");

  registerAction(SolidMechanicsAction, "add_kernel");
}

// External entry point for dynamic execute flag registration
extern "C" void
SolidMechanicsApp__registerExecFlags(Factory & factory)
{
  SolidMechanicsApp::registerExecFlags(factory);
}
void
SolidMechanicsApp::registerExecFlags(Factory & /*factory*/)
{
}
