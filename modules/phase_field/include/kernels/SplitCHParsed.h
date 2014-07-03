#ifndef SPLITCHPARSED_H
#define SPLITCHPARSED_H

#include "SplitCHCRes.h"
#include "ParsedFreeEnergyInterface.h"

//Forward Declarations
class SplitCHParsed;

template<>
InputParameters validParams<SplitCHParsed>();

/**
 * SplitCHParsed allows a free energy functionals to be provided as a parsed
 * expression in the input file. It uses automatic differentiation
 * to generate the necessary derivative. This is the split operator variant.
 * \see SplitCHParsed
 */
class SplitCHParsed : public ParsedFreeEnergyInterface<SplitCHCRes>
{
public:
  SplitCHParsed(const std::string & name, InputParameters parameters);

protected:
  virtual Real computeDFDC(PFFunctionType type);
};

#endif // SPLITCHPARSED_H
