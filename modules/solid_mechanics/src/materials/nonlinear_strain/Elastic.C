#include "Elastic.h"


template<>
InputParameters validParams<Elastic>()
{
  InputParameters params = validParams<MaterialModel>();
  return params;
}

Elastic::Elastic( const std::string & name,
                  InputParameters parameters )
  :MaterialModel( name, parameters )
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
  MaterialModel::computeStress();
}
