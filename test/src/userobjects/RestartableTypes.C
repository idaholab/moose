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

#include "RestartableTypes.h"

template<>
InputParameters validParams<RestartableTypes>()
{
  InputParameters params = validParams<UserObject>();
  return params;
}


RestartableTypes::RestartableTypes(const std::string & name, InputParameters params) :
    GeneralUserObject(name, params),
    _real_data(declareRestartableData<Real>("real_data", 1)),
    _vector_data(declareRestartableData<std::vector<Real> >("vector_data")),
    _vector_vector_data(declareRestartableData<std::vector<std::vector<Real> > >("vector_vector_data")),
    _pointer_data(declareRestartableData<Dummy *>("pointer_data")),
    _custom_data(declareRestartableData<Dummy>("custom_data"))
{
  _vector_data.resize(4,1);
  _vector_vector_data.resize(4);

  for(unsigned int i=0; i<_vector_vector_data.size(); i++)
  {
    _vector_vector_data[i].resize(4);

    for(unsigned int j=0; j<_vector_vector_data[i].size(); j++)
      _vector_vector_data[i][j] = 1;
  }

  _pointer_data = new Dummy;

  _pointer_data->_i = 1;
  _custom_data._i = 1;
}

RestartableTypes::~RestartableTypes()
{
  delete _pointer_data;
}

void RestartableTypes::initialSetup()
{
  _real_data = 2;

  for(unsigned int i=0; i<_vector_data.size(); i++)
    _vector_data[i] = 2;

  for(unsigned int i=0; i<_vector_vector_data.size(); i++)
    for(unsigned int j=0; j<_vector_vector_data[i].size(); j++)
      _vector_vector_data[i][j] = 2;

  _pointer_data->_i = 2;
  _custom_data._i = 2;
}

void RestartableTypes::timestepSetup()
{
  _real_data += 1;

  for(unsigned int i=0; i<_vector_data.size(); i++)
    _vector_data[i] += 1;

  for(unsigned int i=0; i<_vector_vector_data.size(); i++)
    for(unsigned int j=0; j<_vector_vector_data[i].size(); j++)
      _vector_vector_data[i][j] += 1;

  _pointer_data->_i += 1;
  _custom_data._i += 1;
}

void
RestartableTypes::execute()
{
}

