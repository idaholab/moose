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

template<>
inline void
dataStore(std::ostream & stream, Dummy * & v)
{
  dataStore(stream, v->_i);
}

template<>
inline void
dataLoad(std::istream & stream, Dummy * & v)
{
  dataLoad(stream, v->_i);
}

template<>
inline void
dataStore(std::ostream & stream, Dummy & v)
{
  dataStore(stream, v._i);
}

template<>
inline void
dataLoad(std::istream & stream, Dummy & v)
{
  dataLoad(stream, v._i);
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
  Real & _real_data;
  std::vector<Real> & _vector_data;
  std::vector<std::vector<Real> > & _vector_vector_data;
  Dummy * & _pointer_data;
  Dummy & _custom_data;
};


#endif /* RESTARTABLETYPES_H */
