//* This file is part of the MOOSE framework
//* https://mooseframework.inl.gov
//*
//* All rights reserved, see COPYRIGHT for full restrictions
//* https://github.com/idaholab/moose/blob/master/COPYRIGHT
//*
//* Licensed under LGPL 2.1, please see LICENSE for details
//* https://www.gnu.org/licenses/lgpl-2.1.html

#pragma once

#include "Material.h"

/**
 * This material computes Tungsten thermal conductivity as a function of temperature
 */

class TungstenThermalConductivity : public Material
{
    public:
            static InputParameters validParams();
            TungstenThermalConductivity(const InputParameters & parameters);

    protected:
            virtual void computeQpProperties();
            static constexpr Real _kA0  = 7.348e+05;
            static constexpr Real _kA1  = 2.544e+01;
            static constexpr Real _kA2  = -8.304e+03;
            static constexpr Real _kA3  = 1.180e+06;
            static constexpr Real _kB0  = -3.679;
            static constexpr Real _kB1  = 1.181e+02;
            static constexpr Real _kB2  = 5.879e+01;
            static constexpr Real _kB3  = 2.867;
            static constexpr Real _kC0  = -2.052e-02;
            static constexpr Real _kC1  = 4.741e-01;
    private:
            const VariableValue & _temperature;
            /// Thermal conductivity of the material
            MaterialProperty<Real> & _k;
            
};
