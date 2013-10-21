#ifndef REPORTABLEDIFFUSION_H
#define REPORTABLEDIFFUSION_H

#include "Kernel.h"
#include "MooseTypes.h"

//Forward Declarations
class ReportableDiffusion;

template<>
InputParameters validParams<ReportableDiffusion>();

/**
 * A kernel for basic testing of Reportable interface functionality
 */
class ReportableDiffusion : public Kernel
{
public:

  ReportableDiffusion(const std::string & name, InputParameters parameters);

protected:

  virtual Real computeQpResidual();

  virtual Real computeQpJacobian();

  const Real &  _coef;

  const MooseEnum & _test;

  ReportableValue & _calls;
};

#endif //REPORTABLEDIFFUSION_H
