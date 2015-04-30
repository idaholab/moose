#ifndef POLYCRYSTALVORONOIVOIDICACTION_H
#define POLYCRYSTALVORONOIVOIDICACTION_H

#include "InputParameters.h"
#include "Action.h"

/**
 * Automatically generates all variables to model a polycrystal with op_num orderparameters
 */
class PolycrystalVoronoiVoidICAction: public Action
{
public:
  PolycrystalVoronoiVoidICAction(const InputParameters & params);

  virtual void act();

private:
  static const Real _abs_zero_tol;

  unsigned int _op_num;
  unsigned int _grain_num;
  std::string _var_name_base;
  //unsigned int _rand_seed;
  //bool _cody_test;

};

template<>
InputParameters validParams<PolycrystalVoronoiVoidICAction>();

#endif //POLYCRYSTALVORONOIVOIDICACTION_H
