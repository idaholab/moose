#ifndef TIMEDERIVATIVE_H_
#define TIMEDERIVATIVE_H_

#include "TimeKernel.h"

// Forward Declaration
class TimeDerivative;

template<>
InputParameters validParams<TimeDerivative>();

class TimeDerivative : public TimeKernel
{
public:
  TimeDerivative(const std::string & name, InputParameters parameters);
  
protected:
  virtual Real computeQpResidual();
  virtual Real computeQpJacobian();

  /**
   * Coefficients (weights) for the BDF2 time discretization.
   */
//  std::vector<Real> & _time_weight;
};

#endif //TIMEDERIVATIVE_H_
