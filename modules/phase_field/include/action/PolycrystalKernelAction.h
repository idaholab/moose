#ifndef POLYCRYSTALKERNELACTION_H
#define POLYCRYSTALKERNELACTION_H

#include "Action.h"

class PolycrystalKernelAction: public Action
{
public:
  PolycrystalKernelAction(const std::string & name, InputParameters params);

  virtual void act();

private:
  unsigned int _crys_num;
  std::string _var_name_base;
  VariableName _c;
  bool _implicit;
  VariableName _T;
};

template<>
InputParameters validParams<PolycrystalKernelAction>();

#endif //POLYCRYSTALKERNELACTION_H
