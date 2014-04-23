#ifndef DOUBLEWELLPOTENTIAL_H
#define DOUBLEWELLPOTENTIAL_H

#include "ACBulk.h"

//Forward Declarations
class DoubleWellPotential;

template<>
InputParameters validParams<DoubleWellPotential>();

/**
 * Algebraic double well potential.
 */
class DoubleWellPotential : public ACBulk
{
public:
  DoubleWellPotential(const std::string & name, InputParameters parameters);

protected:
  virtual Real computeDFDOP(PFFunctionType type);
};

#endif //DOUBLEWELLPOTENTIAL_H
