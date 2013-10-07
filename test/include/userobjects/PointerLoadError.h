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

#ifndef POINTERLOADERROR_H
#define POINTERLOADERROR_H

#include "GeneralUserObject.h"

class PointerLoadError;

template<>
InputParameters validParams<PointerLoadError>();

class Stupid
{
public:
  int _i;
};

/// Store but no Load!
template<>
inline void
dataStore(std::ostream & stream, Stupid * & v, void * context)
{
  dataStore(stream, v->_i, context);
}


class PointerLoadError : public GeneralUserObject
{
public:
  PointerLoadError(const std::string & name, InputParameters params);
  virtual ~PointerLoadError();

  virtual void initialSetup();
  virtual void timestepSetup();

  virtual void initialize() {};
  virtual void execute();
  virtual void finalize() {};

protected:
  Stupid * & _pointer_data;
};


#endif /* POINTERLOADERROR_H */
