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

#include "BoundaryRestrictableRequired.h"

template<>
InputParameters validParams<BoundaryRestrictableRequired>()
{
  // Create instance of InputParameters
  InputParameters params = emptyInputParameters();

  // Create user-facing 'boundary' input for restricting inheriting object to boundaries
  params.addRequiredParam<std::vector<BoundaryName> >("boundary", "The list of boundary IDs from the mesh where this boundary condition applies");

  // Create a private parameter for storing the boundary IDs
  params.addPrivateParam<std::vector<BoundaryID> >("_boundary_ids", std::vector<BoundaryID>());

  // A parameter for disabling error message for objects restrictable by boundary and block,
  // if the parameter is valid it was already set so don't do anything
  if (!params.isParamValid("_dual_restrictable"))
    params.addPrivateParam<bool>("_dual_restrictable", false);

  return params;
}


BoundaryRestrictableRequired::BoundaryRestrictableRequired(const std::string name, InputParameters & parameters) :
    BoundaryRestrictable(name, parameters)
{
}
