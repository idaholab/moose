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

#ifndef SPLIT_H
#define SPLIT_H
#include <vector>
#include "libmesh/petsc_macro.h"
#include "FEProblem.h"
#include "Restartable.h"

class Split :
  public MooseObject,
  public Restartable
{
 public:
  Split(const std::string& name, InputParameters params);
  virtual void setup(const std::string& prefix = "-");

#if defined(LIBMESH_HAVE_PETSC) && !PETSC_VERSION_LESS_THAN(3,3,0)
 protected:
  /// Which splitting to use
  enum SplittingType {
    SplittingTypeAdditive,
    SplittingTypeMultiplicative,
    SplittingTypeSymmetricMultiplicative,
    SplittingTypeSchur
  };

  FEProblem& _fe_problem;
  std::string _name;
  std::vector<NonlinearVariableName> _vars;
  std::vector<std::string> _blocks;
  std::vector<std::string> _sides;
  std::vector<std::string> _unsides;
  std::vector<std::string> _splitting;
  MooseEnum _splitting_type;
  MooseEnum _schur_type;
  MooseEnum _schur_pre;
  MooseEnum _schur_ainv;
  std::vector<std::string> _petsc_options;
  std::vector<std::string> _petsc_options_iname;
  std::vector<std::string> _petsc_options_value;
#endif // defined(LIBMESH_HAVE_PETSC) && !PETSC_VERSION_LESS_THAN(3,3,0)
};


#endif /* SPLIT_H */
