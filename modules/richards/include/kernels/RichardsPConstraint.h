#ifndef RICHARDSPCONSTRAINT
#define RICHARDSPCONSTRAINT

#include "Kernel.h"

// Forward Declarations
class RichardsPConstraint;

template<>
InputParameters validParams<RichardsPConstraint>();

class RichardsPConstraint : public Kernel
{
public:

  RichardsPConstraint(const std::string & name,
                        InputParameters parameters);


protected:
  virtual Real computeQpResidual();
  virtual Real computeQpJacobian();
  virtual Real computeQpOffDiagJacobian(unsigned int jvar);

private:
  Real _a;
  VariableValue & _lower;
  unsigned int _lower_var_num;

};

#endif //RICHARDSPCONSTRAINT
