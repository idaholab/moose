//* This file is part of the MOOSE framework
//* https://www.mooseframework.org
//*
//* All rights reserved, see COPYRIGHT for full restrictions
//* https://github.com/idaholab/moose/blob/master/COPYRIGHT
//*
//* Licensed under LGPL 2.1, please see LICENSE for details
//* https://www.gnu.org/licenses/lgpl-2.1.html

#pragma once

#include "GeneralUserObject.h"

class Dummy
{
public:
  int _i;
};

class DummyNeedingContext
{
public:
  int _i;
};

template <>
inline void
dataStore(std::ostream & stream, Dummy *& v, void * context)
{
  dataStore(stream, v->_i, context);
}

template <>
inline void
dataLoad(std::istream & stream, Dummy *& v, void * context)
{
  dataLoad(stream, v->_i, context);
}

template <>
inline void
dataStore(std::ostream & stream, Dummy & v, void * context)
{
  dataStore(stream, v._i, context);
}

template <>
inline void
dataLoad(std::istream & stream, Dummy & v, void * context)
{
  dataLoad(stream, v._i, context);
}

template <>
inline void
dataStore(std::ostream & stream, DummyNeedingContext & v, void * context)
{
  int & context_int = *(static_cast<int *>(context));

  int value = v._i + context_int;

  dataStore(stream, value, context);
}

template <>
inline void
dataLoad(std::istream & stream, DummyNeedingContext & v, void * context)
{
  int & context_int = *(static_cast<int *>(context));

  int value = 0;

  dataLoad(stream, value, context);

  v._i = value - context_int;
}

/**
 * User Object for testing Restartable data types
 */
class RestartableTypes : public GeneralUserObject
{
public:
  static InputParameters validParams();

  RestartableTypes(const InputParameters & params);
  virtual ~RestartableTypes();

  virtual void initialSetup();
  virtual void timestepSetup();

  virtual void initialize(){};
  virtual void execute();
  virtual void finalize(){};

protected:
  int _context_int;
  Real & _real_data;
  std::vector<Real> & _vector_data;
  std::vector<std::vector<Real>> & _vector_vector_data;
  Dummy *& _pointer_data;
  Dummy & _custom_data;
  DummyNeedingContext & _custom_with_context;
  std::set<Real> & _set_data;
  std::map<unsigned int, Real> & _map_data;
  DenseVector<Real> & _dense_vector_data;
  DenseMatrix<Real> & _dense_matrix_data;
  libMesh::Parameters & _raw_parameters;
};
