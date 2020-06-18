//* This file is part of the MOOSE framework
//* https://www.mooseframework.org
//*
//* All rights reserved, see COPYRIGHT for full restrictions
//* https://github.com/idaholab/moose/blob/master/COPYRIGHT
//*
//* Licensed under LGPL 2.1, please see LICENSE for details
//* https://www.gnu.org/licenses/lgpl-2.1.html

#pragma once

// Moose Includes
#include "DiracKernel.h"
#include "PenetrationLocator.h"

// Forward Declarations
enum class ContactModel;
enum class ContactFormulation;

class ContactPrimary : public DiracKernel
{
public:
  static InputParameters validParams();

  ContactPrimary(const InputParameters & parameters);

  virtual void timestepSetup() override;

  virtual void addPoints() override;
  void computeContactForce(PenetrationInfo * pinfo, bool update_contact_set);
  virtual Real computeQpResidual() override;
  virtual Real computeQpJacobian() override;

  virtual void updateContactStatefulData();

protected:
  Real nodalArea(PenetrationInfo & pinfo);
  Real getPenalty(PenetrationInfo & pinfo);

  const unsigned int _component;
  const ContactModel _model;
  const ContactFormulation _formulation;
  const bool _normalize_penalty;
  PenetrationLocator & _penetration_locator;

  const Real _penalty;
  const Real _friction_coefficient;
  const Real _tension_release;
  const Real _capture_tolerance;

  NumericVector<Number> & _residual_copy;

  std::map<Point, PenetrationInfo *> _point_to_info;

  const unsigned int _mesh_dimension;

  std::vector<unsigned int> _vars;

  MooseVariable * _nodal_area_var;
  SystemBase & _aux_system;
  const NumericVector<Number> * const _aux_solution;
};
