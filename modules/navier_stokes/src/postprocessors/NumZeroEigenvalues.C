//* This file is part of the MOOSE framework
//* https://www.mooseframework.org
//*
//* All rights reserved, see COPYRIGHT for full restrictions
//* https://github.com/idaholab/moose/blob/master/COPYRIGHT
//*
//* Licensed under LGPL 2.1, please see LICENSE for details
//* https://www.gnu.org/licenses/lgpl-2.1.html

#include "NumZeroEigenvalues.h"

#include <petscmat.h>
#include <slepceps.h>

registerMooseObject("NavierStokesApp", NumZeroEigenvalues);

InputParameters
NumZeroEigenvalues::validParams()
{
  InputParameters params = GeneralPostprocessor::validParams();
  params.addClassDescription("Report the number of zero eigenvalues of a matrix.");
  params.addRequiredParam<std::string>("mat", "The petsc binary mat file containing the matrix");
  params.addParam<Real>("zero_tol", 1e-8, "The tolerance for determining zero values");
  params.addParam<bool>("print", false, "Whether to print the eigensolves to the console");
  return params;
}

NumZeroEigenvalues::NumZeroEigenvalues(const InputParameters & parameters)
  : GeneralPostprocessor(parameters),
    _zero_tol(getParam<Real>("zero_tol")),
    _mat_name(getParam<std::string>("mat")),
    _print(getParam<bool>("print"))
{
}

void
NumZeroEigenvalues::initialize()
{
  PetscViewer matviewer;

  LibmeshPetscCallA(_communicator.get(), MatCreate(_communicator.get(), &_petsc_mat));
  LibmeshPetscCallA(
      _communicator.get(),
      PetscViewerBinaryOpen(_communicator.get(), _mat_name.c_str(), FILE_MODE_READ, &matviewer));
  LibmeshPetscCallA(_communicator.get(), MatLoad(_petsc_mat, matviewer));
  LibmeshPetscCallA(_communicator.get(), PetscViewerDestroy(&matviewer));

  _num_zero_eigenvalues = 0;
}

void
NumZeroEigenvalues::execute()
{
  if (_print)
    _console << std::endl << "Conducting eigen-solve for " << _mat_name << std::endl;

  PetscScalar kr, ki;
  PetscReal error, re, im;
  PetscInt nconv, nev, i;
  EPS eps;

  LibmeshPetscCallA(_communicator.get(), EPSCreate(_communicator.get(), &eps));
  LibmeshPetscCallA(_communicator.get(), EPSSetOperators(eps, _petsc_mat, nullptr));
  LibmeshPetscCallA(_communicator.get(), EPSSetType(eps, EPSLAPACK));
  LibmeshPetscCallA(_communicator.get(), EPSSolve(eps));
  LibmeshPetscCallA(_communicator.get(), EPSGetDimensions(eps, &nev, nullptr, nullptr));
  if (_print)
  {
    LibmeshPetscCallA(_communicator.get(),
                      PetscPrintf(_communicator.get(),
                                  " Number of requested eigenvalues: %" PetscInt_FMT "\n",
                                  nev));
  }

  LibmeshPetscCallA(_communicator.get(), EPSGetConverged(eps, &nconv));
  if (_print)
  {
    LibmeshPetscCallA(_communicator.get(),
                      PetscPrintf(_communicator.get(),
                                  " Number of converged eigenpairs: %" PetscInt_FMT "\n\n",
                                  nconv));
  }

  if (nconv > 0)
  {
    if (_print)
    {
      /*
         Display eigenvalues and relative errors
      */
      LibmeshPetscCallA(_communicator.get(),
                        PetscPrintf(_communicator.get(),
                                    "           k          ||Ax-kx||/||kx||\n"
                                    "   ----------------- ------------------\n"));
    }

    for (i = 0; i < nconv; i++)
    {
      /*
        Get converged eigenpairs: i-th eigenvalue is stored in kr (real part) and
        ki (imaginary part)
      */
      LibmeshPetscCallA(_communicator.get(), EPSGetEigenpair(eps, i, &kr, &ki, nullptr, nullptr));
      /*
         Compute the relative error associated to each eigenpair
      */
      LibmeshPetscCallA(_communicator.get(), EPSComputeError(eps, i, EPS_ERROR_RELATIVE, &error));

#if defined(PETSC_USE_COMPLEX)
      re = PetscRealPart(kr);
      im = PetscImaginaryPart(kr);
#else
      re = kr;
      im = ki;
#endif
      const bool zero_im = std::abs(im) < _zero_tol;
      const bool zero_re = std::abs(re) < _zero_tol;
      if (_print)
      {
        if (!zero_im)
          LibmeshPetscCallA(
              _communicator.get(),
              PetscPrintf(
                  PETSC_COMM_WORLD, " %9f%+9fi %12g\n", (double)re, (double)im, (double)error));
        else
          LibmeshPetscCallA(
              _communicator.get(),
              PetscPrintf(_communicator.get(), "   %12f       %12g\n", (double)re, (double)error));
      }
      if (zero_im && zero_re)
        ++_num_zero_eigenvalues;
    }

    if (_print)
    {
      LibmeshPetscCallA(_communicator.get(), PetscPrintf(_communicator.get(), "\n"));
    }
  }
}

void
NumZeroEigenvalues::finalize()
{
}

Real
NumZeroEigenvalues::getValue() const
{
  return _num_zero_eigenvalues;
}
