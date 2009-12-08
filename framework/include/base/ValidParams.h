#ifndef VALIDPARAMS_H
#define VALIDPARAMS_H

#include "InputParameters.h"

/**
 * This function template should be specialized for all kernels, kernel derived objects,
 * and parser block objects so that parameters can be set, viewed and used for documentation
 * purposes without creating object instances
 */
template<class KernelType>
InputParameters validParams()
{
  InputParameters params;
  return params;
}

#endif //VALIDPARAMS_H
