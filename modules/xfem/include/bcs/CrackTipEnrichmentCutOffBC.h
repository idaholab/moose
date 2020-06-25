//* This file is part of the MOOSE framework
//* https://www.mooseframework.org
//*
//* All rights reserved, see COPYRIGHT for full restrictions
//* https://github.com/idaholab/moose/blob/master/COPYRIGHT
//*
//* Licensed under LGPL 2.1, please see LICENSE for details
//* https://www.gnu.org/licenses/lgpl-2.1.html

#pragma once

#include "DirichletBC.h"
#include "CrackFrontDefinition.h"

/**
 * CrackTipEnrichmentCutOffBC is used in XFEM Crack Tip Enrichment to fix DOFs to zero for those
 * nodes with basis function supports that are far away from any crack tip.
 */
class CrackTipEnrichmentCutOffBC : public DirichletBC
{
public:
  static InputParameters validParams();

  CrackTipEnrichmentCutOffBC(const InputParameters & parameters);

protected:
  virtual bool shouldApply() override;

  const Real _cut_off_radius;

private:
  const CrackFrontDefinition & _crack_front_definition;
};
