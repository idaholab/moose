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

#include "InternalVolume.h"

template <>
InputParameters validParams<InternalVolume>()
{
  InputParameters params = validParams<SideIntegral>();
  return params;
}

InternalVolume::InternalVolume(const std::string & name,
                               InputParameters parameters)
  : SideIntegral( name, parameters )
{}

Real
InternalVolume::computeQpIntegral()
{
  return -_q_point[_qp](0)*_normals[_qp](0);
}
