#ifndef DoubleWellPotential_H
#define DoubleWellPotential_H

// Algebraic double well potential.

#include "ACBulk.h"

//Forward Declarations
class DoubleWellPotential;

template<>
InputParameters validParams<DoubleWellPotential>();

class DoubleWellPotential : public ACBulk
{
public:

  DoubleWellPotential(const std::string & name, InputParameters parameters);

protected:

  virtual Real computeDFDOP(PFFunctionType type);

private:


};
#endif //DoubleWellPotential_H
