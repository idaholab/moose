//* This file is part of the MOOSE framework
//* https://www.mooseframework.org
//*
//* All rights reserved, see COPYRIGHT for full restrictions
//* https://github.com/idaholab/moose/blob/master/COPYRIGHT
//*
//* Licensed under LGPL 2.1, please see LICENSE for details
//* https://www.gnu.org/licenses/lgpl-2.1.html

#include "RestartableTypes.h"

using namespace libMesh;

registerMooseObject("MooseTestApp", RestartableTypes);

InputParameters
RestartableTypes::validParams()
{
  InputParameters params = GeneralUserObject::validParams();
  return params;
}

RestartableTypes::RestartableTypes(const InputParameters & params)
  : GeneralUserObject(params),
    _context_int(3),
    _real_data(declareRestartableData<Real>("real_data", 1)),
    _vector_data(declareRestartableData<std::vector<Real>>("vector_data")),
    _vector_vector_data(
        declareRestartableData<std::vector<std::vector<Real>>>("vector_vector_data")),
    _pointer_data(declareRestartableData<Dummy *>("pointer_data")),
    _custom_data(declareRestartableData<Dummy>("custom_data")),
    _custom_with_context(declareRestartableDataWithContext<DummyNeedingContext>(
        "custom_with_context", &_context_int)),
    _set_data(declareRestartableData<std::set<Real>>("set_data")),
    _map_data(declareRestartableData<std::map<unsigned int, Real>>("map_data")),
    _dense_vector_data(declareRestartableData<DenseVector<Real>>("dense_vector_data")),
    _dense_matrix_data(declareRestartableData<DenseMatrix<Real>>("dense_matrix_data")),
    _raw_parameters(declareRestartableData<Parameters>("raw_parameters"))
{
  _vector_data.resize(4, 1);
  _vector_vector_data.resize(4);

  for (unsigned int i = 0; i < _vector_vector_data.size(); i++)
  {
    _vector_vector_data[i].resize(4);

    for (unsigned int j = 0; j < _vector_vector_data[i].size(); j++)
      _vector_vector_data[i][j] = 1;
  }

  _pointer_data = new Dummy;

  _pointer_data->_i = 1;
  _custom_data._i = 1;
  _custom_with_context._i = 1;
}

RestartableTypes::~RestartableTypes() { delete _pointer_data; }

void
RestartableTypes::initialSetup()
{
  _real_data = 2;

  for (unsigned int i = 0; i < _vector_data.size(); i++)
    _vector_data[i] = 2;

  for (unsigned int i = 0; i < _vector_vector_data.size(); i++)
    for (unsigned int j = 0; j < _vector_vector_data[i].size(); j++)
      _vector_vector_data[i][j] = 2;

  _pointer_data->_i = 2;
  _custom_data._i = 2;
  _custom_with_context._i = 2;

  _set_data.insert(1);
  _set_data.insert(2);

  _map_data[1] = 2.2;
  _map_data[2] = 3.4;

  _dense_vector_data.resize(3);
  for (unsigned int i = 0; i < _dense_vector_data.size(); i++)
    _dense_vector_data(i) = static_cast<Real>(i);

  _dense_matrix_data.resize(2, 3);
  for (unsigned int i = 0; i < _dense_matrix_data.m(); i++)
    for (unsigned int j = 0; j < _dense_matrix_data.n(); j++)
      _dense_matrix_data(i, j) = static_cast<Real>(i + j);

  _raw_parameters.set<int>("i") = 42;
  _raw_parameters.set<unsigned int>("j") = 55;
  _raw_parameters.set<Real>("Pi") = 3.14159;
}

void
RestartableTypes::timestepSetup()
{
  _real_data += 1;

  for (unsigned int i = 0; i < _vector_data.size(); i++)
    _vector_data[i] += 1;

  for (unsigned int i = 0; i < _vector_vector_data.size(); i++)
    for (unsigned int j = 0; j < _vector_vector_data[i].size(); j++)
      _vector_vector_data[i][j] += 1;

  _pointer_data->_i += 1;
  _custom_data._i += 1;
  _custom_with_context._i += 1;

  for (unsigned int i = 0; i < _dense_vector_data.size(); i++)
    _dense_vector_data(i) += 1;

  for (unsigned int i = 0; i < _dense_matrix_data.m(); i++)
    for (unsigned int j = 0; j < _dense_matrix_data.n(); j++)
      _dense_matrix_data(i, j) += 1;
}

void
RestartableTypes::execute()
{
}
