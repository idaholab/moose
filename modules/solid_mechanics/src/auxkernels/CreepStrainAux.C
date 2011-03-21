#include "CreepStrainAux.h"


template<>
InputParameters validParams<CreepStrainAux>()
{
  InputParameters params = validParams<AuxKernel>();
  params.addRequiredParam<unsigned int>("index", "The index into the creep strain array, from 0 to 5 (xx, yy, zz, xy, yz, zx).");
  return params;
}

CreepStrainAux::CreepStrainAux( const std::string & name,
                      InputParameters parameters )
  :AuxKernel( name, parameters ),
   _index( getParam<unsigned int>("index") ),
   _creep_strain( getMaterialProperty<RealTensorValue>("creep_strain") )
{
  if (_index > 5)
  {
    mooseError("CreepStrainAux requires the index to be >= 0 and <= 5.");
  }
}

Real
CreepStrainAux::computeValue()
{
  unsigned i(0), j(0); // xx
  if ( _index == 1 ) // yy
  {
    i = j = 1;
  }
  else if ( _index == 2 ) // zz
  {
    i = j = 2;
  }
  else if ( _index == 3 ) // xy
  {
    j = 1;
  }
  else if ( _index == 4 ) // yz
  {
    i = 1;
    j = 2;
  }
  else if ( _index == 5 ) // zx
  {
    j = 2;
  }
  return _creep_strain[_qp](i,j);
}


