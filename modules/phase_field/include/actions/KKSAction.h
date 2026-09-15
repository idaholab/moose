//* This file is part of the MOOSE framework
//* https://mooseframework.inl.gov
//*
//* All rights reserved, see COPYRIGHT for full restrictions
//* https://github.com/idaholab/moose/blob/master/COPYRIGHT
//*
//* Licensed under LGPL 2.1, please see LICENSE for details
//* https://www.gnu.org/licenses/lgpl-2.1.html

#pragma once

#include "Action.h"
#include "libmesh/fe_type.h"

/**
 * Sets up a multiphase, multicomponent KKS phase field model.
 */
class KKSAction : public Action
{
public:
  static InputParameters validParams();

  KKSAction(const InputParameters & params);
  virtual void act() override;

private:
  enum class PhaseConcentrationSolve
  {
    GLOBAL,
    NESTED
  };

  enum class PhaseConstraint
  {
    NONE,
    LAGRANGE
  };

  void addVariables();
  void addMaterials();
  void addKernels();

  std::vector<VariableName> phaseConcentrations(unsigned int component) const;
  std::vector<MaterialPropertyName> phaseConcentrationProperties() const;
  std::vector<VariableName> otherOrderParameters(unsigned int phase) const;
  std::vector<VariableName> phaseArguments(unsigned int phase,
                                           unsigned int excluded_component) const;
  std::vector<VariableName> acCoupledVariables(unsigned int phase) const;
  void applyKernelParameters(InputParameters & params) const;

  const PhaseConcentrationSolve _phase_concentration_solve;
  const PhaseConstraint _phase_constraint;
  const std::vector<std::string> & _phase_names;
  const std::vector<NonlinearVariableName> & _order_parameters;
  const std::vector<NonlinearVariableName> & _global_concentrations;
  const std::vector<MaterialPropertyName> & _free_energies;
  const std::vector<MaterialPropertyName> & _switching_functions;
  const std::vector<MaterialPropertyName> & _barrier_functions;
  const std::vector<Real> & _barrier_heights;
  const std::vector<MaterialPropertyName> & _concentration_mobilities;
  const std::vector<MaterialPropertyName> & _order_parameter_mobilities;
  const std::vector<MaterialPropertyName> & _kappas;
  const libMesh::FEType _fe_type;
  const Real _scaling;
};
