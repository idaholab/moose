#include "MaterialTensorAux.h"

#include "SymmTensor.h"

template<>
InputParameters validParams<MaterialTensorAux>()
{
  InputParameters params = validParams<AuxKernel>();
  params.addRequiredParam<std::string>("tensor", "The material tensor name.");
  params.addParam<int>("index", -1, "The index into the tensor, from 0 to 5 (xx, yy, zz, xy, yz, zx).");
  params.addParam<std::string>("quantity", "", "A scalar quantity to compute: VonMises, Hydrostatic, FirstInvariant, SecondInvariant, ThirdInvariant.");
  return params;
}

MaterialTensorAux::MaterialTensorAux( const std::string & name,
                                      InputParameters parameters )
  :AuxKernel( name, parameters ),
   _tensor( getMaterialProperty<SymmTensor>( getParam<std::string>("tensor") ) ),
   _index( getParam<int>("index") ),
   _quantity_string( getParam<std::string>("quantity") ),
   _quantity( MTA_COMPONENT )
{
  std::transform(_quantity_string.begin(), _quantity_string.end(),
                 _quantity_string.begin(), ::tolower);
  if ( _quantity_string == "" && _index < 0 )
  {
    mooseError("Neither an index nor a quantity listed for " + _name);
  }
  else if ( _quantity_string == "vonmises" )
  {
    _quantity = MTA_VONMISES;
  }
  else if ( _quantity_string == "plasticstrainmag" )
  {
    _quantity = MTA_PLASTICSTRAINMAG;
  }
  else if ( _quantity_string == "hydrostatic" )
  {
    _quantity = MTA_HYDROSTATIC;
  }
  else if ( _quantity_string == "firstinvariant" )
  {
    _quantity = MTA_FIRSTINVARIANT;
  }
  else if ( _quantity_string == "secondinvariant" )
  {
    _quantity = MTA_SECONDINVARIANT;
  }
  else if ( _quantity_string == "thirdinvariant" )
  {
    _quantity = MTA_THIRDINVARIANT;
  }
  else if ( _quantity_string != "" )
  {
    mooseError("Unrecognized quantity in " + _name);
  }

  if (_index > 0 && _quantity != MTA_COMPONENT)
  {
    mooseError("Cannot define an index and a quantity in " + _name);
  }

  if (_index > -1 && _index > 5)
  {
    mooseError("MaterialTensorAux requires the index to be >= 0 and <= 5 OR < 0 (off).");
  }
}

Real
MaterialTensorAux::computeValue()
{
  Real value(0);
  const SymmTensor & tensor( _tensor[_qp] );
  if (_quantity == MTA_COMPONENT)
  {
    value = tensor.component(_index);
  }
  else if ( _quantity == MTA_VONMISES )
  {
    value = std::sqrt(0.5*(
                           std::pow(tensor.xx() - tensor.yy(), 2) +
                           std::pow(tensor.yy() - tensor.zz(), 2) +
                           std::pow(tensor.zz() - tensor.xx(), 2) + 6 * (
                           std::pow(tensor.xy(), 2) +
                           std::pow(tensor.yz(), 2) +
                           std::pow(tensor.zx(), 2))));
  }
  else if ( _quantity == MTA_PLASTICSTRAINMAG )
  {
    value = std::sqrt(2.0/3.0 * tensor.doubleContraction(tensor));
  }
  else if ( _quantity == MTA_HYDROSTATIC )
  {
    value = tensor.trace()/3;
  }
  else if ( _quantity == MTA_FIRSTINVARIANT )
  {
    value = tensor.trace();
  }
  else if ( _quantity == MTA_SECONDINVARIANT )
  {
    value =
      tensor.xx()*tensor.yy() +
      tensor.yy()*tensor.zz() +
      tensor.zz()*tensor.xx() -
      tensor.xy()*tensor.xy() -
      tensor.yz()*tensor.yz() -
      tensor.zx()*tensor.zx();
  }
  else if ( _quantity == MTA_THIRDINVARIANT )
  {
    value =
      tensor.xx()*tensor.yy()*tensor.zz() -
      tensor.xx()*tensor.yz()*tensor.yz() +
      tensor.xy()*tensor.zx()*tensor.yz() -
      tensor.xy()*tensor.xy()*tensor.zz() +
      tensor.zx()*tensor.xy()*tensor.yz() -
      tensor.zx()*tensor.zx()*tensor.yy();
  }
  else
  {
    mooseError("Internal logic error from " + name());
  }
  return value;
}


