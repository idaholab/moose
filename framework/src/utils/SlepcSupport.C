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
#include "EigenProblem.h"
#include "Conversion.h"

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
    // eigen solve type
    MooseEnum eigenvalue_solve_type("POWER ARNOLDI KRYLOVSCHUR JD");
    params.addParam<MooseEnum>("eigen_solve_type",      eigenvalue_solve_type,
                               "POWER: Power / Inverse / RQI "
                               "ARNOLDI: Arnoldi "
                               "KRYLOVSCHUR: Krylov-Schur "
                               "JD: Jacobi-Davidson ");

   return params;
  }

  void
  storeSlepcOptions(FEProblemBase & fe_problem, const InputParameters & params)
  {
    // if it is not an eigenvalue  problem, do nothing and return.
    if (!(dynamic_cast<EigenProblem *>(&fe_problem)))
      return;

    // can not use solve_type for eigenvalue problems
    if (params.isParamValid("solve_type"))
    {
      mooseError("Can not use solve_type for eigenvalue problems, please use eigen_solve_type instead \n");
    }
    // eigen solve type
    if (params.isParamValid("eigen_solve_type"))
    {
      // Extract the eigen solve type
      const std::string & eigen_solve_type = params.get<MooseEnum>("eigen_solve_type");
      fe_problem.solverParams()._eigen_solve_type = Moose::stringToEnum<Moose::EigenSolveType>(eigen_solve_type);
    }
  }

  void
  setEigenSolverOptions(SolverParams & solver_params)
  {
    // set SLEPC solver options
    switch (solver_params._eigen_solve_type)
    {
    case Moose::EST_POWER:
      Moose::PetscSupport::setSinglePetscOption("-eps_type", "power");
      break;

    case Moose::EST_ARNOLDI:
      Moose::PetscSupport::setSinglePetscOption("-eps_type", "arnoldi");
      break;

    case Moose::EST_KRYLOVSCHUR:
      Moose::PetscSupport::setSinglePetscOption("-eps_type", "krylovschur");
      break;

    case Moose::EST_JD:
      Moose::PetscSupport::setSinglePetscOption("-eps_type", "jd");
      break;

    default:
      mooseError("Unknown eigen solver type \n");
    }
  }

  void
  slepcSetOptions(FEProblemBase & problem)
  {
    Moose::PetscSupport::petscSetOptions(problem);
    setEigenSolverOptions(problem.solverParams());
    Moose::PetscSupport::addPetscOptionsFromCommandline();
  }

} // namespace SlepcSupport
} // namespace moose

#endif // LIBMESH_HAVE_SLEPC

#endif // SLEPCSUPPORT_H
