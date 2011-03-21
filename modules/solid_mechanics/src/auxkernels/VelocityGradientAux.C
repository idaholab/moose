#include "VelocityGradientAux.h"


template<>
InputParameters validParams<VelocityGradientAux>()
{
  InputParameters params = validParams<AuxKernel>();
  params.addRequiredParam<unsigned int>("index", "The index into the plaatic strain array, from 0 to 5 (xx, yy, zz, xy, yz, zx).");
  return params;
}

VelocityGradientAux::VelocityGradientAux( const std::string & name,
                      InputParameters parameters )
  :AuxKernel( name, parameters ),
   _index( getParam<unsigned int>("index") ),
   _total_strain( getMaterialProperty<RealTensorValue>("total_strain") )
{
  if (_index > 5)
  {
    mooseError("PlasticStrainAux requires the index to be >= 0 and <= 5.");
  }
}

Real
VelocityGradientAux::computeValue()
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
  return _total_strain[_qp](i,j);
}


