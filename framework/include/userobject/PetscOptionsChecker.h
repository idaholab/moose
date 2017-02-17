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
#ifndef PETSCOPTIONSCHECKER_H
#define PETSCOPTIONSCHECKER_H

#include "GeneralUserObject.h"
#include "libmesh/fparser.hh"

class PetscOptionsChecker;

template<>
InputParameters validParams<PetscOptionsChecker>();

/**
  * This Userobject is used to check if any petsc options are unused at certain
  * stage
  */
class PetscOptionsChecker : public GeneralUserObject
{
public:
  PetscOptionsChecker(const InputParameters & parameters);

  virtual void initialize() override {}
  virtual void execute() override;
  virtual void finalize() override {}

};

#endif //PETSCOPTIONSCHECKER_H
