//* This file is part of the MOOSE framework
//* https://www.mooseframework.org
//*
//* All rights reserved, see COPYRIGHT for full restrictions
//* https://github.com/idaholab/moose/blob/master/COPYRIGHT
//*
//* Licensed under LGPL 2.1, please see LICENSE for details
//* https://www.gnu.org/licenses/lgpl-2.1.html

#pragma once

#include "ThreadedNodeLoop.h"

// Forward declarations
template <typename T>
class MooseObjectWarehouse;
class AuxiliarySystem;

/**
 * This class evaluates a single mortar nodal aux kernel
 */
template <typename AuxKernelType>
class ComputeMortarNodalAuxBndThread
  : public ThreadedNodeLoop<ConstBndNodeRange, ConstBndNodeRange::const_iterator>
{
public:
  ComputeMortarNodalAuxBndThread(FEProblemBase & fe_problem,
                                 const MooseObjectWarehouse<AuxKernelType> & storage,
                                 BoundaryID bnd_id,
                                 std::size_t object_container_index);
  // Splitting Constructor
  ComputeMortarNodalAuxBndThread(ComputeMortarNodalAuxBndThread & x, Threads::split split);

  void onNode(ConstBndNodeRange::const_iterator & node_it) override;
  void join(const ComputeMortarNodalAuxBndThread & /*y*/);

protected:
  /// The auxiliary system
  AuxiliarySystem & _aux_sys;

  /// Storage object containing active AuxKernel objects
  const MooseObjectWarehouse<AuxKernelType> & _storage;

  /// The boundary ID for whose nodes we will evaluate the aux kernel on
  const BoundaryID _bnd_id;

  /// The index at which we should index the aux kernel containers, e.g. this data member helper
  /// ensures we get the correct mortar nodal aux kernel to evaluate
  const std::size_t _object_container_index;
};
