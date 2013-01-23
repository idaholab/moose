/****************************************************************/
/*               DO NOT MODIFY THIS HEADER                      */
/* MOOSE - Multiphysics Object Oriented Simulation Environment  */
/*                                                              */
/*           (c) 2010 Battelle Energy Alliance, LLC             */
/*                   ALL RIGHTS RESERVED                        */
/*                                                              */
/*          Prepared by Battelle Energy Alliance, LLC           */
/*            Under Contract No. DE-AC07-05ID14517              */
/*            With the U. S. Department of Energy               */
/*                                                              */
/*            See COPYRIGHT for full restrictions               */
/****************************************************************/

#include "MatPropUserObjectAux.h"
#include "MaterialPropertyUserObject.h"

template<>
InputParameters validParams<MatPropUserObjectAux>()
{
  InputParameters params = validParams<AuxKernel>();
  params.addRequiredParam<UserObjectName>("material_user_object", "The MaterialUserObject to retrieve values from.");
  return params;
}

MatPropUserObjectAux::MatPropUserObjectAux(const std::string & name, InputParameters parameters) :
    AuxKernel(name, parameters),
    _mat_uo(getUserObject<MaterialPropertyUserObject>("material_user_object"))
{
}

Real
MatPropUserObjectAux::computeValue()
{
  return _mat_uo.getElementalValue(_current_elem->id());
}
