#include "SolidMechanicsModule.h"
#include "Factory.h"
#include "ActionFactory.h"
#include "Parser.h"

// solid_mechanics
#include "AbaqusCreepMaterial.h"
#include "AbaqusUmatMaterial.h"
#include "AdaptiveTransient.h"
#include "CLSHPlasticMaterial.h"
#include "DashpotBC.h"
#include "Elastic.h"
#include "ElasticEnergyAux.h"
#include "ElementsOnLineAux.h"
#include "Gravity.h"
#include "HomogenizationKernel.h"
#include "HomogenizedElasticConstants.h"
#include "LinearAnisotropicMaterial.h"
#include "LinearGeneralAnisotropicMaterial.h"
#include "LinearIsotropicMaterial.h"
#include "LinearStrainHardening.h"
#include "MacroElastic.h"
#include "Mass.h"
#include "MaterialSymmElasticityTensorAux.h"
#include "MaterialTensorAux.h"
#include "MaterialTensorOnLine.h"
#include "MaterialVectorAux.h"
#include "AccumulateAux.h"
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
#include "StressDivergenceRZ.h"
#include "StressDivergenceRSpherical.h"
#include "StressDivergenceTruss.h"

void
Elk::SolidMechanics::registerObjects(Factory & factory)
{
  registerAux(ElasticEnergyAux);
  registerAux(MaterialSymmElasticityTensorAux);
  registerAux(MaterialTensorAux);
  registerAux(MaterialVectorAux);
  registerAux(AccumulateAux);
  registerAux(ElementsOnLineAux);

  registerExecutioner(AdaptiveTransient);

  registerMaterial(AbaqusCreepMaterial);
  registerMaterial(AbaqusUmatMaterial);
  registerMaterial(CLSHPlasticMaterial);
  registerMaterial(Elastic);
  registerMaterial(LinearAnisotropicMaterial);
  registerMaterial(LinearGeneralAnisotropicMaterial);
  registerMaterial(LinearIsotropicMaterial);
  registerMaterial(LinearStrainHardening);
  registerMaterial(MacroElastic);
  registerMaterial(PLC_LSH);
  registerMaterial(PLSHPlasticMaterial);
  registerMaterial(PowerLawCreep);

  registerBoundaryCondition(DashpotBC);
  registerBoundaryCondition(PlenumPressure);
  registerBoundaryCondition(PresetVelocity);
  registerBoundaryCondition(Pressure);

  registerKernel(Gravity);
  registerKernel(HomogenizationKernel);
  registerKernel(SolidMechImplicitEuler);
  registerKernel(StressDivergence);
  registerKernel(StressDivergenceRZ);
  registerKernel(StressDivergenceRSpherical);
  registerKernel(StressDivergenceTruss);

  registerPostprocessor(HomogenizedElasticConstants);
  registerPostprocessor(Mass);

  registerUserObject(MaterialTensorOnLine);

}

void
Elk::SolidMechanics::associateSyntax(Syntax & syntax, ActionFactory & action_factory)
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
