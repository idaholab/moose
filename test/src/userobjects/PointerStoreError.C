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

#include "PointerStoreError.h"

template<>
InputParameters validParams<PointerStoreError>()
{
  InputParameters params = validParams<UserObject>();
  return params;
}


PointerStoreError::PointerStoreError(const std::string & name, InputParameters params) :
    GeneralUserObject(name, params),
    _pointer_data(declareRestartableData<ReallyDumb *>("pointer_data"))
{
  _pointer_data = new ReallyDumb;
  _pointer_data->_i = 1;
}

PointerStoreError::~PointerStoreError()
{
  delete _pointer_data;
}

void PointerStoreError::initialSetup()
{
  _pointer_data->_i = 2;
}

void PointerStoreError::timestepSetup()
{
  _pointer_data->_i += 1;
}

void
PointerStoreError::execute()
{
}

