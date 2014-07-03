#ifndef TENSORMECHANICSACTION_H
#define TENSORMECHANICSACTION_H

#include "Action.h"

class TensorMechanicsAction;

template<>
InputParameters validParams<TensorMechanicsAction>();

class TensorMechanicsAction : public Action
{
public:
  TensorMechanicsAction(const std::string & name, InputParameters params);

  virtual void act();

private:
  const std::string _disp_x;
  const std::string _disp_y;
  const std::string _disp_z;
  const std::string _disp_r;
  const std::string _temp;
};

#endif //TENSORMECHANICSACTION_H
