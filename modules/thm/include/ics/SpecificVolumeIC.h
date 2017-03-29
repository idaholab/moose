#ifndef SPECIFICVOLUMEIC_H
#define SPECIFICVOLUMEIC_H

#include "InitialCondition.h"

class SpecificVolumeIC;

template <>
InputParameters validParams<SpecificVolumeIC>();

/**
 *
 */
class SpecificVolumeIC : public InitialCondition
{
public:
  SpecificVolumeIC(const InputParameters & parameters);
  virtual ~SpecificVolumeIC();

  virtual Real value(const Point & p);

protected:
  const VariableValue & _rhoA;
  const VariableValue & _area;
  const VariableValue & _alpha;
};

#endif /* SPECIFICVOLUMEIC_H */
