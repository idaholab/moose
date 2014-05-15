#include "ElasticModel.h"

#include "SymmElasticityTensor.h"


template<>
InputParameters validParams<ElasticModel>()
{
  InputParameters params = validParams<ConstitutiveModel>();
  return params;
}

ElasticModel::ElasticModel( const std::string & name,
                            InputParameters parameters )
  :ConstitutiveModel( name, parameters )
{
}

////////////////////////////////////////////////////////////////////////

ElasticModel::~ElasticModel()
{
}

////////////////////////////////////////////////////////////////////////

void
ElasticModel::computeStress( const Elem & /*current_elem*/,
                             unsigned /*qp*/,
                             const SymmElasticityTensor & elasticity_tensor,
                             const SymmTensor & stress_old,
                             SymmTensor & strain_increment,
                             SymmTensor & stress_new )
{
  stress_new = elasticity_tensor * strain_increment;
  stress_new += stress_old;
}
