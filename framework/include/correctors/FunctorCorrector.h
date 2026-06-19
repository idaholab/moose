//* This file is part of the MOOSE framework
//* https://mooseframework.inl.gov
//*
//* All rights reserved, see COPYRIGHT for full restrictions
//* https://github.com/idaholab/moose/blob/master/COPYRIGHT
//*
//* Licensed under LGPL 2.1, please see LICENSE for details
//* https://www.gnu.org/licenses/lgpl-2.1.html

#pragma once

#include "GeneralUserObject.h"
#include "MooseMesh.h"

class NonlinearSystemBase;

/**
 * Set (all or some) values of (one or more) nonlinear variable(s) using functor evaluations
 */
class FunctorCorrector : public GeneralUserObject, public NonADFunctorInterface
{
public:
  static InputParameters validParams();

  FunctorCorrector(const InputParameters & parameters);

  void initialize() override;
  void execute() override;
  void finalize() override;
  void threadJoin(const UserObject &) override {}

protected:
  /// reference to the mesh
  MooseMesh & _mesh;

  /// names of the variables to set using functors
  const std::vector<VariableName> & _var_names;
  /// internal ID numbers of the variables to renormalize
  std::vector<unsigned int> _var_numbers;

  /// Name of the functors
  const std::vector<MooseFunctorName> & _functor_names;

  /// Evaluation method (argument)
  const MooseEnum _functor_evaluation_technique;

  /// Pointers to the functors
  std::vector<const Moose::Functor<Real> *> _functors;
};
