#include "ReconVarIC.h"
#include "MooseMesh.h"

template<>
InputParameters validParams<ReconVarIC>()
{

  InputParameters params = validParams<InitialCondition>();
  params.addRequiredParam<UserObjectName>("ebsd_reader", "The EBSDReader GeneralUserObject");
  params.addRequiredParam<unsigned int>("crys_num", "Specifies the number of order paraameters to create");
  params.addRequiredParam<unsigned int>("grain_num", "Specifies the number of grains in the reconstructed dataset");
  params.addRequiredParam<unsigned int>("crys_index", "The index for the current crystal");
  return params;
}

ReconVarIC::ReconVarIC(const std::string & name,InputParameters parameters) :
    InitialCondition(name, parameters),
    _mesh(_fe_problem.mesh()),
    _nl(_fe_problem.getNonlinearSystem()),
    _ebsd_reader(getUserObject<EBSDReader>("ebsd_reader")),
    _op_num(getParam<unsigned int>("crys_num")),
    _grain_num(getParam<unsigned int>("grain_num")),
    _op_index(getParam<unsigned int>("crys_index"))
{}

void
ReconVarIC::initialSetup()
{
  // Read in EBSD data from user object
  // unsigned int n_elem =  _mesh.getMesh().n_active_elem();
  const MeshBase::element_iterator begin = _mesh.getMesh().active_elements_begin();
  const MeshBase::element_iterator end = _mesh.getMesh().active_elements_end();
  for (MeshBase::element_iterator el = begin; el != end; ++el)
  {
    Elem *current_elem = *el;
    unsigned int index = current_elem->id();
    Point p0 = current_elem->centroid();
    _grn[index] = _ebsd_reader.get_data(p0, _ebsd_reader.getDataType("GRAIN"));
    _x[index] = _ebsd_reader.get_data(p0, _ebsd_reader.getDataType("X"));
    _y[index] = _ebsd_reader.get_data(p0, _ebsd_reader.getDataType("Y"));
    _z[index] = _ebsd_reader.get_data(p0, _ebsd_reader.getDataType("Z"));
    // Moose::out << "Element #, Grain #, X, Y, Z:  " << current_elem->id()  << "  " << _grn[index] << "  " << _x[index] << "  " << _y[index] << "  " << _z[index] << "\n" << std::endl;
  }

  // Calculate centerpoint of each EBSD grain
  _sum_x.resize(_grain_num);
  _sum_y.resize(_grain_num);
  _sum_z.resize(_grain_num);
  _centerpoints.resize(_grain_num);

  unsigned int num_pts;
  for (unsigned int i = 0; i < _grain_num; i++)
  {
    num_pts = 0;
    for (std::map<unsigned int, unsigned int>::iterator it = _grn.begin(); it != _grn.end(); ++it)
    {
      if (it->second == i)
      {
        _sum_x[i] += _x[it->first];
        _sum_y[i] += _y[it->first];
        _sum_z[i] += _z[it->first];
        num_pts += 1;
      }
    }
    _centerpoints[i](0) = _sum_x[i] / num_pts;
    _centerpoints[i](1) = _sum_y[i] / num_pts;
    _centerpoints[i](2) = _sum_z[i] / num_pts;
    // Moose::out << _centerpoints[i] << "\n" << std::endl;
  }

  // Set up domain bounds with mesh tools
  for (unsigned int i = 0; i < LIBMESH_DIM; i++)
  {
    _bottom_left(i) = _mesh.getMinInDimension(i);
    _top_right(i) = _mesh.getMaxInDimension(i);
  }
  _range = _top_right - _bottom_left;

  // Output error message if number of order parameters is larger than number of grains from EBSD dataset
  if (_op_num > _grain_num)
     mooseError("ERROR in PolycrystalReducedIC: Number of order parameters (crys_num) can't be larger than the number of grains (grain_num)");

  // Assign grains to each order parameter in a way that maximizes distance
  _assigned_op.resize(_grain_num);
  for (unsigned int grain=0; grain < _grain_num; grain++)
  {
    std::vector<int> min_op_ind;
    std::vector<Real> min_op_dist;
    min_op_ind.resize(_op_num);
    min_op_dist.resize(_op_num);

      // Determine the distance to the closest center assigned to each order parameter
    if (grain >= _op_num)
    {
      std::fill(min_op_dist.begin() , min_op_dist.end(), _range.size());
      for (unsigned int i=0; i<grain; i++)
      {
        Real dist =  _mesh.minPeriodicDistance(_var.number(), _centerpoints[grain], _centerpoints[i]);
        if (min_op_dist[_assigned_op[i]] > dist)
        {
          min_op_dist[_assigned_op[i]] = dist;
          min_op_ind[_assigned_op[i]] = i;
        }
      }
    }

    // Assign the current center point to the order parameter that is furthest away.
    Real mx;
    if (grain < _op_num)
        _assigned_op[grain] = grain;
    else
    {
      mx = 0.0;
      unsigned int mx_ind = 1e6;
      for (unsigned int i = 0; i < _op_num; i++) // Find index of max
        if (mx < min_op_dist[i])
        {
          mx = min_op_dist[i];
          mx_ind = i;
        }
      _assigned_op[grain] = mx_ind;
    }
    // Moose::out << "For grain " << grain << ", center point = " << _centerpoints[grain](0) << " " << _centerpoints[grain](1) << "\n";
    // Moose::out << "Max index is " << _assigned_op[grain] << ", with a max distance of " << mx << "\n";
  }
}

// Note that we are not actually using Point coordinates that get passed in to assign the order parameter.
// By knowing the curent elements index, we can use it's centroid to grab the EBSD grain index
// associated with the point from the EBSDReader user object.
Real
ReconVarIC::value(const Point &)
{
  Real op = 0.0;
  Point p1 = _current_elem->centroid();
  unsigned int grn_index = _ebsd_reader.get_data(p1, _ebsd_reader.getDataType("GRAIN"));
  if (_assigned_op[grn_index] == _op_index)
    op = 1.0;
  else
    op = 0.0;
  // Moose::out << _current_elem->id() << "  " << p << " " << op <<  "\n" << std::endl;
  return op;
}
