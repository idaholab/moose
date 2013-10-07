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

#ifndef RESTARTABLETYPES_H
#define RESTARTABLETYPES_H

#include "GeneralUserObject.h"

class RestartableTypes;

template<>
InputParameters validParams<RestartableTypes>();

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

template<>
inline void
dataStore(std::ostream & stream, Dummy * & v, void * context)
{
  dataStore(stream, v->_i, context);
}

template<>
inline void
dataLoad(std::istream & stream, Dummy * & v, void * context)
{
  dataLoad(stream, v->_i, context);
}

template<>
inline void
dataStore(std::ostream & stream, Dummy & v, void * context)
{
  dataStore(stream, v._i, context);
}

template<>
inline void
dataLoad(std::istream & stream, Dummy & v, void * context)
{
  dataLoad(stream, v._i, context);
}

template<>
inline void
dataStore(std::ostream & stream, DummyNeedingContext & v, void * context)
{
  int & context_int = *(static_cast<int *>(context));

  int value = v._i + context_int;

  dataStore(stream, value, context);
}

template<>
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
  RestartableTypes(const std::string & name, InputParameters params);
  virtual ~RestartableTypes();

  virtual void initialSetup();
  virtual void timestepSetup();

  virtual void initialize() {};
  virtual void execute();
  virtual void finalize() {};

protected:
  int _context_int;
  Real & _real_data;
  std::vector<Real> & _vector_data;
  std::vector<std::vector<Real> > & _vector_vector_data;
  Dummy * & _pointer_data;
  Dummy & _custom_data;
  DummyNeedingContext & _custom_with_context;
};


#endif /* RESTARTABLETYPES_H */
