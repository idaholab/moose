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
  params.addRequiredParam<std::vector<FieldName> >("marker_fields", "The Marker field_names to combine.");
  return params;
}


ComboMarker::ComboMarker(const std::string & name, InputParameters parameters) :
    Marker(name, parameters),
    _field_names(parameters.get<std::vector<FieldName> >("marker_fields"))
{
  for(unsigned int i=0; i<_field_names.size(); i++)
    _other_fields.push_back(&getMarkerFieldValue(_field_names[i]));
}

int
ComboMarker::computeElementMarker()
{
  // We start with COARSEN because it's _0_
  int marker_value = Elem::COARSEN;

  for(unsigned int i=0; i<_other_fields.size(); i++)
    marker_value = std::max(marker_value, (int)(*_other_fields[i])[0]);

  return marker_value;
}

