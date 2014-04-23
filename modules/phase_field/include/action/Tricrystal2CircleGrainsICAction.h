#ifndef TRICRYSTAL2CIRCLEGRAINSICACTION_H
#define TRICRYSTAL2CIRCLEGRAINSICACTION_H

#include "InputParameters.h"
#include "Action.h"

/**
 * Automatically generates all variables to model a polycrystal with crys_num orderparameters
 */
class Tricrystal2CircleGrainsICAction: public Action
{
public:
  Tricrystal2CircleGrainsICAction(const std::string & name, InputParameters params);

  virtual void act();

private:
  static const Real _abs_zero_tol;

  std::string _var_name_base;
  unsigned int _crys_num;

};

template<>
InputParameters validParams<Tricrystal2CircleGrainsICAction>();

#endif //TRICRYSTAL2CIRCLEGRAINSICACTION_H
