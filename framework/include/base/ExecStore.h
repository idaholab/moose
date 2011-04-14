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

enum ExecFlagType {
  EXEC_RESIDUAL,
  EXEC_JACOBIAN,
  EXEC_TIMESTEP
};

template<typename T>
class ExecStore
{
public:
  ExecStore() :
      _obj_res(libMesh::n_threads()),
      _obj_jac(libMesh::n_threads()),
      _obj_timestep(libMesh::n_threads())
  {
  }

  std::vector<T> &
  operator()(ExecFlagType type)
  {
    switch (type)
    {
    case EXEC_TIMESTEP: return _obj_timestep;
    case EXEC_JACOBIAN: return _obj_jac;
    case EXEC_RESIDUAL:
    default:
      return _obj_res;
    }
  }

  void resize(int size);

protected:
  std::vector<T> _obj_res;                      // executed every residual evaluation
  std::vector<T> _obj_jac;                      // executed every jacobian evaluation
  std::vector<T> _obj_timestep;                 // executed at the end of every time step
};

#endif /* EXECSTORE_H */
