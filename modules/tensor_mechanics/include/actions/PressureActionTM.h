/****************************************************************/
/* MOOSE - Multiphysics Object Oriented Simulation Environment  */
/*                                                              */
/*          All contents are licensed under LGPL V2.1           */
/*             See LICENSE for full restrictions                */
/****************************************************************/
#ifndef PRESSUREACTIONTM_H
#define PRESSUREACTIONTM_H

#include "Action.h"

class PressureActionTM: public Action
{
public:
  PressureActionTM(const std::string & name, InputParameters params);

  virtual void act();

private:
  const std::vector<BoundaryName> _boundary;
  const std::string _disp_x;
  const std::string _disp_y;
  const std::string _disp_z;
  std::vector<std::vector<AuxVariableName> > _save_in_vars;
  std::vector<bool> _has_save_in_vars;

  const Real _factor;
  const std::string _postprocessor;

protected:
  std::string _kernel_name;
  bool _use_displaced_mesh;
};

template<>
InputParameters validParams<PressureActionTM>();

#endif // PRESSUREACTIONTM_H
