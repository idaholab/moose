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
  params.addRequiredParam<std::vector<Real> >("point", "The x,y,z coordinates of the point");
  params.addRequiredCoupledVar("shared", "Constant auxilary variable for storing the total flux");
  params.addParam<Real>("factor", 1, "The multiplier for the shared source value");
  return params;
}

ReportingConstantSource::ReportingConstantSource(const std::string & name, InputParameters parameters) :
    DiracKernel(name, parameters),
    _shared_var(coupledScalarValue("shared")),
    _point_param(getParam<std::vector<Real> >("point")),
    _factor(getParam<Real>("factor"))
{
  _p(0) = _point_param[0];

  if (_point_param.size() > 1)
  {
    _p(1) = _point_param[1];

    if (_point_param.size() > 2)
      _p(2) = _point_param[2];
  }
}

void
ReportingConstantSource::addPoints()
{
  addPoint(_p);
}

Real
ReportingConstantSource::computeQpResidual()
{
  // This is negative because it's a forcing function that has been brought over to the left side.
  return -_test[_i][_qp]*_shared_var[0]*_factor;
}
