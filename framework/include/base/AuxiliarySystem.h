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

  virtual void addVariable(const std::string & var_name, const FEType & type, Real scale_factor, const std::set< subdomain_id_type > * const active_subdomains = NULL);
  void addKernel(const std::string & kernel_name, const std::string & name, InputParameters parameters);
  void addBoundaryCondition(const std::string & bc_name, const std::string & name, InputParameters parameters);

  virtual void reinitElem(const Elem * elem, THREAD_ID tid);

  virtual void compute();
  virtual void compute_ts();

protected:
  virtual void computeInternal(std::vector<AuxWarehouse> & auxs);

  MProblem & _mproblem;
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
  friend class ComputeNodalAuxThread;
  friend class ComputeElemAuxThread;
};

#endif /* EXPLICITSYSTEM_H */
