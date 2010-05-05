#ifndef IMPLICITBD2
#define IMPLICITBD2

#include "Kernel.h"

// Forward Declarations
class ImplicitBackwardDifference2;
template<>
InputParameters validParams<ImplicitBackwardDifference2>();

class ImplicitBackwardDifference2 : public Kernel
{
public:

  ImplicitBackwardDifference2(std::string name, MooseSystem & moose_system, InputParameters parameters);
  
protected:
  virtual Real computeQpResidual();

  virtual Real computeQpJacobian();

  bool _start_with_be;
};
#endif //IMPLICITBD2
