#include "DetailedPinMeshGeneratorBase.h"
#include "libmesh/cell_prism6.h"

InputParameters
DetailedPinMeshGeneratorBase::validParams()
{
  InputParameters params = MeshGenerator::validParams();
  params.addRequiredParam<Real>("pitch", "Pitch [m]");
  params.addRequiredParam<Real>("rod_diameter", "Rod diameter [m]");
  params.addParam<Real>("unheated_length_entry", 0.0, "Unheated length at entry [m]");
  params.addRequiredParam<Real>("heated_length", "Heated length [m]");
  params.addParam<Real>("unheated_length_exit", 0.0, "Unheated length at exit [m]");
  params.addRequiredParam<unsigned int>("n_cells", "The number of cells in the axial direction");
  params.addParam<unsigned int>("block_id", 1, "Block ID used for the mesh subdomain.");
  params.addRangeCheckedParam<unsigned int>("num_radial_parts",
                                            16,
                                            "num_radial_parts>=4",
                                            "Number of radial parts (must be at least 4).");
  return params;
}

DetailedPinMeshGeneratorBase::DetailedPinMeshGeneratorBase(const InputParameters & parameters)
  : MeshGenerator(parameters),
    _unheated_length_entry(getParam<Real>("unheated_length_entry")),
    _heated_length(getParam<Real>("heated_length")),
    _unheated_length_exit(getParam<Real>("unheated_length_exit")),
    _pitch(getParam<Real>("pitch")),
    _rod_diameter(getParam<Real>("rod_diameter")),
    _n_cells(getParam<unsigned int>("n_cells")),
    _block_id(getParam<unsigned int>("block_id")),
    _num_radial_parts(getParam<unsigned int>("num_radial_parts"))
{
  Real L = _unheated_length_entry + _heated_length + _unheated_length_exit;
  Real dz = L / _n_cells;
  for (unsigned int i = 0; i < _n_cells + 1; i++)
    _z_grid.push_back(dz * i);
}

void
DetailedPinMeshGeneratorBase::generatePin(std::unique_ptr<MeshBase> & mesh_base,
                                          const Point & center)
{
  const Real dalpha = 360. / _num_radial_parts;
  Real radius = _rod_diameter / 2.;

  // nodes
  std::vector<std::vector<Node *>> nodes;
  nodes.resize(_n_cells + 1);
  for (unsigned int k = 0; k < _n_cells + 1; k++)
  {
    const Real elev = _z_grid[k];
    // center node
    nodes[k].push_back(mesh_base->add_point(Point(center(0), center(1), elev)));
    // ring around the center
    Real alpha = 0.;
    for (unsigned int i = 0; i < _num_radial_parts; i++, alpha += dalpha)
    {
      const Real dx = radius * std::cos(alpha * M_PI / 180.);
      const Real dy = radius * std::sin(alpha * M_PI / 180.);
      Point pt(center(0) + dx, center(1) + dy, elev);
      nodes[k].push_back(mesh_base->add_point(pt));
    }
  }

  // elements
  for (unsigned int k = 0; k < _n_cells; k++)
  {
    for (unsigned int i = 0; i < _num_radial_parts; i++)
    {
      Elem * elem = new Prism6;
      elem->subdomain_id() = _block_id;
      elem->set_id(_elem_id++);
      mesh_base->add_elem(elem);
      const unsigned int ctr_idx = 0;
      const unsigned int idx1 = (i % _num_radial_parts) + 1;
      const unsigned int idx2 = ((i + 1) % _num_radial_parts) + 1;
      elem->set_node(0) = nodes[k][ctr_idx];
      elem->set_node(1) = nodes[k][idx1];
      elem->set_node(2) = nodes[k][idx2];
      elem->set_node(3) = nodes[k + 1][ctr_idx];
      elem->set_node(4) = nodes[k + 1][idx1];
      elem->set_node(5) = nodes[k + 1][idx2];
    }
  }
}
