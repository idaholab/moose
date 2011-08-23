#ifndef SOLIDMECHANICSACTION_H
#define SOLIDMECHANICSACTION_H

#include "Action.h"

class SolidMechanicsAction;

template<>
InputParameters validParams<SolidMechanicsAction>();

class SolidMechanicsAction : public Action
{
public:
  SolidMechanicsAction(const std::string & name, InputParameters params);

  virtual void act();

private:
  const std::string _disp_x;
  const std::string _disp_y;
  const std::string _disp_z;
  const std::string _disp_r;
  const std::string _temp;
};


#endif
