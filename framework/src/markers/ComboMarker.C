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

#include "ComboMarker.h"

template<>
InputParameters validParams<ComboMarker>()
{
  InputParameters params = validParams<Marker>();
  params.addRequiredParam<std::vector<MarkerName> >("markers", "The Markers to combine.");
  return params;
}


ComboMarker::ComboMarker(const std::string & name, InputParameters parameters) :
    Marker(name, parameters),
    _names(parameters.get<std::vector<MarkerName> >("markers"))
{
  for(unsigned int i=0; i<_names.size(); i++)
    _markers.push_back(&getMarkerValue(_names[i]));
}

int
ComboMarker::computeElementMarker()
{
  // We start with COARSEN because it's _0_
  int marker_value = Elem::COARSEN;

  for(unsigned int i=0; i<_markers.size(); i++)
    marker_value = std::max(marker_value, (int)(*_markers[i])[0]);

  return marker_value;
}

