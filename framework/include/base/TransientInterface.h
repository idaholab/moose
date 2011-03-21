#ifndef TRANSIENTINTERFACE_H
#define TRANSIENTINTERFACE_H

#include "InputParameters.h"


class SubProblem;

class TransientInterface
{
public:
  TransientInterface(InputParameters & parameters);
  virtual ~TransientInterface();

private:
  SubProblem & _ti_problem;

protected:
  Real & _t;
  int & _t_step;
  Real & _dt;
};

#endif /* TRANSIENTINTERFACE_H_ */
