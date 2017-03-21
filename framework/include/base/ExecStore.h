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

#include "MooseTypes.h"
#include "libmesh/libmesh_base.h"

/**
 * The class for storing warehouses that holds objects that are being
 * evaluated at different times.
 *
 * Currently we can do only Post-processors and AuxKernels, but this
 * object will help to extend the other subsystems.
 */
template <typename T>
class ExecStore
{
public:
  ExecStore()
    : _obj_initial(libMesh::n_threads()),
      _obj_linear(libMesh::n_threads()),
      _obj_nonlinear(libMesh::n_threads()),
      _obj_timestep_end(libMesh::n_threads()),
      _obj_timestep_begin(libMesh::n_threads()),
      _obj_custom(libMesh::n_threads())
  {
  }

  /**
   * Parenthesis operator for accessing the warehouses for different times
   * @param type
   * @return
   */
  std::vector<T> & operator()(ExecFlagType type)
  {
    switch (type)
    {
      case EXEC_INITIAL:
        return _obj_initial;
      case EXEC_TIMESTEP_END:
        return _obj_timestep_end;
      case EXEC_TIMESTEP_BEGIN:
        return _obj_timestep_begin;
      case EXEC_NONLINEAR:
        return _obj_nonlinear;
      case EXEC_CUSTOM:
        return _obj_custom;
      case EXEC_LINEAR:
      default:
        return _obj_linear;
    }
  }

protected:
  /// executed once at the beginning of the simulation
  std::vector<T> _obj_initial;
  /// executed every residual evaluation
  std::vector<T> _obj_linear;
  /// executed every jacobian (nonlinear) evaluation
  std::vector<T> _obj_nonlinear;
  /// executed at the end of every time step
  std::vector<T> _obj_timestep_end;
  /// executed at the beginning of every time step
  std::vector<T> _obj_timestep_begin;
  /// executed at a custom time by the Executioner
  std::vector<T> _obj_custom;
};

#endif /* EXECSTORE_H */
