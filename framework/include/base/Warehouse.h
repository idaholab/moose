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

#ifndef WAREHOUSE_H
#define WAREHOUSE_H

#include <vector>

/**
 * Base class for all Warehouse containers. The warehouses in MOOSE hold
 * all of the MooseObjects. Returns various collections of objects when requested
 * and is responsible for deletion of those objects when the simulation ends.
 */
template <typename T>
class Warehouse
{
public:
  virtual ~Warehouse() {}

  /**
   * Get list of all kernels
   * @return The list of all active kernels
   */
  virtual const std::vector<T *> & all() const;

protected:
  /// All instances of objects (raw pointers)
  std::vector<T *> _all_objects;
};

template <typename T>
const std::vector<T *> &
Warehouse<T>::all() const
{
  return _all_objects;
}

#endif // WAREHOUSE_H
