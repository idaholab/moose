//* This file is part of the MOOSE framework
//* https://www.mooseframework.org
//*
//* All rights reserved, see COPYRIGHT for full restrictions
//* https://github.com/idaholab/moose/blob/master/COPYRIGHT
//*
//* Licensed under LGPL 2.1, please see LICENSE for details
//* https://www.gnu.org/licenses/lgpl-2.1.html

#pragma once

#include "KokkosNodalBCBase.h"

#include "libmesh/function_base.h"

namespace Moose::Kokkos
{

/**
 * Adapts a Kokkos Dirichlet-type nodal boundary condition's host-evaluable value
 * (NodalBCBase::hostValue()) to libMesh's FunctionBase interface, so that libMesh's own Dirichlet
 * constraint machinery can be registered against the boundary condition's own variable and used
 * as a value oracle for degrees of freedom the Kokkos device-side dispatch cannot itself reach by
 * boundary node (e.g. HIERARCHIC edge/face modes).
 */
class DirichletValueFunction : public libMesh::FunctionBase<Number>
{
public:
  /**
   * Constructor
   * @param bc The Kokkos nodal boundary condition to source the value from
   */
  DirichletValueFunction(const NodalBCBase & bc);

  virtual std::unique_ptr<libMesh::FunctionBase<Number>> clone() const override;

  virtual Number operator()(const libMesh::Point & p, const Real time = 0) override;

  virtual void
  operator()(const libMesh::Point & p, const Real time, DenseVector<Number> & output) override;

  /**
   * This function serves exactly one variable, the one its boundary condition is registered for,
   * but libMesh asks it for the component at that variable's offset within the whole system. So
   * every component asked for is that one variable's, whatever the index.
   */
  virtual Number component(unsigned int i, const libMesh::Point & p, Real time = 0) override;

private:
  /// The boundary condition this function sources its value from
  const NodalBCBase & _bc;
};

} // namespace Moose::Kokkos
