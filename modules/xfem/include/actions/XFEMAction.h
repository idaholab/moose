//* This file is part of the MOOSE framework
//* https://www.mooseframework.org
//*
//* All rights reserved, see COPYRIGHT for full restrictions
//* https://github.com/idaholab/moose/blob/master/COPYRIGHT
//*
//* Licensed under LGPL 2.1, please see LICENSE for details
//* https://www.gnu.org/licenses/lgpl-2.1.html

#pragma once

#include "Action.h"
#include "UserObjectInterface.h"

class XFEMAction : public Action
{
public:
  static InputParameters validParams();

  XFEMAction(const InputParameters & params);

  virtual void act();

protected:
  std::vector<UserObjectName> _geom_cut_userobjects;
  std::string _xfem_qrule;
  std::string _order;
  std::string _family;
  bool _xfem_cut_plane;
  bool _xfem_use_crack_growth_increment;
  Real _xfem_crack_growth_increment;
  bool _use_crack_tip_enrichment;
  UserObjectName _crack_front_definition;
  std::vector<VariableName> _enrich_displacements;
  std::vector<VariableName> _displacements;
  std::vector<BoundaryName> _cut_off_bc;
  Real _cut_off_radius;
};
