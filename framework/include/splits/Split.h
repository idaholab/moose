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

class Split : public MooseObject {
 public:
  Split(const std::string& name, InputParameters params);
  virtual void setup(const std::string& prefix = "-");

#if defined(LIBMESH_HAVE_PETSC) && !PETSC_VERSION_LESS_THAN(3,3,0)
 protected:
  /// Which decomposition to use
  enum DecompositionType {
    DecompositionTypeAdditive,
    DecompositionTypeMultiplicative,
    DecompositionTypeSymmetricMultiplicative,
    DecompositionTypeSchur
  };
  DecompositionType
  getDecompositionType(const std::string& str);

  /// Which Schur factorization  to use.
  enum SchurType {
    SchurTypeDiag,
    SchurTypeUpper,
    SchurTypeLower,
    SchurTypeFull
  };
  SchurType
  getSchurType(const std::string& str);

  /// Which preconditioning matrix to use with S = D - CA^{-1}B
  /// 'Self' means use S to build the preconditioner.
  ///  limited choices here: PCNONE and PCLSC in PETSc
  /// 'D' means the lower-right block in decomposition J = [A B; C D]
  enum SchurPreconditioner {
    SchurPreconditionerSelf,
    SchurPreconditionerA11
  };
  SchurPreconditioner
  getSchurPreconditioner(const std::string& str);

  FEProblem& _fe_problem;
  std::string _name;
  std::vector<std::string> _vars;
  std::vector<std::string> _blocks;
  std::vector<std::string> _sides;
  std::vector<std::string> _unsides;
  std::vector<std::string> _decomposition;
  std::string              _decomposition_type;
  std::string              _schur_type;
  std::string              _schur_pre;
  std::vector<std::string> _petsc_options;
  std::vector<std::string> _petsc_options_iname;
  std::vector<std::string> _petsc_options_value;
#endif // defined(LIBMESH_HAVE_PETSC) && !PETSC_VERSION_LESS_THAN(3,3,0)
};


#endif /* SPLIT_H */
