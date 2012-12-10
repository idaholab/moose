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

#ifndef UNIFORMMARKER_H
#define UNIFORMMARKER_H

#include "Marker.h"

// libmesh includes
#include "mesh_tools.h"

class UniformMarker;

template<>
InputParameters validParams<UniformMarker>();

class UniformMarker : public Marker
{
public:
  UniformMarker(const std::string & name, InputParameters parameters);
  virtual ~UniformMarker(){};

protected:
  virtual MarkerValue computeElementMarker();

  MarkerValue _mark;
};

#endif /* UNIFORMMARKER_H */
