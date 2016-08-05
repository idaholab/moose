/****************************************************************/
/* MOOSE - Multiphysics Object Oriented Simulation Environment  */
/*                                                              */
/*          All contents are licensed under LGPL V2.1           */
/*             See LICENSE for full restrictions                */
/****************************************************************/

#ifndef POROUSFLOWPERMEABILITYUNITY_H
#define POROUSFLOWPERMEABILITYUNITY_H

#include "DerivativeMaterialInterface.h"
#include "Material.h"
#include "PorousFlowDictator.h"

//Forward Declarations
class PorousFlowPermeabilityUnity;

template<>
InputParameters validParams<PorousFlowPermeabilityUnity>();

/**
 * Base class Material designed to provide the permeability tensor.
 * In this class permeability = 1 0 0  0 1 0  0 0 1
 */
class PorousFlowPermeabilityUnity : public DerivativeMaterialInterface<Material>
{
public:
  PorousFlowPermeabilityUnity(const InputParameters & parameters);

protected:
  virtual void computeQpProperties();

  /// The variable names UserObject for the Porous-Flow variables
  const PorousFlowDictator & _dictator;
  
  /// The number of variables
  const Real _num_var;

  /// quadpoint permeability
  MaterialProperty<RealTensorValue> & _permeability_qp;

  /// d(quadpoint permeability)/d(PorousFlow variable)
  MaterialProperty<std::vector<RealTensorValue> > & _dpermeability_qp_dvar;
};

#endif //POROUSFLOWPERMEABILITYUNITY_H
