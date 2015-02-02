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

#ifndef MOOSEVARIABLEMAP_H
#define MOOSEVARIABLEMAP_H

#include "MooseError.h"
#include "FEProblem.h"

#include <vector>

/**
 * Custom container to hold maps form Moose Variable numbers to arbitrary other
 * objects. This container exploits the numbering scheme that MOOSE and libMesh use
 * for the non-linear and auxiliary variables and avoids tree or hash based mapping
 * in favor of a faster vector based lookup.
 */
template <typename T>
class MooseVariableMap {
public:
  /**
   * Map constructor. This uses the passed in FEProblem reference to
   * determine the number on non-linear and auxiliary variables and the
   * size of the underlying data storage to accomodate them all.
   * This allows us to do the bounds checking through asserts only!
   */
  MooseVariableMap(FEProblem & fe) : _data(2 * std::max(fe.getNonlinearSystem().nVariables(), fe.getAuxiliarySystem().nVariables())) {}
  MooseVariableMap(FEProblem & fe, T undefined) : _data(2 * std::max(fe.getNonlinearSystem().nVariables(), fe.getAuxiliarySystem().nVariables()), undefined) {}

  /**
   * Read-only access operator
   */
  T operator[] (unsigned int var) const {
    return _data[libMeshVarNumberRemap(var)];
  }

  /**
   * Insert method for the map
   */
  void insert(const std::pair<unsigned int, T> & p) {
    _data[libMeshVarNumberRemap(p.first)] = p.second;
  }

private:
  /**
   * This function remaps the the Non-linear (and Aux-Variable) numbers
   * to even (and odd) numbers starting at zero (and one).
   * This allows us to use a fairly dense vector to perform the direct lookup.
   */
  unsigned int libMeshVarNumberRemap(unsigned int var) const
  {
    const int b = static_cast<int>(var);
    const int i = b >= 0 ? b<<1 : (-b<<1)-1;
    mooseAssert (i < _data.size(), "Invalid MOOOSE Variable number passed to MooseVariableMap.");
    return i;
  }

  std::vector<T> _data;
};

#endif //MOOSEVARIABLEMAP_H
