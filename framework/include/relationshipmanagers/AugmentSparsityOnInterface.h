#ifndef AUGMENT_SPARSITY_ON_INTERFACE_H
#define AUGMENT_SPARSITY_ON_INTERFACE_H

// App includes
#include "AutomaticMortarGeneration.h"
#include "RelationshipManager.h"

// libMesh includes
#include "libmesh/mesh_base.h"

using libMesh::boundary_id_type;
using libMesh::Elem;
using libMesh::GhostingFunctor;
using libMesh::MeshBase;
using libMesh::processor_id_type;

class AugmentSparsityOnInterface;

template <>
InputParameters validParams<AugmentSparsityOnInterface>();

class AugmentSparsityOnInterface : public RelationshipManager
{
public:
  AugmentSparsityOnInterface(const InputParameters &);

  /**
   * This function must be overriden by application codes to add
   * required elements from (range_begin, range_end) to the
   * coupled_elements map.
   */
  virtual void operator()(const MeshBase::const_element_iterator & range_begin,
                          const MeshBase::const_element_iterator & range_end,
                          processor_id_type p,
                          map_type & coupled_elements) override;

  /**
   * According to the base class docs, "We call mesh_reinit() whenever
   * the relevant Mesh has changed, but before remote elements on a
   * distributed mesh are deleted."
   */
  virtual void mesh_reinit() override;

  /**
   * Update the cached _lower_to_upper map whenever our Mesh has been
   * redistributed.  We'll be lazy and just recalculate from scratch.
   */
  virtual void redistribute() override { this->mesh_reinit(); }

  std::string getInfo() const override;

  virtual bool operator==(const RelationshipManager & other) const override;

protected:
  virtual void internalInit() override;

  /**
   * The Mesh we're calculating on
   */
  AutomaticMortarGeneration * _amg;

  bool _has_attached_amg;

  BoundaryName _master_boundary_name;
  BoundaryName _slave_boundary_name;
  SubdomainName _master_subdomain_name;
  SubdomainName _slave_subdomain_name;

  std::pair<SubdomainID, SubdomainID> _subdomain_pair;
};

#endif
