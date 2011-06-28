#include "LinearIsotropicMaterialRZ.h"

// Elk Includes
#include "ElasticityTensor.h"
#include "SolidMechanicsMaterial.h"
#include "VolumetricModel.h"

template<>
InputParameters validParams<LinearIsotropicMaterialRZ>()
{
  InputParameters params = validParams<SolidMechanicsMaterialRZ>();
  return params;
}

LinearIsotropicMaterialRZ::LinearIsotropicMaterialRZ(const std::string  & name,
                                                     InputParameters parameters)
  :SolidMechanicsMaterialRZ(name, parameters)
{
}

LinearIsotropicMaterialRZ::~LinearIsotropicMaterialRZ()
{
}

void
LinearIsotropicMaterialRZ::computeStrain(const SymmTensor & total_strain, SymmTensor & elastic_strain)
{
  elastic_strain = total_strain;
}

