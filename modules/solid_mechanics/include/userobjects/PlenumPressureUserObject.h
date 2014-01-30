#ifndef PLENUMPRESSUREUSEROBJECT_H
#define PLENUMPRESSUREUSEROBJECT_H

#include "GeneralUserObject.h"

class PlenumPressureUserObject : public GeneralUserObject
{
public:

  PlenumPressureUserObject(const std::string & name, InputParameters parameters);

  virtual ~PlenumPressureUserObject(){}

  virtual void initialSetup() {}

  virtual void residualSetup() {}

  virtual void timestepSetup() {}

  virtual void execute();

  virtual void initialize();
  virtual void finalize() {}

  Real getValue(const std::string & quantity) const;

protected:

  Real & _plenum_pressure;
  Real & _n0; // The initial number of moles of gas.

  const Real _initial_pressure;

  std::vector<const PostprocessorValue *> _material_input;

  const Real _R;

  const Real & _temperature;

  const Real & _volume;

  Real _start_time;
  const Real _startup_time;

  const unsigned _refab_needed;
  Real & _refab_gas_released;
  const std::vector<Real> _refab_time;
  const std::vector<Real> _refab_pressure;
  const std::vector<Real> _refab_temperature;
  const std::vector<Real> _refab_volume;
  const std::vector<unsigned> _refab_type;
  unsigned & _refab_counter;


  bool & _initialized;

};

template<>
InputParameters validParams<PlenumPressureUserObject>();

#endif //PLENUMRESSUREPOSTPROCESSOR_H
