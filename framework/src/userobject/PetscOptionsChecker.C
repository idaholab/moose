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
#include "PetscOptionsChecker.h"
#include "MooseApp.h"
#include "Executioner.h"
#include "PetscSupport.h"

template<>
InputParameters validParams<PetscOptionsChecker>()
{
  InputParameters params = validParams<GeneralUserObject>();
  params.addParam<bool>  ("petsc_option_check_err", false, "Specifies whether or not to throw out an error if there are unused petsc options \n");
  return params;
}

PetscOptionsChecker::PetscOptionsChecker(const InputParameters & parameters) :
    GeneralUserObject(parameters)
{
}

void
PetscOptionsChecker::execute()
{
  if(Moose::PetscSupport::isUnusedPetscOptions(_fe_problem))
  {
    std::cout<<"unused petsc options "<<std::endl;
  } else
  {
    std::cout<<" used all petsc options "<<std::endl;
  }
}
