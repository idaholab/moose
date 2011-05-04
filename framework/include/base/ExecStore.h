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

#ifndef EXECSTORE_H
#define EXECSTORE_H

#include <vector>

#include "libmesh_common.h"

/// Execution flags - when is the oject executed/evaluated
enum ExecFlagType {
  EXEC_INITIAL,                 /// Object is evaluated only once at the beginning of the simulation
  EXEC_RESIDUAL,                /// Object is evaluated in every residual computation
  EXEC_JACOBIAN,                /// Object is evaluated in every jacobian computation
  EXEC_TIMESTEP                 /// Object is evaluated every time step
};


/**
 * The class for storing warehouses that holds objects that are being evaluated at different times
 *
 * Currently we can do only Post-processors and AuxKernels, but this object will help to extend the other subsystems.
 */
template<typename T>
class ExecStore
{
public:
  ExecStore() :
      _obj_init(libMesh::n_threads()),
      _obj_res(libMesh::n_threads()),
      _obj_jac(libMesh::n_threads()),
      _obj_timestep(libMesh::n_threads())
  {
  }

  /// Parenthesis operator for accessing the warehouses for different times
  std::vector<T> &
  operator()(ExecFlagType type)
  {
    switch (type)
    {
    case EXEC_INITIAL: return _obj_init;
    case EXEC_TIMESTEP: return _obj_timestep;
    case EXEC_JACOBIAN: return _obj_jac;
    case EXEC_RESIDUAL:
    default:
      return _obj_res;
    }
  }

protected:
  std::vector<T> _obj_init;                     ///< executed once at the beginning of the simulation
  std::vector<T> _obj_res;                      ///< executed every residual evaluation
  std::vector<T> _obj_jac;                      ///< executed every jacobian evaluation
  std::vector<T> _obj_timestep;                 ///< executed at the end of every time step
};

#endif /* EXECSTORE_H */
