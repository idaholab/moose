//* This file is part of the MOOSE framework
//* https://www.mooseframework.org
//*
//* All rights reserved, see COPYRIGHT for full restrictions
//* https://github.com/idaholab/moose/blob/master/COPYRIGHT
//*
//* Licensed under LGPL 2.1, please see LICENSE for details
//* https://www.gnu.org/licenses/lgpl-2.1.html

#pragma once

#include "MappingBase.h"
#include "ParallelSolutionStorage.h"
#include <slepcsvd.h>
#include "libmesh/parallel_object.h"
#include "libmesh/petsc_vector.h"

class PODMapping : public MappingBase
{
public:
  static InputParameters validParams();
  PODMapping(const InputParameters & parameters);

  ~PODMapping();

  virtual void buildMapping(const VariableName & vname) override;

  void map(const DenseVector<Real> & full_order_vector,
           std::vector<Real> & reduced_order_vector) const override;

  void map(const NumericVector<Number> & full_order_vector,
           std::vector<Real> & reduced_order_vector) const override;

  void map(const VariableName & vname,
           const unsigned int global_sample_i,
           std::vector<Real> & reduced_order_vector) const override;

  void inverse_map(const std::vector<Real> & reduced_order_vector,
                   std::vector<Real> & full_order_vector) const override;

protected:
  unsigned int determineNumberOfModes(const VariableName & vname,
                                      const std::vector<Real> & converged_evs);

  std::vector<VariableName> & _variable_names;
  const std::vector<unsigned int> _num_modes;
  const std::vector<Real> _energy_threshold;

  std::map<VariableName, std::vector<std::unique_ptr<DenseVector<Real>>>> & _basis_functions;
  std::map<VariableName, std::vector<Real>> & _eigen_values;

  const std::string _extra_slepc_options;

private:
  ParallelSolutionStorage * _parallel_storage;

  std::map<VariableName, SVD> _svds;
  std::map<VariableName, bool> _computed_svd;
};
