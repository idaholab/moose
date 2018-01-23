#include "FaceFaceConstraint.h"

template <>
InputParameters
validParams<FaceFaceConstraint>()
{
  return validParams<MortarConstraint>();
}
