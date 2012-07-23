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

#ifndef MOOSEPRECONDITIONER_H
#define MOOSEPRECONDITIONER_H

#include "MooseObject.h"
// libMesh include
#include "numeric_vector.h"

//Forward declarations
namespace libMesh
{
  class MeshBase;
}

class FEProblem;
class MoosePreconditioner;


template<>
InputParameters validParams<MoosePreconditioner>();

/**
 * Base class for MOOSE preconditioners
 */
class MoosePreconditioner : public MooseObject
{
public:
  MoosePreconditioner(const std::string & name, InputParameters params);
  virtual ~MoosePreconditioner();

  /**
   * Helper function for copying values associated with variables in vectors from two different systems.
   */
  static void copyVarValues(MeshBase & mesh,
                     const unsigned int from_system, const unsigned int from_var, const NumericVector<Number> & from_vector,
                     const unsigned int to_system, const unsigned int to_var, NumericVector<Number> & to_vector);

protected:
  /// Subproblem this preconditioner is part of
  FEProblem & _fe_problem;
};

#endif /* MOOSEPRECONDITIONER_H */
