//* This file is part of the MOOSE framework
//* https://mooseframework.inl.gov
//*
//* All rights reserved, see COPYRIGHT for full restrictions
//* https://github.com/idaholab/moose/blob/master/COPYRIGHT
//*
//* Licensed under LGPL 2.1, please see LICENSE for details
//* https://www.gnu.org/licenses/lgpl-2.1.html

#pragma once

#include "LibmeshDirichletBCBase.h"

#include "libmesh/function_base.h"

/**
 * Adapts a LibmeshDirichletBCBase's prescribed value to libMesh's FunctionBase interface, so that
 * libMesh's own Dirichlet constraint machinery can source the boundary data it projects from a
 * MOOSE boundary condition.
 */
class LibmeshDirichletValueFunction : public libMesh::FunctionBase<Number>
{
public:
  /**
   * Constructor
   * @param bc The boundary condition to source the value from
   */
  LibmeshDirichletValueFunction(const LibmeshDirichletBCBase & bc);

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
  const LibmeshDirichletBCBase & _bc;
};
