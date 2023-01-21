!content pagination previous=tutorial01_app_development/step05_kernel_object.md
                    next=tutorial01_app_development/step07_parallel.md
                    margin-bottom=0px

# Step 6: Define a Set of Input Parameters

This step introduces the class used to a define a unique set of parameters for a `MooseObject`. The values for these parameters can be specified by users via input files. For the demonstration, the `DarcyPressure` class, which was created in the [previous step](tutorial01_app_development/step05_kernel_object.md#source-demo), will be modified to accept any arbitrary real numbers for the `_permeability` and `_viscosity` variables instead of hard-coding ones.

## Input Parameters

Every `MooseObject` includes a set of custom parameters within a single [`InputParameters`](framework/include/utils/InputParameters.h) object whose values may be controlled by its user via an input file. These parameters are defined in the `validParams()` method. This method is `static` to allow for it to be called independently and prior to object construction. The output is then parsed and verified by the core systems of MOOSE before it becomes accessible to the constructor methods and other members for each instance of the associated `MooseObject`.

### Declaring Valid Parameters

When coding a `validParams()` function, it is customary to declare a `params` variable and initialize it with the `validParams()` output from the base class, e.g.,

!listing language=C++
InputParameters params = ADKernelGrad::validParams();

This initialization means that the parameters for the given `MooseObject` always includes those which are defined for the base class. These parameters provide standard usability for inherited classes, e.g., `"variable"`, `"block"`, or `"boundary"`.

The first item appended to `params` should be an `addClassDescription()` object, which has already been demonstrated in the `DarcyPressure.C` file:

!listing tutorials/tutorial01_app_development/step05_kernel_object/src/kernels/DarcyPressure.C
         link=False
         start=params.addClassDescription
         end=");
         include-end=True

There are many more methods available to `InputParameters` objects. All of which provide different strategies to lend, or limit, user-control over an object's construction. One of the most basic is `addRequiredParam()`. This can be used to define parameters that *must* be set by a user:

!listing language=C++
params.addRequiredParam<T>("name", "description");

where `T` is the required data type of the input, e.g., `Real`, `std::vector<Real>`, or `std::string`. Another basic method is `addParam()`, which can be used to define optional parameters:

!listing language=C++
params.addParam<T>("name", value, "description");

where `value` is the default value set for the parameter if one is not specified in the input file.

### Accessing Parameter Values

`InputParameters` objects are passed to the constructor methods of `MooseObjects` so that the parameters defined in the `validParams()` method may be referenced from anywhere throughout the object's source, as was demonstrated in `DarcyPressure.C`:

!listing tutorials/tutorial01_app_development/step05_kernel_object/src/kernels/DarcyPressure.C
         link=False
         start=DarcyPressure::DarcyPressure(const InputParameters & parameters)
         end=: ADKernelGrad(parameters),
         include-end=True

The basic method for retrieving a user-defined input is the template `getParam()` method, which returns a constant reference to the desired parameter. This method should be used to initialize a class member variable that references the parameters.

The `getParam()` method can be called from within any member---not just the constructor. Although, this is typically only necessary for special cases and should be avoided as much as possible, since accessing invariant input parameters during each invocation of a function tends to be unnecessarily expensive. In most cases, parameters should be accessed from the constructor to initialize class member variables.

*For more information about Input Parameters, please visit the [source/utils/InputParameters.md] page.*

## Demonstration id=demo

Recall from the [previous step](tutorial01_app_development/step05_kernel_object.md#physics) that, upon applying the [!ac](BVP) and the isotropy assumption, the weak form of Darcy's pressure equation is

!equation id=darcy-weak
(\nabla \psi, \dfrac{K}{\mu} \nabla p) = 0

The `DarcyPressure` object shall be modified to allow users to define the constant properties $K$ and $\mu$.

### Source Code id=source-demo

A required parameter `"permeability"` shall be used to set the value of $K$ and an optional parameter `"viscosity"` whose default value is the dynamic viscosity of water at $30 \degree \textrm{C}$, shall be used to set the value of $\mu$. Assuming that both are valid for any number in $\mathbb{R}$, the data type of these parameters should be `Real`.

In `DarcyPressure.h`, declare `_permeability` and `_viscosity` as reference variables so that their input value may be accessed without having to create additional copies of them:

!listing tutorials/tutorial01_app_development/step06_input_params/include/kernels/DarcyPressure.h
         link=False
         start=/// The reference variables which hold the value for K and mu
         end=};

Do not modify any other parts of `DarcyPressure.h`. Now, in `DarcyPressure.C`, change the `validParams()` definition to the following:

!listing tutorials/tutorial01_app_development/step06_input_params/src/kernels/DarcyPressure.C
         link=False
         start=InputParameters
         end=DarcyPressure::DarcyPressure(const InputParameters & parameters)

The parameters will be retrieved by the constructor method and used to set the values for the `_permeability` and `_viscosity` variables. The user-inputs shall be read with the `getParam()` method. Thus, change the constructor definition to the following:

!listing tutorials/tutorial01_app_development/step06_input_params/src/kernels/DarcyPressure.C
         link=False
         start=DarcyPressure::DarcyPressure(const InputParameters & parameters)
         end=}
         include-end=True

Do not modify any other parts of `DarcyPressure.C`. Now, recompile the application:

!include commands/make.md

### Input File id=input-demo

The properties in [!eqref](darcy-weak) can now be specified in the input file. However, since the default for the `"viscosity"` parameter is the desired value of $\mu_{f} = 7.98 \times 10^{-4} \, \textrm{Pa} \cdot \textrm{s}$, it need not be set. The `[Kernels]` block in `pressure_diffusion.i` should be as follows:

!listing tutorials/tutorial01_app_development/step06_input_params/problems/pressure_diffusion.i
         block=Kernels
         link=False

Now, execute the input file:

!listing language=bash
cd ~/projects/babbler/problems
../babbler-opt -i pressure_diffusion.i

### Results id=result-demo

Run the following commands to visualize the solution with PEACOCK:

!include commands/peacock_r.md
         replace=['<d>', 'problems',
                  '<e>', 'pressure_diffusion_out']

Since no numerical changes were made here, the results should be identical to the [previous step](tutorial01_app_development/step05_kernel_object.md#result-demo). Note that the above command uses the `peacock` alias so you'll need to set the `$PATH` environment variable in your bash profile (see the [python/peacock.md] page). This syntax shall henceforth be used exclusively whenever executing PEACOCK.

### Commit id=commit-demo

Add the changes made to the `DarcyPressure` object files and `pressure_diffusion.i`:

!include commands/git_add.md
         replace=['*', 'include/kernels/DarcyPressure.h src/kernels/DarcyPressure.C problems/pressure_diffusion.i']

Now, commit and push the changes to the remote repository:

!include commands/git_commit.md
         replace=['<m>', '\'defined "permeability" and "viscosity" input parameters\'']

!content pagination previous=tutorial01_app_development/step05_kernel_object.md
                    next=tutorial01_app_development/step07_parallel.md
