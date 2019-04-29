#pragma once

#include "ConsistencyTest3EqnRDGFluxBase.h"
#include "NumericalFlux3EqnCentered.h"

/**
 * Tests consistency of the centered numerical flux for the 3-equation model.
 */
class ConsistencyTest3EqnRDGFluxCentered : public ConsistencyTest3EqnRDGFluxBase
{
protected:
  virtual const NumericalFlux3EqnBase * createFluxObject() override
  {
    const std::string class_name = "NumericalFlux3EqnCentered";
    InputParameters params = _factory.getValidParams(class_name);
    params.set<UserObjectName>("fluid_properties") = _fp_name;
    _fe_problem->addUserObject(class_name, class_name, params);
    return static_cast<const NumericalFlux3EqnBase *>(
        &_fe_problem->getUserObject<NumericalFlux3EqnCentered>(class_name));
  }
};
