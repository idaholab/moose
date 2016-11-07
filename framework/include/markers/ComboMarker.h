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

class ComboMarker;

/**
 * Combines multiple marker fields.  The most conservative wins.
 */
template <>
InputParameters validParams<ComboMarker>();

class ComboMarker : public Marker
{
public:
  ComboMarker(const InputParameters & parameters);

protected:
  virtual MarkerValue computeElementMarker() override;

  std::vector<MarkerName> _names;

  std::vector<const VariableValue *> _markers;
};

#endif /* COMBOMARKER_H */
