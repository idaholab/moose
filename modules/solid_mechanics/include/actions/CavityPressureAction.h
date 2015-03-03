/****************************************************************/
/* MOOSE - Multiphysics Object Oriented Simulation Environment  */
/*                                                              */
/*          All contents are licensed under LGPL V2.1           */
/*             See LICENSE for full restrictions                */
/****************************************************************/
#ifndef CAVITYPRESSUREACTION_H
#define CAVITYPRESSUREACTION_H

#include "Action.h"
#include "MooseTypes.h"

class CavityPressureAction;

template<>
InputParameters validParams<CavityPressureAction>();

class CavityPressureAction: public Action
{
public:
  CavityPressureAction(const std::string & name, InputParameters params);

  virtual void act();

private:
  const std::vector<BoundaryName> _boundary;
  const NonlinearVariableName _disp_x;
  const NonlinearVariableName _disp_y;
  const NonlinearVariableName _disp_z;
  std::vector<std::vector<AuxVariableName> > _save_in_vars;
  std::vector<bool> _has_save_in_vars;

protected:
  std::string _kernel_name;
  bool _use_displaced_mesh;
};


#endif // CAVITYPRESSUREACTION_H
