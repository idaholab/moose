#define nls _problem_ptr->getNonlinearSystem()
#define scaledValue ((dv(0) + shiftValue) / scalingMax)

#include <fstream>
// #include <png.h>
#include "PNGTest.h"
#include "libmesh/mesh_tools.h"

registerMooseObject("MooseApp", PNGTest);

template <>
InputParameters
validParams<PNGTest>()
{
  InputParameters params = validParams<Output>();
  params.addParam<Real>("resolution", 2000, "The resolution of the image.");
  params.addParam<std::string>("PNGFile", "Adam", "Root filename of the PNG to be created.");
  params.addParam<Real>("picked", -1, "PNG to save.");
  params.addParam<Real>("distx", 1, "How wide.");
  params.addParam<Real>("disty", 1, "How tall.");
  params.addParam<bool>("inColor", false, "Show the image in color?");
  return params;
}

//function currently used to determine colors
void setRGB(png_byte *rgb, float selection) {
 	int color = (int)(selection * 767);
  // If this isn't true, something off with our scaling function,
  // because everything should be between 1 and 0. (Or math is broken)
 	assert(color > 0 && color < 767);

 	int magnitude = color % 256;

 	if (color < 256)
 		rgb[0] = magnitude; rgb[1] = 0; rgb[2] = 255-magnitude;
 	else if (color < 512)
 		rgb[0] = 255; rgb[1] = magnitude; rgb[2] = 0;
 	else
 		rgb[0] = 255; rgb[1] = 255; rgb[2] = magnitude;
}

PNGTest::PNGTest(const InputParameters & parameters) :
Output(parameters),
resolution(getParam<Real>("resolution")),
PNGFile(getParam<std::string>("PNGFile")),
picked(getParam<Real>("picked")),
xdist(getParam<Real>("distx")),
ydist(getParam<Real>("disty")),
inColor(getParam<bool>("inColor"))
//meters(parameters)
{
// Transform transs(_pars).modify();
}

// Funtion for making the _mesh_function object.
void PNGTest::makeMeshFunc() {

  const std::vector<unsigned int> var_nums = {0};

//  _fe_problem->addUserObject("Output", "output", meters);

  // Find the values that will be used for rescaling purposes.
  calculateRescalingValues();

  // Set up the mesh_function
  _mesh_function = libmesh_make_unique<MeshFunction>(
      *_es_ptr, nls.serializedSolution(), nls.dofMap(), var_nums);
  _mesh_function->init();
  const DenseVector<Number> num(1);// = .5;
  _mesh_function->enable_out_of_mesh_mode(num);
}

void PNGTest::calculateRescalingValues()
{
  // The min and max.
  scalingMin = nls.serializedSolution().min();
  scalingMax = nls.serializedSolution().max();
  shiftValue = 0;

  if(scalingMin < 0)
  {
    shiftValue += scalingMin;
  }
  else if(scalingMin > 0)
  {
    shiftValue -= scalingMin;
  }

  scalingMax += shiftValue;

  // for(auto i = 0; i < nls.serializedSolution().size(); i++)
  // {
  //   vecx[i] = (vecx[i] + shiftValue) * (1/scalingMax);
  // }

}

// Initialize by running the makeMeshFunc method.
// void PNGTest::initialize() {
//
//   makeMeshFunc();
// }
//
// void PNGTest::execute() {
//
//   makePNG();
// }

void PNGTest::output(const ExecFlagType & /*type*/)
{
  makeMeshFunc();
  box = MeshTools::create_bounding_box(*_mesh_ptr);
  makePNG();
}

