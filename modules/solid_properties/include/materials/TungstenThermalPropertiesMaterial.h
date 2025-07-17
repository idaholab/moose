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
 * This material computes Tungsten thermal properties as a function of temperature.
 * Constants are taken from Milner, J. L., Karkos, P., & Bowers, J. J. (2024).
 * Space Nuclear Propulsion (SNP) Material Property Handbook (No. SNP-HDBK-0008).
 * National Aeronautics and Space Administration (NASA). https://ntrs.nasa.gov/citations/20240004217
 */

class TungstenThermalPropertiesMaterial : public Material
{
    public:
            static InputParameters validParams();
            TungstenThermalPropertiesMaterial(const InputParameters & parameters);

    protected:
            virtual void computeQpProperties();

            // Constants for the thermal conductivity formulas
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

            // Constants for the specific heat formulas
            static constexpr Real _cN   = 3.030;
            static constexpr Real _cA0  = 3.103e+02;
            static constexpr Real _cA1  = -8.815;
            static constexpr Real _cA2  = 1.295e+02;
            static constexpr Real _cA3  = 1.874e+03;
            static constexpr Real _cB0  = 1.301e-01;
            static constexpr Real _cB1  = 2.225e-02;
            static constexpr Real _cB2  = -7.224e-03;
            static constexpr Real _cB3  = 3.539e-03;
            static constexpr Real _cB_2  = -3.061e-04;
            // Constatns for the density formulas
            static constexpr Real _rA0 = 19250;
            static constexpr Real _rA1 = -8.529e-02;
            static constexpr Real _rA2 = -9.915e-02;
            static constexpr Real _rA3 = 2.257;
            static constexpr Real _rA4 = -3.157;
            static constexpr Real _rB0 = -1.4e-01;
            static constexpr Real _rB1 = 4.869e-01;
            static constexpr Real _rB2 = -3.056e-02;
            static constexpr Real _rB3 = 2.234e-02;
    private:
            const VariableValue & _temperature;
            /// Thermal conductivity of the tungsten material
            MaterialProperty<Real> & _k;
            /// specific heat of the tungsten material
            MaterialProperty<Real> & _c_p;
            /// density of the tungsten material
            MaterialProperty<Real> & _rho;

};
