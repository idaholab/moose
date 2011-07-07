#ifndef PLENUMPRESSUREACTION_H
#define PLENUMPRESSUREACTION_H

#include "Action.h"

class PlenumPressureAction;

template<>
InputParameters validParams<PlenumPressureAction>();

class PlenumPressureAction: public Action
{
public:
  PlenumPressureAction(const std::string & name, InputParameters params);

  virtual void act();

private:
  const std::vector<unsigned int> _boundary;
  const std::string _disp_x;
  const std::string _disp_y;
  const std::string _disp_z;
  const Real _initial_pressure;
  const std::string _material_input;
  const Real _R;
  const std::string _temperature;
  const std::string _volume;
  const Real _startup_time;
  const std::string _output_initial_moles;
  const std::string _output;

protected:
  std::string _kernel_name;
  bool _use_displaced_mesh;
};


#endif // PLENUMPRESSUREACTION_H
