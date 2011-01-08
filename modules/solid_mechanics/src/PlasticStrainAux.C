#include "PlasticStrainAux.h"


template<>
InputParameters validParams<PlasticStrainAux>()
{
  InputParameters params = validParams<AuxKernel>();
  params.addRequiredParam<unsigned int>("index", "The index into the plaatic strain array, from 0 to 5 (xx, yy, zz, xy, yz, zx).");
  return params;
}

PlasticStrainAux::PlasticStrainAux( const std::string & name,
                      InputParameters parameters )
  :AuxKernel( name, parameters ),
   _index( getParam<unsigned int>("index") ),
   _plastic_strain( getMaterialProperty<RealTensorValue>("plastic_strain") )
{
  if (_index > 5)
  {
    mooseError("PlasticStrainAux requires the index to be >= 0 and <= 5.");
  }
}

Real
PlasticStrainAux::computeValue()
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
  return _plastic_strain[_qp](i,j);
}


