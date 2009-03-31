#ifndef UPDATEAUXVARS_H
#define UPDATEAUXVARS_H

//libMesh includes
#include "numeric_vector.h"

namespace Moose
{
  /**
   * Updates the values of all Auxiliary variables.
   */
  void update_aux_vars(const NumericVector<Number>& soln);
}

#endif //UPDATEAUXVARS_H
