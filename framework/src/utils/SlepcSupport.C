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

#ifndef SLEPCSUPPORT_H
#define SLEPCSUPPORT_H

#include "libmesh/libmesh_config.h"

#ifdef LIBMESH_HAVE_SLEPC

#include "SlepcSupport.h"
// MOOSE includes
#include "MultiMooseEnum.h"
#include "InputParameters.h"

namespace Moose
{
namespace SlepcSupport
{

  InputParameters
  getSlepcValidParams()
  {
    InputParameters params = emptyInputParameters();

    // We are solving a Non-Hermitian eigenvalue problem by default
    MooseEnum eigenvalue_problem_type("HEP NHEP GHEP GNHEP GHIEP","NHEP");
    params.addParam<MooseEnum>("eigenvalue_problem_type", eigenvalue_problem_type,
                               "HEP: Hermitian "
                               "NHEP: Non-Hermitian "
                               "GHEP: Gerneralized Hermitian "
                               "GNHEP: Generalized Non-Hermitian "
                               "GHIEP: Generalized indefinite Hermitian ");
    return params;
  }

} // namespace SlepcSupport
} // namespace moose

#endif // LIBMESH_HAVE_SLEPC

#endif // SLEPCSUPPORT_H
