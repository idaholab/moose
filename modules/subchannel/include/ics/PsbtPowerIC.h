#ifndef PSBTPOWERIC_H
#define PSBTPOWERIC_H

#include "PsbtIC.h"

class PsbtPowerIC;

template <>
InputParameters validParams<PsbtPowerIC>();

//! Sets the linear heat rate for the PSBT 01-6232 fluid temperature benchmark.

class PsbtPowerIC: public PsbtIC
{
public:
  PsbtPowerIC(const InputParameters & params);

  Real value(const Point & p) override;

private:
  Real _ref_qprime;
};
#endif // PSBTPOWERIC_H
