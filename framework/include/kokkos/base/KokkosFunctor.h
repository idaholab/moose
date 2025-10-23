//* This file is part of the MOOSE framework
//* https://mooseframework.inl.gov
//*
//* All rights reserved, see COPYRIGHT for full restrictions
//* https://github.com/idaholab/moose/blob/master/COPYRIGHT
//*
//* Licensed under LGPL 2.1, please see LICENSE for details
//* https://www.gnu.org/licenses/lgpl-2.1.html

#pragma once

#include "KokkosTypes.h"
#include "KokkosFunctorWrapper.h"
#include "KokkosFunctorRegistry.h"

class FEProblemBase;

namespace Moose
{
namespace Kokkos
{

/**
 * The abstract class that provides polymorphic interfaces for a functor
 */
class Functor final
{
public:
  /**
   * Constructor
   * @param problem The MOOSE problem
   * @param wrapper The host functor wrapper
   */
  Functor(FEProblemBase & problem, std::shared_ptr<FunctorWrapperHostBase> wrapper);
  /**
   * Copy constructor for parallel dispatch
   */
  Functor(const Functor & functor);
  /**
   * Destructor
   */
  ~Functor();

private:
  /**
   * Pointer to the host functor wrapper
   */
  std::shared_ptr<FunctorWrapperHostBase> _wrapper_host;
  /**
   * Pointer to the device functor wrapper
   */
  FunctorWrapperDeviceBase * _wrapper_device = nullptr;
  /**
   * Reference of the FE problem
   */
  FEProblemBase & _problem;
  /**
   * Current and old time
   */
  ///@{
  Scalar<Real> _t;
  Scalar<Real> _t_old;
  ///@}
};

} // namespace Kokkos
} // namespace Moose
