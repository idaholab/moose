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

#ifndef FIELDSPLITPRECONDITIONER_H
#define FIELDSPLITPRECONDITIONER_H

#include "libmesh/petsc_macro.h"
#if defined(LIBMESH_HAVE_PETSC) && !PETSC_VERSION_LESS_THAN(3,3,0)

//Global includes
#include <vector>
// MOOSE includes
#include "MoosePreconditioner.h"
//libMesh includes
#include "libmesh/preconditioner.h"
#include "libmesh/system.h"
#include "libmesh/linear_implicit_system.h"


class FEProblem;
class NonlinearSystem;
class FieldSplitPreconditioner;

template<>
InputParameters validParams<FieldSplitPreconditioner>();

/**
 * Implements a preconditioner designed to map onto PETSc's PCFieldSplit.
 */
class FieldSplitPreconditioner :
    public MoosePreconditioner
{
public:
  /**
   *  Constructor. Initializes FieldSplitPreconditioner data structures
   */
  FieldSplitPreconditioner (const std::string & name, InputParameters params);

  /**
   * Destructor.
   */
  virtual ~FieldSplitPreconditioner();

  /**
   * Add a diagonal system + type of preconditioning
   */
  void addSplit(unsigned int var);

  /**
   * Sets up DMMoose internal data structures that define the splits.
   *
   */
  void setup();

protected:
  /// The nonlinear system this FSP is associated with (convenience reference)
  NonlinearSystem & _nl;
  /// Which FieldSplit type to use
  enum SplitType {
    SplitTypeAdditive,
    SplitTypeMultiplicative,
    SplitTypeSymmetricMultiplicative,
    SplitTypeSchur
  };
  SplitType
  getSplitType(const std::string& str);
  SplitType _split_type;

  /// Which FieldSplit Schur factorization style to use.
  enum SchurType {
    SchurTypeDiag,
    SchurTypeUpper,
    SchurTypeLower,
    SchurTypeFull
  };
  SchurType
  getSchurType(const std::string& str);
  SchurType _schur_type;

  /// Which preconditioning matrix to use with S = D - CA^{-1}B
  /// 'Self' means use S to build the preconditioner.
  ///  limited choices here: PCNONE and PCLSC in PETSc
  /// 'D' means the lower-right block in decomposition J = [A B; C D]
  enum SchurPreconditioner {
    SchurPreconditionerSelf,
    SchurPreconditionerD
  };
  SchurPreconditioner
  getSchurPreconditioner(const std::string& str);
  SchurPreconditioner _schur_preconditioner;
};
#endif // defined(LIBMESH_HAVE_PETSC) && !PETSC_VERSOION_LESS_THAN(3,3,0)
#endif //FIELDSPLITPRECONDITIONER_H
