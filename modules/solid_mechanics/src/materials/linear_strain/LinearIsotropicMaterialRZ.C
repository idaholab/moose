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
LinearIsotropicMaterialRZ::computeStress(const SymmTensor & total_strain,
                                         const SymmTensor & strain,
                                         const SymmElasticityTensor & elasticity_tensor,
                                         SymmTensor & stress)
{

  SymmTensor elastic_strain;

  computeStrain(strain, elastic_strain);

  // Save that off as the elastic strain
  _elastic_strain[_qp] = elastic_strain;

  // C * e
  stress = elasticity_tensor * elastic_strain;

  computeCracking( total_strain, stress );

}

void
LinearIsotropicMaterialRZ::computeStrain(const SymmTensor & total_strain, SymmTensor & elastic_strain)
{
  elastic_strain = total_strain;
  //Jacobian multiplier of the stress
  _Jacobian_mult[_qp] = *_local_elasticity_tensor;
}

