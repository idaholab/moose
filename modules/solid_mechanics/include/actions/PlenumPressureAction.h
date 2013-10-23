#ifndef PLENUMPRESSUREACTION_H
#define PLENUMPRESSUREACTION_H

#include "Action.h"
#include "MooseTypes.h"

class PlenumPressureAction;

template<>
InputParameters validParams<PlenumPressureAction>();

class PlenumPressureAction: public Action
{
public:
  PlenumPressureAction(const std::string & name, InputParameters params);

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


#endif // PLENUMPRESSUREACTION_H
