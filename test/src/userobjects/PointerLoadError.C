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

#include "PointerLoadError.h"

template<>
InputParameters validParams<PointerLoadError>()
{
  InputParameters params = validParams<UserObject>();
  return params;
}


PointerLoadError::PointerLoadError(const std::string & name, InputParameters params) :
    GeneralUserObject(name, params),
    _pointer_data(declareRestartableData<Stupid *>("pointer_data"))
{
  _pointer_data = new Stupid;
  _pointer_data->_i = 1;
}

PointerLoadError::~PointerLoadError()
{
  delete _pointer_data;
}

void PointerLoadError::initialSetup()
{
  _pointer_data->_i = 2;
}

void PointerLoadError::timestepSetup()
{
  _pointer_data->_i += 1;
}

void
PointerLoadError::execute()
{
}

