#ifndef DEFAULTMATPROPCONSUMERKERNEL_H
#define DEFAULTMATPROPCONSUMERKERNEL_H

#include "Kernel.h"
#include "DerivativeMaterialInterface.h"

//Forward declarations
class DefaultMatPropConsumerKernel;

template<>
InputParameters validParams<DefaultMatPropConsumerKernel>();

class DefaultMatPropConsumerKernel : public DerivativeMaterialInterface<Kernel>
{
public:
	DefaultMatPropConsumerKernel(const std::string & name, InputParameters parameters);

protected:
	virtual Real computeQpResidual() { return 0.0; };

	std::string _prop_name;
	const MaterialProperty<Real> & _prop;
};

#endif //DEFAULTMATPROPCONSUMERKERNEL_H
