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
  InputParameters params = emptyInputParameters();
  params.addRequiredParam<std::vector<BoundaryName> >("boundary", "The list of boundary IDs from the mesh where this boundary condition applies");
  return params;
}

BoundaryRestrictableRequired::BoundaryRestrictableRequired(InputParameters & parameters) :
    BoundaryRestrictable(parameters)
{
}
