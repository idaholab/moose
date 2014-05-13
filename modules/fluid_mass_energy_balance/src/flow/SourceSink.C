/****************************************************************/
/*             DO NOT MODIFY OR REMOVE THIS HEADER              */
/*          FALCON - Fracturing And Liquid CONvection           */
/*                                                              */
/*       (c) pending 2012 Battelle Energy Alliance, LLC         */
/*                   ALL RIGHTS RESERVED                        */
/*                                                              */
/*          Prepared by Battelle Energy Alliance, LLC           */
/*            Under Contract No. DE-AC07-05ID14517              */
/*            With the U. S. Department of Energy               */
/*                                                              */
/*            See COPYRIGHT for full restrictions               */
/****************************************************************/

#include "SourceSink.h"

template<>
InputParameters validParams<SourceSink>()
{
  InputParameters params = validParams<Kernel>();
  params.addRequiredParam<Real>("value", "The value of the point source");
  params.addRequiredParam<std::vector<Real> >("point", "The x,y,z coordinates of the point");
  params.addRequiredParam<std::vector<Real> >("size",  "The x,y,z sizes of source or sink influence zone");
  return params;
}

SourceSink::SourceSink(const std::string & name, InputParameters parameters)

    :Kernel(name,parameters),
     _value(getParam<Real>("value")),
     _point_param(getParam<std::vector<Real> >("point")),
     _range_param(getParam<std::vector<Real> >("size"))
{
  _p(0) = _point_param[0];

  if (_point_param.size() > 1)
  {
    _p(1) = _point_param[1];

    if (_point_param.size() > 2)
    {
      _p(2) = _point_param[2];
    }
  }
}

Real SourceSink::computeQpResidual()
{
  Real xmin = _p(0) - _range_param[0]/2.0;
  Real xmax = _p(0) + _range_param[0]/2.0;

  Real ymin = _p(1) - _range_param[1]/2.0;
  Real ymax = _p(1) + _range_param[1]/2.0;

  Real zmin = 0.0;
  Real zmax = 0.0;

  if (_point_param.size() > 2)
  {
    zmin = _p(2) - _range_param[2]/2.0;
    zmax = _p(2) + _range_param[2]/2.0;
  }

//  Moose::out << "_E=" <<xmax<<" "<<ymax<<" "<<zmax<<"\n";
//  Moose::out << "_e=" <<xmin<<" "<<ymin<<" "<<zmin<<"\n";

  if (_point_param.size() > 2)
  {

    if (_q_point[_qp](0) >= xmin && _q_point[_qp](0) <= xmax &&
        _q_point[_qp](1) >= ymin && _q_point[_qp](1) <= ymax &&
        _q_point[_qp](2) >= zmin && _q_point[_qp](2) <= zmax )
    {


      Real strength = -_test[_i][_qp]*_value/_range_param[0]/_range_param[1]/_range_param[2];
      //    Real strength2 = _value/_range_param[0]/_range_param[1]/_range_param[2];
      // if (strength !=0)
      //   Moose::out << "_E= " <<_q_point[_qp](0)<<" "<<_q_point[_qp](1)<<" "<<_q_point[_qp](2)<<" "<<_value<< " " <<strength2<<"\n";

      return strength;
      //  return -_test[_i][_qp]*_value/_range_param[0]/_range_param[1]/_range_param[2];
    }

    else
    {
      return 0.0;
    }

  }
  else
  {
    if (_q_point[_qp](0) >= xmin && _q_point[_qp](0) <= xmax &&
        _q_point[_qp](1) >= ymin && _q_point[_qp](1) <= ymax)
      return -_test[_i][_qp]*_value/_range_param[0]/_range_param[1];
    else
      return 0.0;
  }

}

Real SourceSink::computeQpJacobian()
{
  return 0.0;
}
