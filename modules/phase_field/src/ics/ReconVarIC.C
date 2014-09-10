#include "ReconVarIC.h"
#include "MooseMesh.h"

template<>
InputParameters validParams<ReconVarIC>()
{

  InputParameters params = validParams<InitialCondition>();
  params.addRequiredParam<UserObjectName>("ebsd_reader", "The EBSDReader GeneralUserObject");
  params.addRequiredParam<unsigned int>("op_num", "Specifies the number of order paraameters to create");
  params.addRequiredParam<unsigned int>("grain_num", "Specifies the number of grains in the reconstructed dataset");
  params.addRequiredParam<unsigned int>("op_index", "The index for the current crystal");
  return params;
}

ReconVarIC::ReconVarIC(const std::string & name,InputParameters parameters) :
    InitialCondition(name, parameters),
    _mesh(_fe_problem.mesh()),
    _nl(_fe_problem.getNonlinearSystem()),
    _ebsd_reader(getUserObject<EBSDReader>("ebsd_reader")),
    _op_num(getParam<unsigned int>("op_num")),
    _grain_num(getParam<unsigned int>("grain_num")),
    _op_index(getParam<unsigned int>("op_index"))
{
}

void
ReconVarIC::initialSetup()
{
  // Read in EBSD data from user object
  // unsigned int n_elem =  _mesh.getMesh().n_active_elem();
  const MeshBase::element_iterator begin = _mesh.getMesh().active_elements_begin();
  const MeshBase::element_iterator end = _mesh.getMesh().active_elements_end();
  for (MeshBase::element_iterator el = begin; el != end; ++el)
  {
    Elem * current_elem = *el;
    unsigned int index = current_elem->id();
    Point p0 = current_elem->centroid();
    const EBSDReader::EBSDPointData & d = _ebsd_reader.getData(p0);
    _gp[index].grain = d.grain;
    _gp[index].p = d.p;
  }

  // Calculate centerpoint of each EBSD grain
  _centerpoints.resize(_grain_num);
  std::vector<unsigned int> num_pts(_grain_num);
  for (unsigned int i = 0; i < _grain_num; i++)
  {
    _centerpoints[i] = 0.0;
    num_pts[i] = 0;
  }

  for (std::map<unsigned int, GrainPoint>::iterator it = _gp.begin(); it != _gp.end(); ++it)
  {
    _centerpoints[it->second.grain] += it->second.p;
    num_pts[it->second.grain]++;
  }

  for (unsigned int i = 0; i < _grain_num; i++)
  {
    if (num_pts[i] == 0) continue;
    _centerpoints[i] *= 1.0 / Real(num_pts[i]);
  }

  // Output error message if number of order parameters is larger than number of grains from EBSD dataset
  if (_op_num > _grain_num)
     mooseError("ERROR in PolycrystalReducedIC: Number of order parameters (op_num) can't be larger than the number of grains (grain_num)");

  // Assign grains to each order parameter in a way that maximizes distance
  _assigned_op.resize(_grain_num);
  std::vector<int> min_op_ind;
  std::vector<Real> min_op_dist;
  min_op_ind.resize(_op_num);
  min_op_dist.resize(_op_num);
  for (unsigned int grain=0; grain < _grain_num; grain++)
  {
    // Determine the distance to the closest center assigned to each order parameter
    if (grain >= _op_num)
    {
      // We can set the array to the distances to the grains 0.._op_num-1 (see assignment in the else case)
      for (unsigned int i=0; i<_op_num; ++i)
      {
        min_op_dist[i] = _mesh.minPeriodicDistance(_var.number(), _centerpoints[grain], _centerpoints[i]);
        min_op_ind[_assigned_op[i]] = i;
      }

      // Now check if any of the extra grains are even closer
      for (unsigned int i=_op_num; i<grain; ++i)
      {
        Real dist = _mesh.minPeriodicDistance(_var.number(), _centerpoints[grain], _centerpoints[i]);
        if (min_op_dist[_assigned_op[i]] > dist)
        {
          min_op_dist[_assigned_op[i]] = dist;
          min_op_ind[_assigned_op[i]] = i;
        }
      }
    }
    else
    {
      _assigned_op[grain] = grain;
      continue;
    }

    // Assign the current center point to the order parameter that is furthest away.
    unsigned int mx_ind = 0;
    for (unsigned int i = 1; i < _op_num; i++) // Find index of max
      if (min_op_dist[mx_ind] < min_op_dist[i])
        mx_ind = i;

    _assigned_op[grain] = mx_ind;
  }
}

// Note that we are not actually using Point coordinates that get passed in to assign the order parameter.
// By knowing the curent elements index, we can use it's centroid to grab the EBSD grain index
// associated with the point from the EBSDReader user object.
Real
ReconVarIC::value(const Point &)
{
  const Point p1 = _current_elem->centroid();
  const unsigned int grn_index = _ebsd_reader.getData(p1).grain;;

  if (_assigned_op[grn_index] == _op_index)
    return 1.0;
  else
    return 0.0;
}
