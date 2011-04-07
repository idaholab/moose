#ifndef CONTACTACTION_H
#define CONTACTACTION_H

#include "Action.h"

class ContactAction;

template<>
InputParameters validParams<ContactAction>();

class ContactAction: public Action
{
public:
  ContactAction(const std::string & name, InputParameters params);

  virtual void act();

private:
  const unsigned int _master;
  const unsigned int _slave;
  const std::string _disp_x;
  const std::string _disp_y;
  const std::string _disp_z;
  const Real _penalty;
};


#endif // CONTACTACTION_H
