#include "Init.h"
#include "Parser.h"
#include "Executioner.h"
#include "App.h"

#if 0
//////
#include "Diffusion.h"

#include "fe.h"
#include "boundary_info.h"
#include "mesh_refinement.h"
#include "mesh_generation.h"

const std::string mesh_file_name("square.e");

void
addKernels(SubProblem &p)
{
  InputParameters pars;
  
  pars.set<std::string>("variable") = "u";
  p.addKernel("Diffusion", "diff", pars);

/*  pars.set<std::string>("variable") = "u";
  pars.set<std::string>("coupled_var_name") = "v";
  p.addKernel("Coupled", "cpl", pars);
*/
  pars.set<std::string>("variable") = "u";
  p.addKernel("ForcingFn", "ffn", pars);

/*
  // V
  pars.set<std::string>("var_name") = "v";
  p.addKernel("Diffusion", "diff_v", pars);

  pars.set<std::string>("var_name") = "v";
  p.addKernel("ForcingFn", "ffn", pars);
*/
}

void
addBCs(SubProblem &p)
{
  InputParameters pars;

  // V
  std::vector<unsigned int> bnd1(4);
  bnd1[0] = 0;
  bnd1[1] = 1;
  bnd1[2] = 2;
  bnd1[3] = 3;

  pars.set<std::vector<unsigned int> >("boundary") = bnd1;
  pars.set<std::string>("variable") = "u";
  p.addBoundaryCondition("DirichletBC", "all", pars);

//  std::vector<unsigned int> bnd2(1, 2);
//  pars.set<std::vector<unsigned int> >("boundary") = bnd2;
//  pars.set<Real>("value") = 0;
//  p.addBoundaryCondition("DirichletBC", "right", pars);

/*
  // U
  std::vector<unsigned int> bnd3(1, 1);

  pars.set<std::vector<unsigned int> >("boundary") = bnd1;
  pars.set<Real>("value") = 0;
  pars.set<std::string>("var_name") = "u";
  p.addBoundaryCondition("DirichletBC", "v_left", pars);

//  std::vector<unsigned int> bnd4(1, 3);
//  pars.set<std::vector<unsigned int> >("boundary") = bnd4;
//  pars.set<Real>("value") = 0;
//  p.addBoundaryCondition("DirichletBC", "v_right", pars);
*/
}

void
setupProblem(Problem &p)
{
  SubProblem *nl = p.addImplicitSystem("nl");
  nl->addVariable("u", FEType(FIRST, LAGRANGE));
//  nl->addVariable("v", FEType(FIRST, LAGRANGE));

  addKernels(*nl);
  addBCs(*nl);

  p.init();
}

int
main(int argc, char *argv[])
{
  Init init(argc, argv);
  App::registerObjects();

  libMesh::Mesh mesh(2);
//  mesh.read(mesh_file_name);
  MeshTools::Generation::build_square(mesh,
      5, 5,
      -1, 1,
      -1, 1,
      libMeshEnums::QUAD4);


//  MeshRefinement mr(mesh);
//  mr.uniformly_refine(2);

  mesh.boundary_info->build_node_list_from_side_list();

  Problem p(mesh);
  setupProblem(p);

  p.solve();

  p.write_output("out.e");


  return 0;
}

#endif


int
main(int argc, char *argv[])
{
  Init init(argc, argv);
  App::registerObjects();

  Parser p;

  std::string input_filename = "";
  if (Moose::command_line->search("-i"))
    input_filename = Moose::command_line->next(input_filename);
  else
    p.printUsage();

  p.parse(input_filename);
  p.execute();

  Executioner &e = p.getExecutioner();
  e.execute();

  return 0;
}
