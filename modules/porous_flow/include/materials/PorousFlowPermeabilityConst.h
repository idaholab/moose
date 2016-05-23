/****************************************************************/
/* MOOSE - Multiphysics Object Oriented Simulation Environment  */
/*                                                              */
/*          All contents are licensed under LGPL V2.1           */
/*             See LICENSE for full restrictions                */
/****************************************************************/

#ifndef POROUSFLOWPERMEABILITYCONST_H
#define POROUSFLOWPERMEABILITYCONST_H

#include "PorousFlowMaterialVectorBase.h"

//Forward Declarations
class PorousFlowPermeabilityConst;

template<>
InputParameters validParams<PorousFlowPermeabilityConst>();

/**
 * Material designed to provide the permeability tensor
 * which is assumed constant
 */
class PorousFlowPermeabilityConst : public PorousFlowMaterialVectorBase
{
public:
  PorousFlowPermeabilityConst(const InputParameters & parameters);

protected:
    virtual void initQpStatefulProperties();
    virtual void computeQpProperties();

  /// constant value of permeability tensor
  const RealTensorValue _input_permeability;

  /// permeability
  MaterialProperty<RealTensorValue> & _permeability;

  /// d(permeability)/d(PorousFlow variable) which are all zero in this case
  MaterialProperty<std::vector<RealTensorValue> > & _dpermeability_dvar;
};

#endif //POROUSFLOWPERMEABILITYCONST_H
