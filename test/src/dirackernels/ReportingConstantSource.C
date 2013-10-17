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

#include "ReportingConstantSource.h"

template<>
InputParameters validParams<ReportingConstantSource>()
{
  InputParameters params = validParams<DiracKernel>();
  params.addRequiredParam<Real>("value", "The value of the point source");
  params.addRequiredParam<std::vector<Real> >("point", "The x,y,z coordinates of the point");
  return params;
}

ReportingConstantSource::ReportingConstantSource(const std::string & name, InputParameters parameters) :
    DiracKernel(name, parameters),
    _value(getParam<Real>("value")),
    _point_param(getParam<std::vector<Real> >("point")),
    _reporter(declareReportableValue("dirac_reporter"))
{
  _p(0) = _point_param[0];

  if(_point_param.size() > 1)
  {
    _p(1) = _point_param[1];

    if(_point_param.size() > 2)
    {
      _p(2) = _point_param[2];
    }
  }
}

void
ReportingConstantSource::addPoints()
{
  // This function gets called just before the DiracKernel is evaluated
  // so this is a handy place to zero this out.
  _reporter = 0.0;

  addPoint(_p);
}

Real
ReportingConstantSource::computeQpResidual()
{
  // This is negative because it's a forcing function that has been brought over to the left side.
  Real flux = -_test[_i][_qp]*_value;

  _reporter += flux;

  return flux;
}
