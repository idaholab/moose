// //* This file is part of the MOOSE framework
// //* https://mooseframework.inl.gov
// //*
// //* All rights reserved, see COPYRIGHT for full restrictions
// //* https://github.com/idaholab/moose/blob/master/COPYRIGHT
// //*
// //* Licensed under LGPL 2.1, please see LICENSE for details
// //* https://www.gnu.org/licenses/lgpl-2.1.html

// #ifdef MOOSE_MFEM_ENABLED

// #pragma once

// #include <vector>

// #include "mfem.hpp"

// /**
//  * Auxiliary class to extract locations of nodes in MFEM GridFunctions
//  * and project values defined at them to set DoFs
//  */
// class MFEMTransferInterpolator
// {
// public:
//   MFEMTransferInterpolator() = default;
//   /// Extract points from MFEM FESpace at which projection will take place
//   void extractProjectionPoints(mfem::ParFiniteElementSpace & to_fespace, mfem::Vector & vxyz, mfem::Ordering::Type & point_ordering);
//   /// Project a vector of values provided at projection points (nodes) to set GridFunction DoFs
//   mfem::FindPointsGSLIB & getInterpolator() {return &_mfem_interpolator};
// protected:
//   mfem::FindPointsGSLIB _mfem_interpolator;  
// };

// #endif
