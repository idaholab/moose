#ifndef PLENUMPRESSUREPPACTION_H
#define PLENUMPRESSUREPPACTION_H

#include "Action.h"
#include "MooseTypes.h"

class PlenumPressurePPAction: public Action
{
public:
  PlenumPressurePPAction(const std::string & name, InputParameters params);

  virtual void act();

private:
  const std::vector<BoundaryName> _boundary;
  const NonlinearVariableName _disp_x;
  const NonlinearVariableName _disp_y;
  const NonlinearVariableName _disp_z;
  const Real _initial_pressure;
  const std::vector<PostprocessorName> _material_input;
  const Real _R;
  const PostprocessorName _temperature;
  const PostprocessorName _volume;
  const Real _startup_time;
  const std::string _output_initial_moles;

protected:
  std::string _pp_name;
};

template<>
InputParameters validParams<PlenumPressurePPAction>();


#endif // PLENUMPRESSUREPPACTION_H
