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

// MOOSE includes
#include "EigenExecutioner.h"
#include "EigenProblem.h"
#include "Factory.h"
#include "MooseApp.h"
#include "NonlinearEigenSystem.h"

template <>
InputParameters
validParams<EigenExecutioner>()
{
  InputParameters params = validParams<Steady>();

  params.addPrivateParam<bool>("_use_eigen_executioner", true);

  return params;
}

EigenExecutioner::EigenExecutioner(const InputParameters & parameters)
  : Steady(parameters),
    _eigen_problem(*parameters.getCheckedPointerParam<EigenProblem *>(
        "_eigen_problem", "This might happen if you don't have a mesh"))
{
}
