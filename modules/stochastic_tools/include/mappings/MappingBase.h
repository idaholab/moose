//* This file is part of the MOOSE framework
//* https://www.mooseframework.org
//*
//* All rights reserved, see COPYRIGHT for full restrictions
//* https://github.com/idaholab/moose/blob/master/COPYRIGHT
//*
//* Licensed under LGPL 2.1, please see LICENSE for details
//* https://www.gnu.org/licenses/lgpl-2.1.html

#pragma once

#include "StochasticToolsApp.h"
#include "MooseObject.h"
#include "RestartableModelInterface.h"

#include "libmesh/petsc_vector.h"
#include "libmesh/petsc_matrix.h"

class MappingBase : public MooseObject, public RestartableModelInterface
{
public:
  static InputParameters validParams();
  MappingBase(const InputParameters & parameters);

  virtual void buildMapping(const VariableName & vname) = 0;

  virtual void map(const VariableName & vname,
                   const DenseVector<Real> & full_order_vector,
                   std::vector<Real> & reduced_order_vector) const = 0;

  virtual void map(const NumericVector<Number> & full_order_vector,
                   std::vector<Real> & reduced_order_vector) const = 0;

  virtual void map(const VariableName & vname,
                   const unsigned int global_sample_i,
                   std::vector<Real> & reduced_order_vector) const = 0;

  virtual void inverse_map(const std::vector<Real> & reduced_order_vector,
                           std::vector<Real> & full_order_vector) const = 0;

  virtual const std::vector<VariableName> & getVariableNames() { return _variable_names; }

  virtual const DenseVector<Real> & basis(const VariableName & vname,
                                          const unsigned int base_i) = 0;

protected:
  std::vector<VariableName> & _variable_names;
};
