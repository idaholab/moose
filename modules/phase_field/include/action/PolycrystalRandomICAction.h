#ifndef POLYCRYSTALRANDOMICACTION_H
#define POLYCRYSTALRANDOMICACTION_H

#include "InputParameters.h"
#include "Action.h"

/**
 * Automatically generates all variables to model a polycrystal with crys_num orderparameters
 */
class PolycrystalRandomICAction: public Action
{
public:
  PolycrystalRandomICAction(const std::string & name, InputParameters params);

  virtual void act();

private:
  static const Real _abs_zero_tol;

  unsigned int _crys_num;
  //unsigned int _grain_num;
  std::string _var_name_base;
  MooseEnum _random_type;
};

template<>
InputParameters validParams<PolycrystalRandomICAction>();

#endif //POLYCRYSTALRANDOMICACTION_H
