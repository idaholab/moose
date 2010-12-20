#ifndef PLENUMPRESSUREBC_H
#define PLENUMPRESSUREBC_H

#include "BoundaryCondition.h"

//Forward Declarations
class PlenumPressureBC;

template<>
InputParameters validParams<PlenumPressureBC>();

class PlenumPressureBC : public BoundaryCondition
{
public:

  PlenumPressureBC(const std::string & name, InputParameters parameters);

  virtual ~PlenumPressureBC(){}

  virtual void setup();

protected:

  virtual Real computeQpResidual();

  bool _initialized;

  Real _n0; // The initial number of moles of gas.

  const int _component;

  const Real _initial_pressure;

  const Real & _material_input;

  const Real _R;

  const Real & _temperature;

  const Real & _volume;

  const Real _startup_time;

};

#endif //PLENUMRESSUREBC_H
