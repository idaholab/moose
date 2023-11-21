//* This file is part of the MOOSE framework
//* https://www.mooseframework.org
//*
//* All rights reserved, see COPYRIGHT for full restrictions
//* https://github.com/idaholab/moose/blob/master/COPYRIGHT
//*
//* Licensed under LGPL 2.1, please see LICENSE for details
//* https://www.gnu.org/licenses/lgpl-2.1.html

#pragma once

#include "ElementVectorPostprocessor.h"
#include "NonADFunctorInterface.h"

/**
 * This MeshDivisionFunctorReductionVectorPostprocessor serves to integrate functors based
 * on the index of the elements in the mesh division
 */
class MeshDivisionFunctorReductionVectorPostprocessor : public ElementVectorPostprocessor,
                                                        public NonADFunctorInterface
{
public:
  static InputParameters validParams();
  MeshDivisionFunctorReductionVectorPostprocessor(const InputParameters & parameters);

  virtual void initialize() override;
  virtual void execute() override;
  virtual void finalize() override;
  virtual void threadJoin(const UserObject & uo) override;

  /// Return Integral values
  const std::vector<VectorPostprocessorValue *> & getReductions() const
  {
    return _functor_reductions;
  };

  virtual Real spatialValue(const Point & p) const override;

  enum ReductionEnum
  {
    INTEGRAL,
    AVERAGE,
    MIN,
    MAX
  };

protected:
  /// Reduction operation to be performed
  const MooseEnum _reduction;
  /// Number of functors to be integrated
  const unsigned int _nfunctors;
  /// Mesh division providing the division
  const MeshDivision & _mesh_division;
  /// Functors that are evaluated to create the reduction
  std::vector<const Moose::Functor<Real> *> _functors;
  /// Vectors holding functor reductions (integrals, averages, extrema..) over each mesh division
  std::vector<VectorPostprocessorValue *> _functor_reductions;
  /// Vectors holding the mesh division volumes
  std::vector<Real> _volumes;
};
