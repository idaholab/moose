#include "HeatFluxFromHeatStructureBaseUserObject.h"
#include "MooseMesh.h"
#include "KDTree.h"
#include "Assembly.h"

template <>
InputParameters
validParams<HeatFluxFromHeatStructureBaseUserObject>()
{
  InputParameters params = validParams<ElementUserObject>();
  params.addRequiredParam<std::vector<BoundaryName>>("slave_boundary",
                                                     "Boundary name on the flow channel mesh");
  params.addRequiredParam<BoundaryName>("master_boundary",
                                        "Boundary name on the heat structure mesh");
  params.addClassDescription(
      "Base class for caching heat flux between flow channels and heat structures.");
  return params;
}

HeatFluxFromHeatStructureBaseUserObject::HeatFluxFromHeatStructureBaseUserObject(
    const InputParameters & parameters)
  : ElementUserObject(parameters)
{
  // master element centroids
  std::vector<Point> master_points;
  // element ids corresponding to the centroids in `master_points`
  std::vector<dof_id_type> master_elem_ids;
  // slave element centroids
  std::vector<Point> slave_points;
  // element ids corresponding to the centroids in `slave_points`
  std::vector<dof_id_type> slave_elem_ids;

  // list of q-points per master elements
  std::map<dof_id_type, std::vector<Point>> master_elem_qps;
  // list of q-points per slave elements
  std::map<dof_id_type, std::vector<Point>> slave_elem_qps;

  BoundaryID master_bnd_id = _mesh.getBoundaryID(getParam<BoundaryName>("master_boundary"));
  std::vector<BoundaryID> slave_bnd_id =
      _mesh.getBoundaryIDs(getParam<std::vector<BoundaryName>>("slave_boundary"));

  ConstBndElemRange & range = *_mesh.getBoundaryElementRange();
  for (const auto & belem : range)
  {
    const Elem * elem = belem->_elem;
    BoundaryID boundary_id = belem->_bnd_id;

    if (elem->processor_id() == _subproblem.processor_id())
    {
      _assembly.setCurrentSubdomainID(elem->subdomain_id());
      if (boundary_id == master_bnd_id)
      {
        // 2D elements
        _assembly.reinit(elem, belem->_side);
        const MooseArray<Point> & q_points = _assembly.qPointsFace();
        for (std::size_t i = 0; i < q_points.size(); i++)
          master_elem_qps[elem->id()].push_back(q_points[i]);

        master_elem_ids.push_back(elem->id());
        master_points.push_back(elem->centroid());
      }
      else if (std::find(slave_bnd_id.begin(), slave_bnd_id.end(), boundary_id) !=
               slave_bnd_id.end())
      {
        if (std::find(slave_elem_ids.begin(), slave_elem_ids.end(), elem->id()) ==
            slave_elem_ids.end())
        {
          // 1D elements
          _assembly.reinit(elem);
          const MooseArray<Point> & q_points = _assembly.qPoints();
          for (std::size_t i = 0; i < q_points.size(); i++)
            slave_elem_qps[elem->id()].push_back(q_points[i]);

          slave_elem_ids.push_back(elem->id());
          slave_points.push_back(elem->centroid());
        }
      }
    }
  }

  // find the master elements that are nearest to the slave elements
  KDTree kd_tree(master_points, _mesh.getMaxLeafSize());
  for (std::size_t i = 0; i < slave_points.size(); i++)
  {
    unsigned int patch_size = 1;
    std::vector<std::size_t> return_index(patch_size);
    kd_tree.neighborSearch(slave_points[i], patch_size, return_index);

    _nearest_elem_ids.insert(
        std::pair<dof_id_type, dof_id_type>(slave_elem_ids[i], master_elem_ids[return_index[0]]));
  }
  // now find out how q-points correspond to each other on the (master, slave) pair of elements
  for (std::size_t i = 0; i < slave_elem_ids.size(); i++)
  {
    dof_id_type elem_id = slave_elem_ids[i];
    dof_id_type nearest_elem_id = _nearest_elem_ids[elem_id];

    std::vector<Point> & slave_qps = slave_elem_qps[elem_id];
    std::vector<Point> & master_qps = master_elem_qps[nearest_elem_id];
    mooseAssert(slave_qps.size() == master_qps.size(),
                "Number of master and slave q-points have to match");
    _elem_qp_map[elem_id].resize(slave_qps.size());
    KDTree kd_tree_qp(master_qps, 5);
    for (std::size_t i = 0; i < slave_qps.size(); i++)
    {
      unsigned int patch_size = 1;
      std::vector<std::size_t> return_index(patch_size);
      kd_tree_qp.neighborSearch(slave_qps[i], patch_size, return_index);
      _elem_qp_map[elem_id][i] = return_index[0];
    }
  }
}

void
HeatFluxFromHeatStructureBaseUserObject::initialize()
{
}

void
HeatFluxFromHeatStructureBaseUserObject::execute()
{
  unsigned int n_qpts = _qrule->n_points();
  dof_id_type nearest_elem_id = _nearest_elem_ids[_current_elem->id()];

  _heat_flux[_current_elem->id()].resize(n_qpts);
  _heat_flux[nearest_elem_id].resize(n_qpts);
  for (_qp = 0; _qp < n_qpts; _qp++)
  {
    unsigned int nearest_qp = _elem_qp_map[_current_elem->id()][_qp];
    Real q_wall = computeQpHeatFlux();

    _heat_flux[_current_elem->id()][_qp] = q_wall;
    _heat_flux[nearest_elem_id][nearest_qp] = q_wall;
  }

  if (_fe_problem.currentlyComputingJacobian())
  {
    _heat_flux_jacobian[_current_elem->id()].resize(n_qpts);
    _heat_flux_jacobian[nearest_elem_id].resize(n_qpts);
    for (_qp = 0; _qp < n_qpts; _qp++)
    {
      unsigned int nearest_qp = _elem_qp_map[_current_elem->id()][_qp];
      DenseVector<Real> jac = computeQpHeatFluxJacobian();

      _heat_flux_jacobian[_current_elem->id()][_qp] = jac;
      _heat_flux_jacobian[nearest_elem_id][nearest_qp] = jac;
    }
  }
}

void
HeatFluxFromHeatStructureBaseUserObject::finalize()
{
}

void
HeatFluxFromHeatStructureBaseUserObject::threadJoin(const UserObject & y)
{
  const HeatFluxFromHeatStructureBaseUserObject & uo =
      static_cast<const HeatFluxFromHeatStructureBaseUserObject &>(y);
  for (auto & it : uo._heat_flux)
    _heat_flux[it.first] = it.second;
  for (auto & it : uo._heat_flux_jacobian)
    _heat_flux_jacobian[it.first] = it.second;
}

const std::vector<Real> &
HeatFluxFromHeatStructureBaseUserObject::getHeatFlux(dof_id_type element_id) const
{
  Threads::spin_mutex::scoped_lock lock(Threads::spin_mtx);
  auto it = _heat_flux.find(element_id);
  if (it != _heat_flux.end())
    return it->second;
  else
    mooseError(name(), ": Requested heat flux for element ", element_id, " was not computed.");
}

const std::vector<DenseVector<Real>> &
HeatFluxFromHeatStructureBaseUserObject::getHeatFluxJacobian(dof_id_type element_id) const
{
  Threads::spin_mutex::scoped_lock lock(Threads::spin_mtx);
  auto it = _heat_flux_jacobian.find(element_id);
  if (it != _heat_flux_jacobian.end())
    return it->second;
  else
    mooseError(
        name(), ": Requested heat flux jacobian for element ", element_id, " was not computed.");
}
