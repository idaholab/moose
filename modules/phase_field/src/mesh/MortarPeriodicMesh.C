/****************************************************************/
/* MOOSE - Multiphysics Object Oriented Simulation Environment  */
/*                                                              */
/*          All contents are licensed under LGPL V2.1           */
/*             See LICENSE for full restrictions                */
/****************************************************************/

#include "MortarPeriodicMesh.h"

// libMesh includes
#include "libmesh/mesh_modification.h"

template <>
InputParameters
validParams<MortarPeriodicMesh>()
{
  InputParameters params = validParams<GeneratedMesh>();
  params.addClassDescription("Set up an orthogonal mesh with additional dim-1 dimensional side "
                             "domains for use with the Mortar method.");
  MultiMooseEnum periodic_dirs("x=0 y=1 z=2");
  params.addRequiredParam<MultiMooseEnum>(
      "periodic_directions",
      periodic_dirs,
      "Directions along which additional Mortar meshes are generated");
  return params;
}

MortarPeriodicMesh::MortarPeriodicMesh(const InputParameters & parameters)
  : GeneratedMesh(parameters),
    _periodic_dirs(getParam<MultiMooseEnum>("periodic_directions")),
    _mortar_subdomains(_dim, Moose::INVALID_BLOCK_ID)
{
}

MortarPeriodicMesh::MortarPeriodicMesh(const MortarPeriodicMesh & other_mesh)
  : GeneratedMesh(other_mesh),
    _periodic_dirs(other_mesh._periodic_dirs),
    _mortar_subdomains(other_mesh._mortar_subdomains)
{
}

MortarPeriodicMesh::~MortarPeriodicMesh() {}

MooseMesh &
MortarPeriodicMesh::clone() const
{
  return *(new MortarPeriodicMesh(*this));
}

void
MortarPeriodicMesh::buildMesh()
{
  // build the main mesh
  GeneratedMesh::buildMesh();

  // boundaries
  const std::vector<BoundaryName> boundary_names = {"left", "bottom", "back"};

  buildBndElemList();

  // build side meshes
  for (unsigned short i = 0; i < _dim; ++i)
    if (_periodic_dirs.contains(i))
    {
      BoundaryID current_boundary_id = getBoundaryID(boundary_names[i]);

      for (auto it = bndElemsBegin(); it != bndElemsEnd(); ++it)
        if ((*it)->_bnd_id == current_boundary_id)
        {
          Elem * elem = (*it)->_elem;
          unsigned short int s = (*it)->_side;

          // build element from the side
          std::unique_ptr<Elem> side(elem->build_side(s, false));
          side->processor_id() = elem->processor_id();

          // Add the side set subdomain
          Elem * new_elem = _mesh->add_elem(side.release());
          _mortar_subdomains[i] = 10 + i;
          new_elem->subdomain_id() = _mortar_subdomains[i];

          // TODO: this does not assign unique IDs
        }
    }
}
