#ifndef TIMEKERNEL_H_
#define TIMEKERNEL_H_

#include "Kernel.h"

// Forward Declaration
class TimeKernel;

template<>
InputParameters validParams<TimeKernel>();

/**
 * All time kernels should inherit from this class
 *
 */
class TimeKernel : public Kernel
{
public:
  TimeKernel(const std::string & name, InputParameters parameters);
};

#endif //TIMEKERNEL
