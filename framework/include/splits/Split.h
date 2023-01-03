//* This file is part of the MOOSE framework
//* https://www.mooseframework.org
//*
//* All rights reserved, see COPYRIGHT for full restrictions
//* https://github.com/idaholab/moose/blob/master/COPYRIGHT
//*
//* Licensed under LGPL 2.1, please see LICENSE for details
//* https://www.gnu.org/licenses/lgpl-2.1.html

#pragma once

// MOOSE includes
#include "MooseEnum.h"
#include "MooseObject.h"
#include "PetscSupport.h"
#include "Restartable.h"

// Forward declarations
class FEProblemBase;

/**
 * Base class for split-based preconditioners.
 */
class Split : public MooseObject, public Restartable
{
public:
  static InputParameters validParams();

  Split(const InputParameters & parameters);
  virtual ~Split() = default;

  virtual void setup(const std::string & prefix = "-");

protected:
  /// Which splitting to use
  enum SplittingType
  {
    SplittingTypeAdditive,
    SplittingTypeMultiplicative,
    SplittingTypeSymmetricMultiplicative,
    SplittingTypeSchur
  };

  FEProblemBase & _fe_problem;

  /// "Variables Split operates on
  std::vector<NonlinearVariableName> _vars;

  ///@{
  /// Block and bounrdary restrictions for the split
  std::vector<SubdomainName> _blocks;
  std::vector<BoundaryName> _sides;
  std::vector<BoundaryName> _unsides;
  ///@}

  /// Split subsystem list
  std::vector<std::string> _splitting;

  ///@{
  /// Splitting type and (in case of Schur split) options
  MooseEnum _splitting_type;
  MooseEnum _schur_type;
  MooseEnum _schur_pre;
  ///@}

  /// Additional PETSc options
  Moose::PetscSupport::PetscOptions _petsc_options;
};
