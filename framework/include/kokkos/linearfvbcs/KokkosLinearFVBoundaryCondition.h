//* This file is part of the MOOSE framework
//* https://www.mooseframework.org
//*
//* All rights reserved, see COPYRIGHT for full restrictions
//* https://github.com/idaholab/moose/blob/master/COPYRIGHT
//*
//* Licensed under LGPL 2.1, please see LICENSE for details
//* https://www.gnu.org/licenses/lgpl-2.1.html

#pragma once

#include "KokkosLinearSystemContributionObject.h"
#include "KokkosVariable.h"

#include "BoundaryRestrictableRequired.h"
#include "DistributionInterface.h"
#include "GeometricSearchInterface.h"
#include "MooseLinearVariableFV.h"
#include "MooseVariableDependencyInterface.h"
#include "NonADFunctorInterface.h"
#include "FaceArgInterface.h"

namespace Moose::Kokkos
{

class LinearFVBoundaryCondition : public LinearSystemContributionObject,
                                  public BoundaryRestrictableRequired,
                                  public DistributionInterface,
                                  public GeometricSearchInterface,
                                  public MooseVariableDependencyInterface,
                                  public NonADFunctorInterface,
                                  public FaceArgProducerInterface
{
public:
  static InputParameters validParams();

  LinearFVBoundaryCondition(const InputParameters & parameters);
  LinearFVBoundaryCondition(const LinearFVBoundaryCondition & object);

  virtual const MooseLinearVariableFV<Real> & variable() const { return _var; }
  virtual bool hasFaceSide(const FaceInfo & fi, const bool fi_elem_side) const override;

  virtual void computeRightHandSide() = 0;
  virtual void computeMatrix() = 0;

protected:
  MooseLinearVariableFV<Real> & _var;
  Variable _kokkos_var;

  const unsigned int _var_num;
  const unsigned int _sys_num;
};

} // namespace Moose::Kokkos
