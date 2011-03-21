#ifndef FORCINGFN_H_
#define FORCINGFN_H_

#include "Kernel.h"

class ForcingFn : public Kernel
{
public:
  ForcingFn(const std::string & name, InputParameters parameters);

protected:
  virtual Real computeQpResidual();

  virtual Real value();
};

template<>
InputParameters validParams<ForcingFn>();

#endif /* FORCINGFN_H_ */
