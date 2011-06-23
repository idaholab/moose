#include "LinearIsotropicMaterialRZ.h"

// Elk Includes
#include "ElasticityTensor.h"
#include "SolidMechanicsMaterial.h"
#include "VolumetricModel.h"

template<>
InputParameters validParams<LinearIsotropicMaterialRZ>()
{
  InputParameters params = validParams<SolidMechanicsMaterialRZ>();
  params.addParam<Real>("thermal_conductivity", 0.0, "The thermal conductivity coeffecient.");
  return params;
}

LinearIsotropicMaterialRZ::LinearIsotropicMaterialRZ(const std::string  & name,
                                                     InputParameters parameters)
  :SolidMechanicsMaterialRZ(name, parameters),
   _input_thermal_conductivity(getParam<Real>("thermal_conductivity"))
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

