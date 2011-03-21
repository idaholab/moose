#ifndef TRANSIENTINTERFACE_H
#define TRANSIENTINTERFACE_H

#include "InputParameters.h"


class Problem;

class TransientInterface
{
public:
  TransientInterface(InputParameters & parameters);
  virtual ~TransientInterface();

private:
  Problem & _ti_problem;

protected:
  Real & _t;
  int & _t_step;
  Real & _dt;
  std::vector<Real> & _time_weight;

  // NOTE: dunno if it is set properly in time of instantiation (might be a source of bugs)
  bool _is_transient;
};

#endif /* TRANSIENTINTERFACE_H_ */
