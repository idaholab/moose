#include "MaterialTensorAux.h"


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
   _tensor( getMaterialProperty<RealTensorValue>( getParam<std::string>("tensor") ) ),
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
  RealTensorValue & tensor( _tensor[_qp] );
  if (_quantity == MTA_COMPONENT)
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
    value = tensor(i,j);
  }
  else if ( _quantity == MTA_VONMISES )
  {
    value = std::sqrt(0.5*(
                        std::pow(tensor(0,0) - tensor(1,1), 2) +
                        std::pow(tensor(1,1) - tensor(2,2), 2) +
                        std::pow(tensor(2,2) - tensor(0,0), 2) + 6 * (
                          std::pow(tensor(0,1), 2) +
                          std::pow(tensor(1,2), 2) +
                          std::pow(tensor(2,0), 2))));
  }
  else if ( _quantity == MTA_HYDROSTATIC )
  {
    value = tensor.tr()/3;
  }
  else if ( _quantity == MTA_FIRSTINVARIANT )
  {
    value = tensor.tr();
  }
  else if ( _quantity == MTA_SECONDINVARIANT )
  {
    value =
      tensor(0,0)*tensor(1,1) +
      tensor(1,1)*tensor(2,2) +
      tensor(2,2)*tensor(0,0) -
      tensor(0,1)*tensor(1,0) -
      tensor(1,2)*tensor(2,1) -
      tensor(2,0)*tensor(0,2);
  }
  else if ( _quantity == MTA_THIRDINVARIANT )
  {
    value =
      tensor(0,0)*tensor(1,1)*tensor(2,2) -
      tensor(0,0)*tensor(1,2)*tensor(2,1) +
      tensor(0,1)*tensor(2,0)*tensor(1,2) -
      tensor(0,1)*tensor(1,0)*tensor(2,2) +
      tensor(0,2)*tensor(1,0)*tensor(2,1) -
      tensor(0,2)*tensor(2,0)*tensor(1,1);
  }
  else
  {
    mooseError("Internal logic error from " + name());
  }
  return value;
}


