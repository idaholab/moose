#ifndef ADDVARIABLEACTION_H
#define ADDVARIABLEACTION_H

#include "InputParameters.h"
#include "Moose.h"
#include "Action.h"

#include <string>

class AddVariableAction : public Action
{
public:
  AddVariableAction(const std::string & name, InputParameters params);

  virtual void act();

  bool restartRequired() const;
  bool autoResizeable() const;
  std::pair<std::string, unsigned int> initialValuePair() const;
  
private:
  static const Real _abs_zero_tol;
  std::string _variable_to_read;
  unsigned int _timestep_to_read;
};

template<>
InputParameters validParams<AddVariableAction>();

#endif // ADDVARIABLEACTION_H
