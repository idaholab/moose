#ifndef DIFFTENSORKERNEL_H
#define DIFFTENSORKERNEL_H

#include "Kernel.h"
#include "MooseParsedVectorFunction.h"
#include "MaterialProperty.h"

// Forward Declaration
class DiffTensorKernel;

template<>
InputParameters validParams<DiffTensorKernel>();

/**
 * A Kernel for Testing ParsedVectorFunction
 */
class DiffTensorKernel : public Kernel
{
public:
  /** Class constructor */
  DiffTensorKernel(const std::string & name, InputParameters parameters);

  /** Class destructor */
  virtual ~DiffTensorKernel();

protected:
  virtual Real computeQpResidual();

  virtual Real computeQpJacobian();

  /// A vector function containing the components of k for the tensor
  Function & _k_comp;

private:

  /** Compute the k Tensor from the vector function input */
  RealTensorValue computeConductivity(Real t, const Point & pt);

};
#endif //DIFFTENSORKERNEL_H
