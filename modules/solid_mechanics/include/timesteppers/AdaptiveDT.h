#ifndef ADAPTIVEDT_H
#define ADAPTIVEDT_H

#include "IterationAdaptiveDT.h"

#include "LinearInterpolation.h"

class Function;
class PiecewiseLinear;

/**
 *
 */
class AdaptiveDT : public IterationAdaptiveDT
{
public:
  AdaptiveDT(const std::string & name, InputParameters parameters);
  virtual ~AdaptiveDT();
};

template<>
InputParameters validParams<AdaptiveDT>();

#endif /* ADAPTIVEDT_H */
