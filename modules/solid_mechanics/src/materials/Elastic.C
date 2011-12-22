#include "Elastic.h"

#include "SymmElasticityTensor.h"


template<>
InputParameters validParams<Elastic>()
{
  InputParameters params = validParams<SolidModel>();
  return params;
}

Elastic::Elastic( const std::string & name,
                  InputParameters parameters )
  :SolidModel( name, parameters )
{
}

////////////////////////////////////////////////////////////////////////

Elastic::~Elastic()
{
}

////////////////////////////////////////////////////////////////////////

void
Elastic::computeStress()
{
  SymmTensor stress_new( _elasticity_tensor[_qp] * _strain_increment );
  _stress[_qp] = stress_new;
  _stress[_qp] += _stress_old;
}
