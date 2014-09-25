#include "ReconVarIC.h"
#include "MooseMesh.h"

template<>
InputParameters validParams<ReconVarIC>()
{

  InputParameters params = validParams<InitialCondition>();
  params.addRequiredParam<UserObjectName>("ebsd_reader", "The EBSDReader GeneralUserObject");
  params.addRequiredParam<bool>("consider_phase","If true, IC will only act on one phase");
  params.addParam<unsigned int>("phase", 0,"EBSD phase number to be assigned to this grain");
  params.addParam<bool>("all_to_one",false,"If true, assign all grain numbers in this phase to the same variable");
  params.addParam<unsigned int>("op_num", 0, "Specifies the number of order paraameters to create, all_to_one = false");
  params.addParam<unsigned int>("op_index", 0,"The index for the current order parameter, if all_to_one = false");
  return params;
}

ReconVarIC::ReconVarIC(const std::string & name,InputParameters parameters) :
    InitialCondition(name, parameters),
    _mesh(_fe_problem.mesh()),
    _nl(_fe_problem.getNonlinearSystem()),
    _ebsd_reader(getUserObject<EBSDReader>("ebsd_reader")),
    _consider_phase(getParam<bool>("consider_phase")),
    _phase(getParam<unsigned int>("phase")),
    _all_to_one(getParam<bool>("all_to_one")),
    _op_num(getParam<unsigned int>("op_num")),
    _op_index(getParam<unsigned int>("op_index"))
{
  if (!_consider_phase && _all_to_one)
    mooseError("In ReconVarIC, if you are not considering phase, you can't assign all grains to one variable");
}

void
ReconVarIC::initialSetup()
{
  //Get the number of grains from the EBSD data
  _grain_num = _ebsd_reader.getGrainNum();
  

  if (!_all_to_one)
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
    
    _assigned_op = PolycrystalICTools::assignPointsToVariables(_centerpoints,_op_num, _mesh, _var);
    
  }
}


// Note that we are not actually using Point coordinates that get passed in to assign the order parameter.
// By knowing the curent elements index, we can use it's centroid to grab the EBSD grain index
// associated with the point from the EBSDReader user object.
Real
ReconVarIC::value(const Point &)
{
  const Point p1 = _current_elem->centroid();
  const EBSDReader::EBSDPointData & d = _ebsd_reader.getData(p1);
  const unsigned int grn_index = d.grain;
  const unsigned int phase_index = d.phase;

  if (!_consider_phase) //This is the old methodology where everything is an order parameter
  {
    if (_assigned_op[grn_index] == _op_index && phase_index == _phase)
      return 1.0;
    else
      return 0.0;
  }
  else
  {
    if (_all_to_one) //Ever part of this phase will be assigned to the same variable
    {
      if (phase_index == _phase)
        return 1.0;
      else
        return 0.0;
    }
    else //For the given phase, each grain is assigned to the corresponding op. The +1 is because we start grain numbering in phase 1 at 1. Once we stop doing that, we can get rid of the +1
    {
      if (_assigned_op[grn_index] == _op_index+1 && phase_index == _phase)
        return 1.0;
      else
        return 0.0;
    }
  }
}
