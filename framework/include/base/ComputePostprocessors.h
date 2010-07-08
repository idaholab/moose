#ifndef COMPUTERESIDUAL_H
#define COMPUTERESIDUAL_H

#include "numeric_vector.h"
#include "nonlinear_implicit_system.h"

namespace Moose
{
  void compute_postprocessors (const NumericVector<Number>& soln, NonlinearImplicitSystem& sys);
}

#endif //COMPUTERESIDUAL_H
