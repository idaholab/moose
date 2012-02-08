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

#ifndef MOOSEVARIABLESCALAR_H
#define MOOSEVARIABLESCALAR_H

#include "Moose.h"
#include "MooseArray.h"
#include "ParallelUniqueId.h"
#include "MooseVariable.h"

// libMesh
#include "dof_map.h"

class Assembly;
class SubProblem;
class SystemBase;


/**
 * Class for scalar variables (they are different)
 *
 */
class MooseVariableScalar
{
public:
  MooseVariableScalar(unsigned int var_num, SystemBase & sys, Assembly & assembly);
  virtual ~MooseVariableScalar();

  void reinit();

  /// Get the variable number
  unsigned int number() { return _var_num; }

  /// Get the variable number
  const std::string & name();

  //
  VariableValue & sln() { return _u; }                  ///< The value of Lagrange multiplier

  const std::vector<unsigned int> & dofIndices() { return _dof_indices; }

  /// Set the scaling factor for this variable
  void scalingFactor(Real factor) { _scaling_factor = factor; }
  /// Get the scaling factor for this variable
  Real scalingFactor() { return _scaling_factor; }

protected:
  unsigned int _var_num;                                        ///< variable number (from libMesh)
  SubProblem & _subproblem;                                     ///< Problem this variable is part of
  SystemBase & _sys;                                            ///< System this variable is part of

  Assembly & _assembly;                                         ///< Assembly data
  const DofMap & _dof_map;                                      ///< DOF map
  std::vector<unsigned int> _dof_indices;                       ///< DOF indices

  VariableValue _u;                                             ///< The value of Lagrange multiplier

  Real _scaling_factor;                                                 ///< scaling factor for this variable
};

#endif /* MOOSEVARIABLESCALAR_H */
