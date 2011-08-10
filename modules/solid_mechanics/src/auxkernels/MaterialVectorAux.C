#include "MaterialVectorAux.h"

template<>
InputParameters validParams<MaterialVectorAux>()
{
  InputParameters params = validParams<AuxKernel>();
  params.addRequiredParam<std::string>("vector", "The material tensor name.");
  params.addParam<int>("index", -1, "The index into the tensor, from 0 to 5 (xx, yy, zz, xy, yz, zx).");
  params.addParam<std::string>("quantity", "", "A scalar quantity to compute: (only option is Length).");
  return params;
}

MaterialVectorAux::MaterialVectorAux( const std::string & name,
                                      InputParameters parameters )
  :AuxKernel( name, parameters ),
   _vector( getMaterialProperty<RealVectorValue>( getParam<std::string>("vector") ) ),
   _index( getParam<int>("index") ),
   _quantity_string( getParam<std::string>("quantity") ),
   _quantity( MVA_COMPONENT )
{
  std::transform(_quantity_string.begin(), _quantity_string.end(),
                 _quantity_string.begin(), ::tolower);
  if ( _quantity_string == "" && _index < 0 )
  {
    mooseError("Neither an index nor a quantity listed for " + _name);
  }
  else if ( _quantity_string == "length" )
  {
    _quantity = MVA_LENGTH;
  }
  else if ( _quantity_string != "" )
  {
    mooseError("Unrecognized quantity in " + _name);
  }

  if (_index > 0 && _quantity != MVA_COMPONENT)
  {
    mooseError("Cannot define an index and a quantity in " + _name);
  }

  if (_index > -1 && _index > 2)
  {
    mooseError("MaterialVectorAux requires the index to be >= 0 and <= 2 OR < 0 (off).");
  }
}

Real
MaterialVectorAux::computeValue()
{
  Real value(0);
  const RealVectorValue & vector( _vector[_qp] );
  if (_quantity == MVA_COMPONENT)
  {
    value = vector(_index);
  }
  else if ( _quantity == MVA_LENGTH )
  {
    value = vector.size();
  }
  else
  {
    mooseError("Internal logic error from " + name());
  }
  return value;
}