void PNGTest::makePNG() {

//   if(appe == "three")
//     appe = "four";
//
//   else if(appe == "two")
//     appe = "three";
//
//   else if(appe == "one")
//     appe = "two";
//
//   std::string PNGFile = "Adam";
//
//   PNGFile += appe;
//   PNGFile += ".png";

  numpng++;

//  BoundingBox box = MeshTools::create_bounding_box(*_mesh_ptr);

  //FindBBox fbbx;
  //std::cout << "The min and the max " << (comm().min(box.min()))(0) << comm().max(box.max()) << "\n";

//  _mesh_ptr->comm(); ----------- communicator we have access to.
//  comm(); ------------------ Another communicator we have access to.
  //_app._comm ----- this is the communicator.  It's protected.



  BoundingBox boxy = _mesh_ptr->getInflatedProcessorBoundingBox(.1);

  Point ptx = box.max();
  Point ptn = box.min();

  //_mesh_ptr->comm().min(ptn);

  Real distx = ptx(0) - ptn(0);
  Real disty = ptx(1) - ptn(1);

  std::cout << "Values: " << distx << " " << disty << "\n";

  std::string PNGFile2 = PNGFile;
  PNGFile2 += std::to_string((int)numpng);

  FILE * fp = nullptr;
  png_structrp pngp = nullptr;
  png_infop infop = nullptr;
  png_bytep row = nullptr;
//    int status = 1;
  Real depth = 8;
//  Real resolution = 3000;
  // Real xdist = 2;
  // Real ydist = 2;
  Real width = distx*resolution;
  Real height = disty*resolution;

  if(picked != -1) {
    if(numpng != picked) {
      std::cout << "Not the picked one.";
      return;
    }
    std::cout << "The picked one." << PNGFile2;
  }

  else
    std::cout << "Nothing selected.";

//  std::cout << "\n\n\nFile opening!\n\n\n";
  fp = fopen(PNGFile2.c_str(), "wb");
//  std::cout << "\n\n\nFile OPENED!\n\n\n";

  if(!fp) {
    std::cout << "Failed to open.";
    return;
  }

  pngp = png_create_write_struct(PNG_LIBPNG_VER_STRING, nullptr, nullptr, nullptr);
  if(!pngp) {
    std::cout << "Failed to make pointer string.";
    return;
  }

  infop = png_create_info_struct(pngp);
  if(!infop) {
    std::cout << "Failed to make info pointer.";
    return;
  }

  png_init_io(pngp, fp);

//  std::cout << "\n\n\nRight before the IDHR!\n\n\n";

  png_set_IHDR (pngp,
                infop,
                width,
                height,
                depth,
                (inColor?PNG_COLOR_TYPE_RGB:PNG_COLOR_TYPE_GRAY),
                PNG_INTERLACE_NONE,
                PNG_COMPRESSION_TYPE_DEFAULT,
                PNG_FILTER_TYPE_DEFAULT);
//  std::cout << "\n\n\nRight before info write!\n\n\n";

  png_write_info(pngp, infop);

//  row = (png_bytep) malloc(width * sizeof(png_byte));
//std::cout << "\n\n\nRight before setting row!\n\n\n";

  row = new png_byte[(width * 3)+1];

//  Point * p(0);
//  Point pt(*p);
//std::cout << "\n\n\nPOINT!\n\n\n";

  Point pt(1,1,0);
//  std::cout << "\n\n\nvec!\n\n\n";
  std::vector<double> v ={5};
//  std::cout << "\n\n\ndv!\n\n\n";
  DenseVector<Number> dv(0);

//   row = (png_bytep) malloc(width * sizeof(png_byte));
  // int grayscale[((int)(resolution*resolution))];
  // for (int i = 0; i < resolution*resolution; i++)
  //   grayscale[i] = rand() % 256;


//int colorconverter;
//  std::cout << "Outside";
//  int indy = 0;

//std::cout << "\n\n\nRight before the loop!\n\n\n";
  for (Real y=ptx(1) ; y>=ptn(1) ; y -= 1/resolution) {
    int indx = 0;
    for (Real x=ptn(0) ; x<=ptx(0) ; x += 1/resolution) {
//      std::cout << "Inside0";
      pt(0) = x;
//      std::cout << "\n\n\nBefore meshfunction!\n\n\n";
      (*_mesh_function)(pt, _time, dv, nullptr);

//      colorconverter = ((int)(dv(x)*10000))%767;
      if(inColor)
        setRGB(&row[indx*3], scaledValue);
      else {
//        std::cout << "Seggers";
//std::cout << "\n\n\nJust set the row at index to value dv(x) = " << dv(0) << "\n\n\n";
        row[indx] = scaledValue * 256;//grayscale[((int)(x*y*resolution*resolution))];
//      std::cerr << "The DV value is: " << dv(x) << "\n";

      }
      indx++;
//      std::cout << "Inside1";
    }
      pt(1) = y;
      png_write_row(pngp, row);
//      std::cout << "Inside2";
  }

  png_write_end(pngp, nullptr);

  if (fp != nullptr) fclose(fp);
  if (infop != nullptr) png_free_data(pngp, infop, PNG_FREE_ALL, -1);
  if (pngp != nullptr) png_destroy_write_struct(&pngp, (png_infopp)nullptr);
  if (row != nullptr) delete[] row;
}
