#ifndef POLYNOMIALFREEENERGY_H
#define POLYNOMIALFREEENERGY_H

#include "DerivativeParsedMaterialHelper.h"
#include "ExpressionBuilder.h"

// Forward Declarations
class PolynomialFreeEnergy;

template<>
InputParameters validParams<PolynomialFreeEnergy>();

/**
 * Derivative free energy material defining polynomial free energies for single component materials, with derivatives from ExpressionBuilder
 */
class PolynomialFreeEnergy : public DerivativeParsedMaterialHelper,
                             public ExpressionBuilder
{
public:
  PolynomialFreeEnergy(const std::string & name, InputParameters parameters);

protected:
  ///Concentration variable used in the free energy expression
  EBTerm _c;

  ///Equilibrium concentration
  EBTerm _a;

  ///Barrier height
  EBTerm _W;

  ///Polynomial order
  MooseEnum _order;
};

#endif //POLYNOMIALFREEENERGY_H
