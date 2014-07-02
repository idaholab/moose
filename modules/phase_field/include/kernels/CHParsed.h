#ifndef CHPARSED_H
#define CHPARSED_H

#include "CHBulk.h"
#include "ParsedFreeEnergyInterface.h"

//Forward Declarations
class CHParsed;

template<>
InputParameters validParams<CHParsed>();

/**
 * CHParsed allows a free energy functionals to be provided as a parsed
 * expression in the input file. It uses automatic differentiation
 * to generate the necessary derivative.
 * \see SplitCHParsed
 */
class CHParsed : public ParsedFreeEnergyInterface<CHBulk>
{
public:
  CHParsed(const std::string & name, InputParameters parameters);

protected:
  virtual RealGradient computeGradDFDCons(PFFunctionType type);
};

#endif // CHPARSED_H
