namespace Moose
{
  void gatherNearbyElements (MooseMesh & moose_mesh, const std::set<unsigned int> & boundaries_to_ghost, const std::vector<Real> & inflation);
}

