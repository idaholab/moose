#include "SolidMechanicsModule.h"
#include "Factory.h"
#include "ActionFactory.h"
#include "Parser.h"

// solid_mechanics
#include "AdaptiveTransient.h"
#include "CLSHPlasticMaterial.h"
#include "Elastic.h"
#include "ElasticEnergyAux.h"
#include "Gravity.h"
#include "LinearAnisotropicMaterial.h"
#include "LinearIsotropicMaterial.h"
#include "LinearStrainHardening.h"
#include "MaterialTensorAux.h"
#include "MaterialVectorAux.h"
#include "PLC_LSH.h"
#include "PowerLawCreep.h"
#include "PlenumPressure.h"
#include "PlenumPressureAction.h"
#include "PresetVelocity.h"
#include "Pressure.h"
#include "PressureAction.h"
#include "PLSHPlasticMaterial.h"
#include "SolidMechanicsAction.h"
#include "SolidMechImplicitEuler.h"
#include "StressDivergence.h"
#include "HomogenizationKernel.h"
#include "StressDivergenceRZ.h"
#include "LinearGeneralAnisotropicMaterial.h"
#include "DashpotBC.h"
#include "MaterialSymmElasticityTensorAux.h"
#include "HomogenizedElasticConstants.h"
#include "Mass.h"
#include "AbaqusUmatMaterial.h"
#include "AbaqusCreepMaterial.h"

void
Elk::SolidMechanics::registerObjects()
{
  // solid_mechanics
  registerExecutioner(AdaptiveTransient);

  registerMaterial(CLSHPlasticMaterial);
  registerMaterial(Elastic);
  registerKernel(Gravity);
  registerMaterial(LinearAnisotropicMaterial);
  registerMaterial(LinearIsotropicMaterial);
  registerMaterial(LinearStrainHardening);
  registerAux(MaterialTensorAux);
  registerAux(MaterialVectorAux);
  registerMaterial(PLC_LSH);
  registerMaterial(PLSHPlasticMaterial);
  registerMaterial(PowerLawCreep);
  registerMaterial(LinearGeneralAnisotropicMaterial);
  registerMaterial(AbaqusUmatMaterial);
  registerMaterial(AbaqusCreepMaterial);

  registerBoundaryCondition(DashpotBC);
  registerBoundaryCondition(PlenumPressure);

  registerBoundaryCondition(PresetVelocity);
  registerBoundaryCondition(Pressure);

  registerKernel(SolidMechImplicitEuler);
  registerAux(ElasticEnergyAux);
  registerAux(MaterialSymmElasticityTensorAux);

  registerKernel(StressDivergence);
  registerKernel(StressDivergenceRZ);
  registerKernel(HomogenizationKernel);
  registerPostprocessor(HomogenizedElasticConstants);
  registerPostprocessor(Mass);
}

void
Elk::SolidMechanics::associateSyntax(Syntax & syntax)
{
  syntax.registerActionSyntax("EmptyAction", "BCs/PlenumPressure");
  syntax.registerActionSyntax("PlenumPressureAction", "BCs/PlenumPressure/*");

  syntax.registerActionSyntax("EmptyAction", "BCs/Pressure");
  syntax.registerActionSyntax("PressureAction", "BCs/Pressure/*");

  syntax.registerActionSyntax("SolidMechanicsAction", "SolidMechanics/*");

  registerAction(PlenumPressureAction, "add_bc");
  registerAction(PressureAction, "add_bc");
  registerAction(SolidMechanicsAction, "add_kernel");
}
