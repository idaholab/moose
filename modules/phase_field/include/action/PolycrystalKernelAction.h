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
  bool _with_bub;
  std::string _c_name;
  Real _en_ratio;
  std::string _bnds;
  bool _implicit;
  
  
};

template<>
InputParameters validParams<PolycrystalKernelAction>();
  
#endif //POLYCRYSTALKERNELACTION_H
