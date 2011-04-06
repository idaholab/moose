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

#ifndef AUXILIARYSYSTEM_H
#define AUXILIARYSYSTEM_H

#include <set>
#include "SystemBase.h"
#include "AuxWarehouse.h"

// libMesh include
#include "equation_systems.h"
#include "explicit_system.h"
#include "transient_system.h"

#include "numeric_vector.h"

class AuxKernel;
class MProblem;

class AuxiliarySystem : public SystemTempl<TransientExplicitSystem>
{
public:
  AuxiliarySystem(MProblem & subproblem, const std::string & name);
  virtual ~AuxiliarySystem();

  virtual void init();

  virtual void initialSetup();
  virtual void timestepSetup();
  virtual void residualSetup();
  virtual void jacobianSetup();

  virtual void addVariable(const std::string & var_name, const FEType & type, Real scale_factor, const std::set< subdomain_id_type > * const active_subdomains = NULL);
  void addKernel(const std::string & kernel_name, const std::string & name, InputParameters parameters);
  void addBoundaryCondition(const std::string & bc_name, const std::string & name, InputParameters parameters);

  virtual void reinitElem(const Elem * elem, THREAD_ID tid);

  virtual const NumericVector<Number> * & currentSolution() { _current_solution = _sys.current_local_solution.get(); return _current_solution; }

  virtual void serializeSolution();
  virtual NumericVector<Number> & serializedSolution();

  void augmentSendList(std::vector<unsigned int> & send_list);

  virtual void compute();
  virtual void compute_ts();

protected:
  virtual void computeInternal(std::vector<AuxWarehouse> & auxs);

  void computeNodalVars(std::vector<AuxWarehouse> & auxs);
  void computeElementalVars(std::vector<AuxWarehouse> & auxs);

  MProblem & _mproblem;

  const NumericVector<Number> * _current_solution;      /// solution vector from nonlinear solver
  NumericVector<Number> & _serialized_solution;         /// Serialized version of the solution vector

  bool _need_serialized_solution;                       /// Whether or not a copy of the residual needs to be made

  // Variables
  std::vector<std::map<std::string, MooseVariable *> > _nodal_vars;
  std::vector<std::map<std::string, MooseVariable *> > _elem_vars;
  std::vector<AuxWarehouse> _auxs;
  std::vector<AuxWarehouse> _auxs_ts;                           // aux_kernels executed at the end of time step

  // data
  struct AuxData
  {
    Real _current_volume;

    friend class AuxKernel;
  };

  std::vector<AuxData> _data;

  friend class AuxKernel;
  friend class ComputeNodalAuxVarsThread;
  friend class ComputeElemAuxVarsThread;
};

#endif /* EXPLICITSYSTEM_H */
