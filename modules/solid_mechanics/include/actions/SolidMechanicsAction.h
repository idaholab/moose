#ifndef SOLIDMECHANICSACTION_H
#define SOLIDMECHANICSACTION_H

#include "Action.h"
#include "MooseTypes.h"

class SolidMechanicsAction;

template<>
InputParameters validParams<SolidMechanicsAction>();

class SolidMechanicsAction : public Action
{
public:
  SolidMechanicsAction(const std::string & name, InputParameters params);

  virtual void act();

private:
  const NonlinearVariableName _disp_x;
  const NonlinearVariableName _disp_y;
  const NonlinearVariableName _disp_z;
  const NonlinearVariableName _disp_r;
  const NonlinearVariableName _temp;
};


#endif
