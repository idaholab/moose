#ifndef COMPUTEPOSTPROCESSORS_H
#define COMPUTEPOSTPROCESSORS_H

#include "numeric_vector.h"
#include "nonlinear_implicit_system.h"

namespace Moose
{
  void compute_postprocessors (const NumericVector<Number>& soln, NonlinearImplicitSystem& sys);
}

#endif //COMPUTEPOSTPROCESSORS_H
