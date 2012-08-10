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

#ifndef MARKER_H
#define MARKER_H

#include "MooseObject.h"

class Marker;

template<>
InputParameters validParams<Marker>();

class Marker :
  public MooseObject
{
public:
  Marker(const std::string & name, InputParameters parameters);
  virtual ~Marker(){};

  virtual Real creatEV();

protected:

};

#endif /* MARKER_H */
