#pragma once

#include "SymmetryTest3EqnRDGFluxBase.h"
#include "NumericalFlux3EqnHLLC.h"

/**
 * Tests symmetry of the HLLC numerical flux for the 3-equation model.
 */
class SymmetryTest3EqnRDGFluxHLLC : public SymmetryTest3EqnRDGFluxBase
{
protected:
  virtual const NumericalFlux3EqnBase * createFluxObject() override
  {
    const std::string class_name = "NumericalFlux3EqnHLLC";
    InputParameters params = _factory.getValidParams(class_name);
    params.set<UserObjectName>("fluid_properties") = _fp_name;
    _fe_problem->addUserObject(class_name, class_name, params);
    return static_cast<const NumericalFlux3EqnBase *>(
        &_fe_problem->getUserObjectTempl<NumericalFlux3EqnHLLC>(class_name));
  }

  virtual std::vector<std::pair<std::vector<Real>, std::vector<Real>>>
  getPrimitiveSolutionPairs() const override;
};
