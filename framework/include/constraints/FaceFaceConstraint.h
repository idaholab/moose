#ifndef FACEFACECONSTRAINT_H
#define FACEFACECONSTRAINT_H

#include "MortarConstraint.h"

class FaceFaceConstraint;

template <>
InputParameters validParams<FaceFaceConstraint>();

/**
 * This is a deprecated object!  Use MortarConstraint instead!
 */
class FaceFaceConstraint : public MortarConstraint
{
public:
  FaceFaceConstraint(const InputParameters & params) : MortarConstraint(params)
  {
    mooseDeprecated("FaceFaceConstraint is deprecated!  Use MortarConstraint instead!");
  }
};

#endif
