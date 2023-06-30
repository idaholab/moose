// //* This file is part of the MOOSE framework
// //* https://www.mooseframework.org
// //*
// //* All rights reserved, see COPYRIGHT for full restrictions
// //* https://github.com/idaholab/moose/blob/master/COPYRIGHT
// //*
// //* Licensed under LGPL 2.1, please see LICENSE for details
// //* https://www.gnu.org/licenses/lgpl-2.1.html

// #pragma once

// #include "Material.h"
// #include "CompileTimeDerivatives.h"

// using namespace CompileTimeDerivatives;

// /**
//  * Material class that computes a value with a standard deviation from a function fit and a
//  * covariance matrix.
//  */
// class LarrysFit : public Material
// {
// public:
//   static InputParameters validParams();

//   LarrysFit(const InputParameters & parameters);

// protected:
//   virtual void computeQpProperties();

//   /// Covariance matrix (for 10 fitting parameters)
//   const CompileTimeDerivatives::CTMatrix<Real, 10, 10> _covariance;

//   /// porosity variable
//   const CompileTimeDerivatives::
//       CTArrayRef<CompileTimeDerivatives::CTNoTag, ADVariableValue, unsigned int>
//           _porosity;

//   /// sodium filled fraction
//   const CompileTimeDerivatives::
//       CTArrayRef<CompileTimeDerivatives::CTNoTag, ADVariableValue, unsigned int>
//           _na_filled_fraction;

//   /// effective diffusivity constant of Nd in U-Zr
//   ADMaterialProperty<Real> & _diff;

//   /// effective diffusivity constant of Nd in U-Zr
//   ADMaterialProperty<Real> & _stddev_diff;
// };
