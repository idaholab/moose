#ifndef PSBTFLOWAREAIC_H
#define PSBTFLOWAREAIC_H

#include "PsbtIC.h"

class PsbtFlowAreaIC;

template <>
InputParameters validParams<PsbtFlowAreaIC>();

class PsbtFlowAreaIC: public PsbtIC
{
public:
  PsbtFlowAreaIC(const InputParameters & params);

  Real value(const Point & p) override;
};
#endif // PSBTFLOWAREAIC_H
