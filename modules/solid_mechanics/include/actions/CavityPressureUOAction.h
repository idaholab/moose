#ifndef CAVITYPRESSUREUOACTION_H
#define CAVITYPRESSUREUOACTION_H

#include "Action.h"
#include "MooseTypes.h"

class CavityPressureUOAction: public Action
{
public:
  CavityPressureUOAction(const std::string & name, InputParameters params);

  virtual void act();

  const Real _initial_pressure;
  const std::vector<PostprocessorName> _material_input;
  const Real _R;
  const PostprocessorName _temperature;
  const PostprocessorName _volume;
  const Real _startup_time;
};

template<>
InputParameters validParams<CavityPressureUOAction>();


#endif
