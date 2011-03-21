#ifndef TRANSIENTINTERFACE_H_
#define TRANSIENTINTERFACE_H_

#include "InputParameters.h"


namespace Moose
{

class SubProblem;

class TransientInterface
{
public:
  TransientInterface(InputParameters & parameters);
  virtual ~TransientInterface();

private:
  Moose::SubProblem & _ti_problem;

protected:
  Real & _t;
  int & _t_step;
  Real & _dt;
};

}

#endif /* TRANSIENTINTERFACE_H_ */
