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

#ifndef COMBOMARKER_H
#define COMBOMARKER_H

#include "Marker.h"

// libmesh includes
#include "mesh_tools.h"

class ComboMarker;

/**
 * Combines multiple marker fields.  The most conservative wins.
 */
template<>
InputParameters validParams<ComboMarker>();

class ComboMarker : public Marker
{
public:
  ComboMarker(const std::string & name, InputParameters parameters);
  virtual ~ComboMarker(){};

protected:
  virtual int computeElementMarker();

  std::vector<MarkerName> _names;

  std::vector<VariableValue *> _markers;
};

#endif /* COMBOMARKER_H */
