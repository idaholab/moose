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

#include "FrontSource.h"

template<>
InputParameters validParams<FrontSource>()
{
  InputParameters params = validParams<DiracKernel>();

  params.addParam<Real>("value", 1.0, "The value of the strength of the point source.");
  params.addRequiredParam<UserObjectName>("front_uo", "A TrackDiracFront UserObject that will be supplying the positions");
    
  return params;
}

FrontSource::FrontSource(const std::string & name, InputParameters parameters) :
    DiracKernel(name, parameters),
    _value(getParam<Real>("value")),
    _front_tracker(getUserObject<TrackDiracFront>("front_uo"))
{
}

void
FrontSource::addPoints()
{
  const std::vector<std::pair<Elem *, Point> > & points = _front_tracker.getDiracPoints();

  std::vector<std::pair<Elem *, Point> >::const_iterator i = points.begin();
  std::vector<std::pair<Elem *, Point> >::const_iterator end = points.end();

  // Add all of the points the front tracker found
  for(; i != end; ++i)
    addPoint(i->first, i->second);
}

Real
FrontSource::computeQpResidual()
{
  // This is negative because it's a forcing function that has been brought over to the left side.
  return -_test[_i][_qp]*_value;
}
