#ifndef RICHARDSMASSDENSITY_H
#define RICHARDSMASSDENSITY_H

#include "AuxKernel.h"

#include "RichardsDensity.h"
#include "RichardsSeff.h"
#include "RichardsSat.h"

//Forward Declarations
class RichardsMassDensity;

template<>
InputParameters validParams<RichardsMassDensity>();

class RichardsMassDensity: public AuxKernel
{
public:
  RichardsMassDensity(const std::string & name, InputParameters parameters);

protected:
  virtual Real computeValue();

  int _p_num;
  const RichardsDensity & _density_UO;
  const RichardsSeff & _seff_UO;
  const RichardsSat & _sat_UO;

  std::vector<unsigned int> _pressure_vars;
  std::vector<VariableValue *> _pressure_vals;
};

#endif // RICHARDSMASSDENSITY_H
