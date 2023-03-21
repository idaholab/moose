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

class PODMapping : public MappingBase
{
public:
  static InputParameters validParams();
  PODMapping(const InputParameters & parameters);

  virtual void buildMapping(const VariableName & vname) override;

  void map(const DenseVector<Real> & full_order_vector,
           std::vector<Real> & reduced_order_vector) const override;

  void map(const NumericVector<Number> & full_order_vector,
           std::vector<Real> & reduced_order_vector) const override;

  void inverse_map(const std::vector<Real> & reduced_order_vector,
                   std::vector<Real> & full_order_vector) const override;

protected:
  std::map<VariableName, std::vector<std::unique_ptr<DenseVector<Real>>>> & _basis_functions;
  std::map<VariableName, std::vector<Real>> & _eigen_values;
};
