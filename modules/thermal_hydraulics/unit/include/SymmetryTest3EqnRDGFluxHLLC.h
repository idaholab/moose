//* This file is part of the MOOSE framework
//* https://www.mooseframework.org
//*
//* All rights reserved, see COPYRIGHT for full restrictions
//* https://github.com/idaholab/moose/blob/master/COPYRIGHT
//*
//* Licensed under LGPL 2.1, please see LICENSE for details
//* https://www.gnu.org/licenses/lgpl-2.1.html

#pragma once

#include "SymmetryTest3EqnRDGFluxBase.h"
#include "ADNumericalFlux3EqnHLLC.h"

/**
 * Tests symmetry of the HLLC numerical flux for the 3-equation model.
 */
class SymmetryTest3EqnRDGFluxHLLC : public SymmetryTest3EqnRDGFluxBase
{
protected:
  virtual const ADNumericalFlux3EqnBase * createFluxObject() override
  {
    const std::string class_name = "ADNumericalFlux3EqnHLLC";
    InputParameters params = _factory.getValidParams(class_name);
    params.set<UserObjectName>("fluid_properties") = _fp_name;
    _fe_problem->addUserObject(class_name, class_name, params);
    return static_cast<const ADNumericalFlux3EqnBase *>(
        &_fe_problem->getUserObject<ADNumericalFlux3EqnHLLC>(class_name));
  }

  virtual std::vector<std::pair<std::vector<ADReal>, std::vector<ADReal>>>
  getPrimitiveSolutionPairs() const override;
};
