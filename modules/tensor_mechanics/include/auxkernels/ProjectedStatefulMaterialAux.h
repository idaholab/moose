//* This file is part of the MOOSE framework
//* https://www.mooseframework.org
//*
//* All rights reserved, see COPYRIGHT for full restrictions
//* https://github.com/idaholab/moose/blob/master/COPYRIGHT
//*
//* Licensed under LGPL 2.1, please see LICENSE for details
//* https://www.gnu.org/licenses/lgpl-2.1.html

#pragma once

#include "AuxKernel.h"
#include "IndexableProperty.h"

template <bool is_ad>
class ProjectedStatefulMaterialAuxTempl : public AuxKernel
{
public:
  static InputParameters validParams();

  ProjectedStatefulMaterialAuxTempl(const InputParameters & parameters);

  virtual void initialSetup() override;
  virtual void subdomainSetup() override;

protected:
  virtual Real computeValue() override;

  const IndexableProperty<AuxKernel, is_ad> _prop;
  const SubdomainID & _current_subdomain_id;

  std::set<MaterialBase *> _all_materials;
  std::vector<MaterialBase *> _active_materials;
};

typedef ProjectedStatefulMaterialAuxTempl<false> ProjectedStatefulMaterialAux;
typedef ProjectedStatefulMaterialAuxTempl<true> ADProjectedStatefulMaterialAux;
