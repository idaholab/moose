#include "StressAux.h"


template<>
InputParameters validParams<StressAux>()
{
  InputParameters params = validParams<AuxKernel>();
  params.addRequiredParam<unsigned int>("index", "The index into the stress array, from 0 to 5 (xx, yy, zz, xy, yz, zx).");
  return params;
}

StressAux::StressAux( const std::string & name,
                      MooseSystem & moose_system,
                      InputParameters parameters )
  :AuxKernel( name, moose_system, parameters ),
   _index( getParam<unsigned int>("index") )
//    _stress( getMaterialProperty<RealTensorValue>("stress") )
{
  if (_index > 5)
  {
    mooseError("StressAux requires the index to be >= 0 and <= 5.");
  }
}

Real
StressAux::computeValue()
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
//   return _stress[_qp](i,j);
  return _index;
}
