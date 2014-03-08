#include "AdaptiveDT.h"

template<>
InputParameters validParams<AdaptiveDT>()
{
  InputParameters params = validParams<IterationAdaptiveDT>();
  return params;
}

AdaptiveDT::AdaptiveDT(const std::string & name, InputParameters parameters) :
  IterationAdaptiveDT(name, parameters)
{
  mooseWarning("AdaptiveDT is deprecated and will be removed in the near future.  Please replace with IterationAdaptiveDT");
}

AdaptiveDT::~AdaptiveDT()
{
}
