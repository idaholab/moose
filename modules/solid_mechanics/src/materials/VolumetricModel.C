/****************************************************************/
/*               DO NOT MODIFY THIS HEADER                      */
/* MOOSE - Multiphysics Object Oriented Simulation Environment  */
/*                                                              */
/*           (c) 2011 Battelle Energy Alliance, LLC             */
/*                   ALL RIGHTS RESERVED                        */
/*                                                              */
/*          Prepared by Battelle Energy Alliance, LLC           */
/*            Under Contract No. DE-AC07-05ID14517              */
/*            With the U. S. Department of Energy               */
/*                                                              */
/*            See COPYRIGHT for full restrictions               */
/****************************************************************/

#include "VolumetricModel.h"

template<>
InputParameters validParams<VolumetricModel>()
{
  return validParams<Material>();
}

VolumetricModel::VolumetricModel( const std::string & name,
                                  InputParameters & parameters ):
  Material( name, parameters )
{}

VolumetricModel::~VolumetricModel() {}
