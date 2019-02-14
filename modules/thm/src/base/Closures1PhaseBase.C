#include "Closures1PhaseBase.h"
#include "FlowModelSinglePhase.h"

template <>
InputParameters
validParams<Closures1PhaseBase>()
{
  InputParameters params = validParams<ClosuresBase>();
  return params;
}

Closures1PhaseBase::Closures1PhaseBase(const InputParameters & params) : ClosuresBase(params) {}
