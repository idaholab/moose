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

#include "RestartableTypesChecker.h"

template <>
InputParameters
validParams<RestartableTypesChecker>()
{
  InputParameters params = validParams<RestartableTypes>();
  return params;
}

RestartableTypesChecker::RestartableTypesChecker(const InputParameters & parameters)
  : RestartableTypes(parameters)
{
}

RestartableTypesChecker::~RestartableTypesChecker() {}

void
RestartableTypesChecker::initialSetup()
{
}

void
RestartableTypesChecker::timestepSetup()
{
}

void
RestartableTypesChecker::execute()
{
  // First we need to check that the data has been restored properly
  checkData();

  /**
   * For testing the packed range routines, we'll make sure that we can pack up several types
   * into a single string and send it as a packed range and successfully restore it.
   */

  // Buffers for parallel communication
  std::vector<std::string> send_buffers(1);
  std::vector<std::string> recv_buffers;

  // String streams for serialization and deserialization
  std::ostringstream oss;
  std::istringstream iss;

  /**
   * Serialize
   */
  dataStore(oss, _real_data, this);
  dataStore(oss, _vector_data, this);
  dataStore(oss, _vector_vector_data, this);
  dataStore(oss, _pointer_data, this);
  dataStore(oss, _custom_data, this);
  dataStore(oss, _set_data, this);
  dataStore(oss, _map_data, this);
  dataStore(oss, _dense_vector_data, this);
  dataStore(oss, _dense_matrix_data, this);

  send_buffers[0] = oss.str();

  /**
   * Communicate
   */
  recv_buffers.reserve(_app.n_processors());
  _communicator.allgather_packed_range(
      (void *)(NULL), send_buffers.begin(), send_buffers.end(), std::back_inserter(recv_buffers));

  if (recv_buffers.size() != _app.n_processors())
    mooseError("Error in sizes of communicated buffers");

  /**
   * Deserialize and check
   */
  for (unsigned int i = 0; i < recv_buffers.size(); ++i)
  {
    iss.str(recv_buffers[i]);
    // reset the stream state
    iss.clear();

    // Clear types (just to make sure we don't get any false positives in our testing)
    clearTypes();

    // Now load the values
    dataLoad(iss, _real_data, this);
    dataLoad(iss, _vector_data, this);
    dataLoad(iss, _vector_vector_data, this);
    dataLoad(iss, _pointer_data, this);
    dataLoad(iss, _custom_data, this);
    dataLoad(iss, _set_data, this);
    dataLoad(iss, _map_data, this);
    dataLoad(iss, _dense_vector_data, this);
    dataLoad(iss, _dense_matrix_data, this);
    dataLoad(iss, *this, this);

    // Finally confirm that the data is sane
    checkData();
  }
}

void
RestartableTypesChecker::checkData()
{
  if (_real_data != 3)
    mooseError("Error reading restartable Real expected 3 got ", _real_data);

  if (_vector_data.size() != 4)
    mooseError("Error reading restartable std::vector<Real> expected size 4 got ",
               _vector_data.size());

  for (unsigned int i = 0; i < _vector_data.size(); i++)
    if (_vector_data[i] != 3)
      mooseError("Error reading restartable std::vector<Real> expected 3 got ", _vector_data[i]);

  if (_vector_vector_data.size() != 4)
    mooseError("Error reading restartable std::vector<std::vector<Real> > expected size 4 got ",
               _vector_data.size());

  for (unsigned int i = 0; i < _vector_vector_data.size(); i++)
  {
    for (unsigned int j = 0; j < _vector_vector_data[i].size(); j++)
      if (_vector_vector_data[i][j] != 3)
        mooseError("Error reading restartable std::vector<std::vector<Real> > expected 3 got ",
                   _vector_vector_data[i][j]);
  }

  if (_pointer_data->_i != 3)
    mooseError("Error reading restartable pointer data!");

  if (_custom_data._i != 3)
    mooseError("Error reading restartable custom data!");

  if (_custom_with_context._i != 3)
    mooseError("Error reading restartable custom data with context!");

  if (_set_data.size() != 2)
    mooseError("Error reading restartable std::set expected size 2 got ", _set_data.size());

  for (std::set<Real>::iterator it = _set_data.begin(); it != _set_data.end(); ++it)
    if (*it != 1 && *it != 2)
      mooseError("Error reading restartable set data!");

  if (_map_data.size() != 2)
    mooseError("Error reading restartable std::map expected size 2 got ", _map_data.size());

  if (_map_data[1] != 2.2)
    mooseError("Error reading restartable map data!");

  if (_map_data[2] != 3.4)
    mooseError("Error reading restartable map data!");

  if (_dense_vector_data.size() != 3)
    mooseError("Error reading restartable DenseVector size");
  for (unsigned int i = 0; i < _dense_vector_data.size(); i++)
    if (_dense_vector_data(i) != static_cast<Real>(i + 1))
      mooseError("Error reading restartable DenseMatrix data!");

  if (_dense_matrix_data.m() != 2 || _dense_matrix_data.n() != 3)
    mooseError("Error reading restartable DenseMatrix size");
  for (unsigned int i = 0; i < _dense_matrix_data.m(); i++)
    for (unsigned int j = 0; j < _dense_matrix_data.n(); j++)
      if (_dense_matrix_data(i, j) != static_cast<Real>(i + j + 1))
        mooseError("Error reading restartable DenseMatrix data!");
}

void
RestartableTypesChecker::clearTypes()
{
  _real_data = 0;
  _vector_data.clear();
  _vector_vector_data.clear();
  _pointer_data->_i = 0;
  _custom_data._i = 0;
  _set_data.clear();
  _map_data.clear();
  _dense_vector_data.zero();
  _dense_matrix_data.zero();
}
