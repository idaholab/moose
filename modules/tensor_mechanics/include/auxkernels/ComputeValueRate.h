/*
AuxKernel of Passing Variable Time Derivative
*/

#pragma once

#include "AuxKernel.h"

class ComputeValueRate : public AuxKernel
{
    public:

    static InputParameters validParams();
    ComputeValueRate(const InputParameters & parameters);

    protected:

    virtual Real computeValue() override;

    const VariableValue & _coupled_val;

};



