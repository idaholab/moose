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

// MOOSE includes
#include "MoosePreconditioner.h"

// Forward declarations
class NonlinearSystemBase;
class FieldSplitPreconditioner;

template <>
InputParameters validParams<FieldSplitPreconditioner>();

/**
 * Implements a preconditioner designed to map onto PETSc's PCFieldSplit.
 */
class FieldSplitPreconditioner : public MoosePreconditioner
{
public:
  /**
   *  Constructor. Initializes SplitBasedPreconditioner data structures
   */
  FieldSplitPreconditioner(const InputParameters & parameters);

  /**
   * top split
   */
  std::vector<std::string> _top_split;

protected:
  /// The nonlinear system this FSP is associated with (convenience reference)
  NonlinearSystemBase & _nl;
};

#endif // FIELDSPLITPRECONDITIONER_H
