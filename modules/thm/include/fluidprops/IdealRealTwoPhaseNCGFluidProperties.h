#ifndef IDEALREALTWOPHASENCGFLUIDPROPERTIES_H
#define IDEALREALTWOPHASENCGFLUIDPROPERTIES_H

#include "TwoPhaseNCGFluidProperties.h"

class IdealRealTwoPhaseNCGFluidProperties;

template <>
InputParameters validParams<IdealRealTwoPhaseNCGFluidProperties>();

/**
 * Fluid properties for 2-phase fluid with an arbitrary mixture of non-condensable gases
 */
class IdealRealTwoPhaseNCGFluidProperties : public TwoPhaseNCGFluidProperties
{
public:
  IdealRealTwoPhaseNCGFluidProperties(const InputParameters & parameters);
};

#endif /* IDEALREALTWOPHASENCGFLUIDPROPERTIES_H */
