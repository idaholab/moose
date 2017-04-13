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

#ifndef MESHEXTRUDER_H
#define MESHEXTRUDER_H

#include "MeshModifier.h"
#include "libmesh/mesh_generation.h"

class MeshExtruder;

template <>
InputParameters validParams<MeshExtruder>();

class MeshExtruder : public MeshModifier
{
public:
  MeshExtruder(const InputParameters & parameters);

protected:
  virtual void modify() override;

  const RealVectorValue _extrusion_vector;
  unsigned int _num_layers;
  std::vector<SubdomainID> _existing_subdomains;
  std::vector<unsigned int> _layers;
  std::vector<unsigned int> _new_ids;

  /**
   * This class is used during the mesh construction (extrusion) to set element ids as they are
   * created.
   */
  class QueryElemSubdomainID : public MeshTools::Generation::QueryElemSubdomainIDBase
  {
  public:
    QueryElemSubdomainID(std::vector<SubdomainID> existing_subdomains,
                         std::vector<unsigned int> layers,
                         std::vector<unsigned int> new_ids,
                         unsigned int num_layers);

    /// The override from the base class for obtaining a new id based on the old (original) element and the specified layer
    virtual subdomain_id_type get_subdomain_for_layer(const Elem * old_elem, unsigned int layer);

  private:
    /// Data structure for holding the old -> new id mapping based on the layer number
    std::map<unsigned int, std::map<SubdomainID, unsigned int>> _layer_data;

/// The total number of layers in the extrusion.  This is
/// currently only used for a sanity check in dbg mode.
#ifndef NDEBUG
    unsigned int _num_layers;
#endif
  };

private:
  void changeID(const std::vector<BoundaryName> & names, BoundaryID old_id);
};

#endif /* MESHEXTRUDER_H */
