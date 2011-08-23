#ifndef PRESSUREACTION_H
#define PRESSUREACTION_H

#include "Action.h"

class PressureAction: public Action
{
public:
  PressureAction(const std::string & name, InputParameters params);

  virtual void act();

private:
  const std::vector<unsigned int> _boundary;
  const std::string _disp_x;
  const std::string _disp_y;
  const std::string _disp_z;
  const Real _factor;
  const std::string _function;

protected:
  std::string _kernel_name;
  bool _use_displaced_mesh;
};

template<>
InputParameters validParams<PressureAction>();

#endif // PRESSUREACTION_H
