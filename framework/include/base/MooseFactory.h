/****************************************************************/
/*               DO NOT MODIFY THIS HEADER                      */
/* MOOSE - Multiphysics Object Oriented Simulation Environment  */
/*                                                              */
/*           (c) 2010 Battelle Energy Alliance, LLC             */
/*                   ALL RIGHTS RESERVED                        */
/*                                                              */
/*          Prepared by Battelle Energy Alliance, LLC           */
/*            Under Contract No. DE-AC07-05ID14517              */
/*            With the U. S. Department of Energy               */
/*                                                              */
/*            See COPYRIGHT for full restrictions               */
/****************************************************************/

#ifndef MOOSEFACTORY_H
#define MOOSEFACTORY_H

#include "KernelFactory.h"
#include "BCFactory.h"
#include "AuxFactory.h"
#include "MaterialFactory.h"
#include "ParserBlockFactory.h"
#include "FunctionFactory.h"
#include "InitialConditionFactory.h"
#include "ExecutionerFactory.h"
#include "StabilizerFactory.h"
#include "PostprocessorFactory.h"
#include "DamperFactory.h"
#include "DGKernelFactory.h"

class MooseFactory
{
public:
  template <typename T>
  static void registerMooseKernel(const std::string & name);

  template <typename T>
  static void registerMooseBoundaryCondition(const std::string & name);

  template <typename T>
  static void registerMooseAux(const std::string & name);

  template <typename T>
  static void registerMooseMaterial(const std::string & name);

  template <typename T>
  static void registerMooseParserBlock(const std::string & name);

  template <typename T>
  static void registerMooseFunction(const std::string & name);

  template <typename T>
  static void registerMooseInitialCondition(const std::string & name);

  template <typename T>
  static void registerMooseExecutioner(const std::string & name);

  template <typename T>
  static void registerMooseStabilizer(const std::string & name);

  template <typename T>
  static void registerMoosePostprocessor(const std::string & name);

  template <typename T>
  static void registerMooseDamper(const std::string & name);

  template <typename T>
  static void registerMooseDGKernel(const std::string & name);
};


template <typename T>
void
MooseFactory::registerMooseKernel(const std::string & name)
{
  KernelFactory::instance()->registerKernel<T>(name);
}

template <typename T>
void
MooseFactory::registerMooseBoundaryCondition(const std::string & name)
{
  BCFactory::instance()->registerBoundaryCondition<T>(name);
}

template <typename T>
void
MooseFactory::registerMooseAux(const std::string & name)
{
  AuxFactory::instance()->registerAux<T>(name);
}

template <typename T>
void
MooseFactory::registerMooseMaterial(const std::string & name)
{
  MaterialFactory::instance()->registerMaterial<T>(name);
}

template <typename T>
void
MooseFactory::registerMooseParserBlock(const std::string & name)
{
  ParserBlockFactory::instance()->registerParserBlock<T>(name);
}

template <typename T>
void
MooseFactory::registerMooseFunction(const std::string & name)
{
  FunctionFactory::instance()->registerFunction<T>(name);
}

template <typename T>
void
MooseFactory::registerMooseInitialCondition(const std::string & name)
{
  InitialConditionFactory::instance()->registerInitialCondition<T>(name);
}

template <typename T>
void
MooseFactory::registerMooseExecutioner(const std::string & name)
{
  ExecutionerFactory::instance()->registerExecutioner<T>(name);
}

template <typename T>
void
MooseFactory::registerMooseStabilizer(const std::string & name)
{
  StabilizerFactory::instance()->registerStabilizer<T>(name);
}

template <typename T>
void
MooseFactory::registerMoosePostprocessor(const std::string & name)
{
  PostprocessorFactory::instance()->registerPostprocessor<T>(name);
}

template <typename T>
void
MooseFactory::registerMooseDamper(const std::string & name)
{
  DamperFactory::instance()->registerDamper<T>(name);
}

template <typename T>
void
MooseFactory::registerMooseDGKernel(const std::string & name)
{
  DGKernelFactory::instance()->registerDGKernel<T>(name);
}

/**
 * MOOSE Factory Macros
 */
#define stringifyName(name) #name

#define registerKernel(name)                      MooseFactory::registerMooseKernel<name>(stringifyName(name));
#define registerBoundaryCondition(name)           MooseFactory::registerMooseBoundaryCondition<name>(stringifyName(name));
#define registerAux(name)                         MooseFactory::registerMooseAux<name>(stringifyName(name));
#define registerMaterial(name)                    MooseFactory::registerMooseMaterial<name>(stringifyName(name));
#define registerParserBlock(name)                 MooseFactory::registerMooseParserBlock<name>(stringifyName(name));
#define registerFunction(name)                    MooseFactory::registerMooseFunction<name>(stringifyName(name));
#define registerInitialCondition(name)            MooseFactory::registerMooseInitialCondition<name>(stringifyName(name));
#define registerExecutioner(name)                 MooseFactory::registerMooseExecutioner<name>(stringifyName(name));
#define registerStabilizer(name)                  MooseFactory::registerMooseStabilizer<name>(stringifyName(name));
#define registerPostprocessor(name)               MooseFactory::registerMoosePostprocessor<name>(stringifyName(name));
#define registerDamper(name)                      MooseFactory::registerMooseDamper<name>(stringifyName(name));
#define registerDGKernel(name)                    MooseFactory::registerMooseDGKernel<name>(stringifyName(name));

#define registerNamedKernel(tplt, name)           MooseFactory::registerMooseKernel<tplt>(name);
#define registerNamedBoundaryCondition(tplt, name) MooseFactory::registerMooseBoundaryCondition<tplt>(name);
#define registerNamedAux(tplt, name)              MooseFactory::registerMooseAux<tplt>(name);
#define registerNamedMaterial(tplt, name)         MooseFactory::registerMooseMaterial<tplt>(name);
#define registerNamedParserBlock(tplt, name)      MooseFactory::registerMooseParserBlock<tplt>(name);
#define registerNamedFunction(tplt, name)         MooseFactory::registerMooseFunction<tplt>(name);
#define registerNamedInitialCondition(tplt, name) MooseFactory::registerMooseInitialCondition<tplt>(name);
#define registerNamedExecutioner(tplt, name)      MooseFactory::registerMooseExecutioner<tplt>(name);
#define registerNamedStabilizer(tplt, name)       MooseFactory::registerMooseStabilizer<tplt>(name);
#define registerNamedPostprocessor(tplt, name)    MooseFactory::registerMoosePostprocessor<tplt>(name);
#define registerNamedDamper(tplt, name)           MooseFactory::registerMooseDamper<tplt>(name);
#define registerNamedDGKernel(tplt, name)         MooseFactory::registerMooseDGKernel<tplt>(name);


#endif //MOOSEFACTORY_H
