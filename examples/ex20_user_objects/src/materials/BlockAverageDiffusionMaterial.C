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

#include "BlockAverageDiffusionMaterial.h"

template<>
InputParameters validParams<BlockAverageDiffusionMaterial>()
{
  InputParameters params = validParams<Material>();

  // UserObjectName is the MOOSE type used for getting the name of a UserObject from the input file
  params.addRequiredParam<UserObjectName>("block_average_userobject", "The name of the UserObject that is going to be computing the average value of a variable on each block");

  return params;
}

BlockAverageDiffusionMaterial::BlockAverageDiffusionMaterial(const std::string & name,
                                 InputParameters parameters) :
    Material(name, parameters),

    // Declare that this material is going to provide a Real
    // valued property named "diffusivity" that Kernels can use.
    _diffusivity(declareProperty<Real>("diffusivity")),

    // When getting a UserObject from the input file pass the name
    // of the UserObjectName _parameter_
    // Note that getUserObject returns a _const reference_ of the type in < >
    _block_average_value(getUserObject<BlockAverageValue>("block_average_userobject"))
{}

void
BlockAverageDiffusionMaterial::computeQpProperties()
{
  // We will compute the diffusivity based on the average value of the variable on each block.

  // We'll get that value from a UserObject that is computing it for us.

  // To get the current block number we're going to query the "subdomain_id()" of the current element
  _diffusivity[_qp] = 0.5 * _block_average_value.averageValue(_current_elem->subdomain_id());
}
