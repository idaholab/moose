#ifndef TRANSIENTHALF_H
#define TRANSIENTHALF_H

#include "TransientExecutioner.h"

// Forward Declarations
class TransientHalf;

template<>
InputParameters validParams<TransientHalf>();

class TransientHalf: public TransientExecutioner
{
public:

  TransientHalf(const std::string & name, MooseSystem & moose_system, InputParameters parameters);

protected:
  virtual Real computeDT();

private:
  Real _ratio;
  Real _min_dt;
};

#endif //TRANSIENTHALF_H
