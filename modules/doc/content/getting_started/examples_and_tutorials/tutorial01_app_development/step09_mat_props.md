!content pagination previous=tutorial01_app_development/step08_test_harness.md
                    next=tutorial01_app_development/step10_auxkernels.md
                    margin-bottom=0px

# Step 9: Develop a Material Object

In this step, the basic components of the `Material` class will be presented. To demonstrate its use, a new `MooseObject` that provides the values for permeability and viscosity needed by the `DarcyPressure` class will be developed. A test for this new object shall be written in accordance with the procedures outlined in the [previous step](getting_started/examples_and_tutorials/tutorial01_app_development/step08_test_harness.md). In addition, any input files which create `DarcyPressure` objects will be need to be modified.

## Material Objects

The [syntax/Materials/index.md] in MOOSE is one for defining spatially and/or temporally varying properties and storing their values in a single object. The types of terms that belong to this system are the coefficients in the governing equations, though they may or may not be continuously differentiable with respect to the problem domain. The values are always indexed at individual [!ac](QPs) and the associated class methods tend to be invoked at every time step. The properties may then be accessed by other MOOSE systems. And, like many systems in MOOSE, Materials can be coupled to variables if a property being calculated depends on them.

[`Material`](framework/src/materials/MaterialBase.C) objects are developed in C++ similar to how those in other MOOSE systems are, such as Kernels. Like `Kernel` objects, they inherit a default set of input parameters and [virtual methods](https://www.geeksforgeeks.org/virtual-function-cpp/) from the base class. Thus, by recalling some of the preceding steps of this tutorial, the reader may recognize the format and procedure for developing a new `MooseObject` as it is presented in the [#demo] section.

### Producing and Consuming Properties id=prod-cons

The Materials System relies on a producer-consumer relationship among objects:

- `Material` (and `ADMaterial`) objects +produce+ properties.
- Other MOOSE objects (including materials themselves) +consume+ these properties.

The life span of a material property will proceed as follows:

1. Each property must be declared to be available for use. The `declareADProperty<T>("name")` method does this and returns a  writable reference, where `T` may be any valid C++ type, such as `Real` or `std::vector<Real>`.

1. The values to be associated with `"name"` are computed and set from within one of the available methods---typically, `computeQpProperties()`.

1. Other objects may then call `getADMaterialProperty<T>("name")` to retrieve the property referred to by `"name"`.

### Property Output id=out-props

Every `Material` object will have input parameters to control how properties are saved to output files. A property can be designated for output by providing its name to the `"output_properties"` parameter in the input file. The values for these properties are written to one or more `Output` object(s), such as an ExodusII file, by providing their names to the `"outputs"` parameter. These parameters will be made especially useful when developing a regression test as part of the demonstration for this step.

!alert warning title=Property output parameters
The `"outputs"` parameter defaults to `none` and, therefore, must be explicitly set in order to write the values requested by `"output_properties"` to any output files. Also, while the properties produced by `Material` objects can be any valid C++ type, not all types are supported when outputting. Please see the [syntax/Materials/index.md#material-property-output] discussion for more information.

## Demonstration id=demo

Now that the basic purpose of the Materials System has been established, another object in the isotropic, divergence-free weak form of Darcy's equation can be identified, i.e.,

!equation id=darcy-weak
\underbrace{(\nabla \psi, \underbrace{\dfrac{K}{\mu}}_{\clap{Material}} \nabla p)}_{\clap{Kernel}} = 0

For this demonstration, a new `Material` object will be developed to produce the $K / \mu$ term in [!eqref](darcy-weak) and the `DarcyPressure` class will be modified to consume it. The new `MooseObject` shall accept an input for the diameter $d$ of steel spheres tightly packed to form a porous medium. The [tutorial01_app_development/problem_statement.md#mats] section of the Problem Statement provided the following linear relationship between the permeability ($\text{m}^{2}$) of such media and the sphere diameter ($\text{mm}$):

!equation id=permeability
K(d) = \frac{1}{2} \begin{bmatrix} -d + 3 & d - 1 \end{bmatrix} \begin{Bmatrix} 0.8451 \\ 8.968 \end{Bmatrix} \times 10^{-9}, \enspace \forall \, d \in [1, 3]

The new object shall restrict the input diameter to the domain of [!eqref](permeability) and enforce the requirement that $\mu \; {=}\mathllap{\small{/}\,} \; 0$.

Since the pressure vessel model uses $d = d_{s} = 1 \, \text{mm}$, `pressure_diffusion.i` shall be modified to use this value as input. It will be shown that this input reproduces the isotropic permeability value $K = 0.8451 \times 10^{-9} \, \textrm{m}^{2}$, introduced in [Step 5](tutorial01_app_development/step05_kernel_object.md#demo). Thus, the results of the `darcy_pressure_test.i` file created in the [previous step](tutorial01_app_development/step08_test_harness.md#test-demo) should still match the gold file after modifying its inputs.

### Source Code id=source-demo

To produce the material property term in [!eqref](darcy-weak), a new `ADMaterial` object can be created and it shall be called `PackedColumn`: a name intended to depict a coarse filter medium in a slender tube. Start by making the directories to store files for objects that are part of the Materials System:

!include commands/mkdir.md
         replace=['<d>', 'include/materials src/materials']

In `include/materials`, create a file name `PackedColumn.h` and add the code given in [pc-header]. Here, the header file for the base class was included so that it can be inherited. In addition, [`LinearInterpolation.h`](framework/include/utils/LinearInterpolation.h) was included so that an object of the `LinearInterpolation` class---a member of the [framework_development/utils/index.md] System in MOOSE---can be made to evaluate [!eqref](permeability). The `validParams()` and constructor methods were the first members declared, as is typical, and the `computeQpProperties()` method from the base class was overridden. Two variables, `_diameter` and `_input_viscosity`, were declared to store the values input for $d$ and $\mu$, respectively. The former of these two variables will need to be passed to a `LinearInterpolation` object, in order to obtain $K$, so `_permeability_interpolation` was declared for this purpose. Finally, two `ADMaterialProperty` variables were declared and will, ultimately, become available to other MOOSE objects that need them.

!listing tutorials/tutorial01_app_development/step09_mat_props/include/materials/PackedColumn.h
         link=False
         id=pc-header
         caption=Header file for the `PackedColumn` class.

In `src/kernels`, create a file named `PackedColumn.C` and add the code given in [pc-source]. To enforce the necessary restrictions on values for $d$ and $\mu$, their inputs are parsed with the [`addRangeCheckedParam()` method](source/utils/InputParameters.md#range-checked-parameters optional=True). This method is like `addParam()`, i.e., it sets a default value in lieu of user input, for which `diameter = 1` ($d_{s}$) and `viscosity = 7.98e-04` ($\mu_{f}$) were used here. However, it has an additional argument that accepts logical expressions, which operate on the parameter itself. If the expression is false for a given input, an error message is invoked and the application terminates. This method provides more convenient means for enforcing $\mu \; {=}\mathllap{\small{/}\,} \; 0$, or $1 \le d \le 3$ for that matter, than hard-coding a condition that leads to a `paramError()`---the approach followed in the previous step, but not all types of parameters are able to be validated in this way.

In the constructor definition, two `Real` type properties by the names `"permeability"` and `"viscosity"` were declared available for consumption using the `declareADProperty()` method that was mentioned in the [#prod-cons] section. Two vector variables, `sphere_sizes` and `permeability`, were declared and set to the domain and range of [!eqref](permeability) in an abscissa-ordinate pair fashion. These vectors are then passed to the `setData()` method of the `LinearInterpolation` object, which is a wise task to handle in the constructor, since there's no need to repeatedly send invariant copies of the data at each solve step and at each [!ac](QP) index. Finally, in the `computeQpProperties()` definition, the references to the properties are set, for which MOOSE and [libMesh] work together to resolve the `_qp` indexing scheme. Here, the `_diameter` variable was passed to the `sample()` method to retrieve the permeability.

!listing tutorials/tutorial01_app_development/step09_mat_props/src/materials/PackedColumn.C
         link=False
         id=pc-source
         caption=Source file for the `PackedColumn` class.

The `DarcyPressure` class needs to be modified to consume the `"permeability"` and `"viscosity"` properties at each [!ac](QP) index. Consequently, it also no longer needs input for the corresponding values. In `DarcyPressure.h`, change the delcaration of the `_permeability` and `_viscosity` variables to the following:

!listing tutorials/tutorial01_app_development/step09_mat_props/include/kernels/DarcyPressure.h
         link=False
         include-start=False
         start=virtual ADRealVectorValue
         end=};

Do not modify any other parts of `DarcyPressure.h`.

All definitions in the source file need to be modified, so the reader may simply overwrite it with the syntax given in [darcy-source]. Still, they should take a moment to review the changes made by running `git diff *DarcyPressure.C`.

!listing tutorials/tutorial01_app_development/step09_mat_props/src/kernels/DarcyPressure.C
         link=False
         id=darcy-source
         caption=Source file for the `DarcyPressure` class modified to consume material properties produced by a `PackedColumn` object.

Be sure to recompile the application before proceeding:

!include commands/make.md

### Input File id=input-demo

An input file specialized to test the `PackedColumn` class is in order. Start by creating a directory to store the test files:

!include commands/mkdir.md
         replace=['<d>', 'test/tests/materials/packed_column']

In this folder, create a file named `packed_column_test.i` and add the inputs given in [pc-test]. In the `[filter]` block, a `PackedColumn` object is created with a `"viscosity"` input that is trivial for testing purposes as its value goes directly to the property reference. However, for the `"diameter"` input, the median value of the domain ($d = 2$) was selected because this obviously corresponds to the median value of the range ($K = 4.907 \times 10^{-9}$) and, therefore, can be verified in such terms. Outputs were requested for both properties by means which were discussed in the [#out-props] section, where the format will be an ExodusII file so that an `Exodiff` tester may reference it.

!listing tutorials/tutorial01_app_development/step09_mat_props/test/tests/materials/packed_column/packed_column_test.i
         link=False
         id=pc-test
         caption=Input file to test the `PackedColumn` class with an `Exodiff` object.

Execute the input file in [pc-test], confirm that the solver completes, and that a file named `packed_column_test_out.e` is generated:

!listing language=bash
cd ~/projects/babbler/test/tests/materials/packed_column
../../../../babbler-opt -i packed_column_test.i

The inputs in `problems/pressure_diffusion.i` need to be modified to use a `PackedColumn` object. In this file, update the `[Kernels]` block and add a `[Materials]` one, e.g.,

!listing tutorials/tutorial01_app_development/step09_mat_props/problems/pressure_diffusion.i
         link=False
         block=Kernels Materials

Since this model uses the default input values, there was no need to specify them explicitly.

!alert note icon-name=create prefix=False title=TASK:$\;\;$Update the `darcy_pressure_test.i` file.
A similar modification as the one just made to `pressure_diffusion.i` must also be made to `darcy_pressure_test.i`. Otherwise, the associated test will fail. It is left up to the reader to implement the correct changes to this file.

!!!
I might start leaving certain tasks up to the reader from here on. This one is easy, but I can make them increasingly challenging over the remaining steps. There's no need to feel guilty, since they will essentially be provided with an answer key in the form of a repo, eventually.

Perhaps we could make a new alert brand: !alert exercise, !alert task, e.g., but that'd probably be overkill
!!!

### Results id=result-demo

Use PEACOCK to query the `"permeability"` and `"viscosity"` outputs from `packed_column_test.i` on each element:

!include commands/peacock_r.md
         replace=['<d>', 'test/tests/materials/packed_column',
                  '<e>', 'packed_column_test_out']

In the ExodusViewer tab, the property values can be resolved by referring to the color bar (enabling the "View Mesh" checkbox might help with interpreting the image). Verify that the values match the expected ones and note that, since neither property was made spatially dependent, the `PackedColumn` object assigned the same number to all [!ac](QPs) and the contours render as a solid color. These results are illustrated in [results].

!media tutorial01_app_development/step09_result.png
       style=width:68%;margin-left:auto;margin-right:auto;
       id=results
       caption=Rendering of the `"permeability"` and `"viscosity"` material properties produced by the `PackedColumn` test. These properties are spatially invariant and so their values are uniformly distributed throughout the mesh.

### Test id=test-demo

Since the results of the `packed_column_test.i` input file have been deemed good, the ExodusII output can now become a certified gold file:

!include commands/new_gold.md
         replace=['<d>', 'materials/packed_column',
                  '<e>', 'packed_colum_test_out']

Next, create a file named `tests` in `test/tests/materials/packed_column` and add the inputs given in [pc-test-spec].

!listing tutorials/tutorial01_app_development/step09_mat_props/test/tests/materials/packed_column/tests
         link=False
         remove=Tests/issues Tests/design Tests/test/requirement
         id=pc-test-spec
         caption=Test specification file for the `PackedColumn` class.

The `RunException` tester that was specified in the previous step and was used to test proper invocation of the `paramError()`, when the input is `viscosity = 0`, is no longer useful. In fact, this test will fail and may simply be removed. In the `test/tests/kernels/darcy_pressure/tests` file, remove the `[zero_viscosity_error]` block, but leave the `[test]` block unchanged. Also, delete `zero_viscosity_error.i`.

!alert tip title=Limit the scope of tests to the application itself.
One might wonder if it would be beneficial to test that an input which does not satisfy a logical expression passed to the `addRangeCheckedParam()` method leads to an error. This surely wouldn't hurt. Just know that almost all developer tools available from MOOSE already have their own testing systems.

Be sure that the `darcy_pressure_test.i` file has been updated in the manner demonstrated in the [#input-demo] section. Finally, run `TestHarness`:

!include commands/run_tests.md

If the tests passed, the terminal output should look something like that shown below.

```
test:materials/packed_column.test ......................................................................... OK
test:kernels/darcy_pressure.test .......................................................................... OK
test:kernels/simple_diffusion.test ........................................................................ OK
--------------------------------------------------------------------------------------------------------------
Ran 3 tests in 0.4 seconds. Average test time 0.1 seconds, maximum test time 0.1 seconds.
3 passed, 0 skipped, 0 pending, 0 failed
```

### Commit id=commit-demo

There are many changes to add to the git tracker here, but a wildcard can help simplify the command:

!include commands/git_add.md

Disregard the warning about `.gitignore` files. Now, commit and push the changes to the remote repository:

!include commands/git_commit.md
         replace=['<m>', '"developed material to compute properties of fluid flow through packed steel sphere medium and modified Darcy kernel"']

!content pagination previous=tutorial01_app_development/step08_test_harness.md
                    next=tutorial01_app_development/step10_auxkernels.md
