/****************************************************************/
/* MOOSE - Multiphysics Object Oriented Simulation Environment  */
/*                                                              */
/*          All contents are licensed under LGPL V2.1           */
/*             See LICENSE for full restrictions                */
/****************************************************************/
#include "MaterialVectorAux.h"

template<>
InputParameters validParams<MaterialVectorAux>()
{
  MooseEnum quantities("length=1");

  InputParameters params = validParams<AuxKernel>();
  params.addRequiredParam<std::string>("vector", "The material vector name.");
  params.addParam<int>("index", -1, "The index into the tensor, from 0 to 2.");
  params.addParam<MooseEnum>("quantity", quantities, "A scalar quantity to compute: (only option is Length).");
  return params;
}

MaterialVectorAux::MaterialVectorAux( const InputParameters & parameters)
  :AuxKernel(parameters),
   _vector( getMaterialProperty<RealVectorValue>( getParam<std::string>("vector") ) ),
   _index( getParam<int>("index") ),
   _quantity_moose_enum( getParam<MooseEnum>("quantity") )
{
  if (_quantity_moose_enum.isValid())
  {
    if ( _index > 0 )
      mooseError("Cannot define an index and a quantity in " + name());
    else
      _quantity = MVA_ENUM(int(_quantity_moose_enum));
  }
  else
  {
    if ( _index < 0 )
      mooseError("Neither an index nor a quantity listed for " + name());
    else
      _quantity = MVA_COMPONENT;  // default
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
    value = vector.norm();
  }
  else
  {
    mooseError("Internal logic error from " + name());
  }
  return value;
}
