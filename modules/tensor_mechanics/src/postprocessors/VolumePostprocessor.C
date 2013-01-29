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

#include "VolumePostprocessor.h"

template<>
InputParameters validParams<VolumePostprocessor>()
{
  InputParameters params = validParams<ElementIntegral>();
//  params.addRequiredParam<std::string>("mat_prop","dummy variable for volume");
  return params;
}

VolumePostprocessor::VolumePostprocessor(const std::string & name, InputParameters parameters) :
    ElementIntegral(name, parameters)
    /*    _scalar(getMaterialProperty<Real>(getParam<std::string>("mat_prop"))) */

{}
void
VolumePostprocessor::threadJoin(const UserObject &y)
{
  const VolumePostprocessor & pps = dynamic_cast<const VolumePostprocessor &>(y);
  _integral_value += pps._integral_value;
}

Real
VolumePostprocessor::computeQpIntegral()
{
  return 1.0; //_scalar[_qp];
}
