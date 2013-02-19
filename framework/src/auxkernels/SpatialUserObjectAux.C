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

#include "SpatialUserObjectAux.h"
#include "UserObject.h"

template<>
InputParameters validParams<SpatialUserObjectAux>()
{
  InputParameters params = validParams<AuxKernel>();
  params.addRequiredParam<UserObjectName>("user_object", "The UserObject UserObject to get values from.  Note that the UserObject _must_ implement the spatialValue() virtual function!");
  return params;
}

SpatialUserObjectAux::SpatialUserObjectAux(const std::string & name, InputParameters parameters) :
    AuxKernel(name, parameters),
    _user_object(getUserObjectBase("user_object"))
{
}

Real
SpatialUserObjectAux::computeValue()
{
  return _user_object.spatialValue(_current_elem->centroid());
}
