# Input File Syntax
* Creating a set of slides from an existing Markdown file is accomplished using the `FileSet` object.
* Three parameters must be set as shown in the input file below.

```text
[presentation]
  [./file_set_demo]
    type = FileSet
    title = '**FileSet:** Build and modify slides directly from a markdown file'
    file = test.md
  [../]
[]
```


---
name: second

# Markdown Flavor
The [Remark.js](http://remarkjs.com) framework is used to convert the raw markdown into a presentation.

--
* All syntax of [Remark.js](http://remarkjs.com) is supported. For example:
  - using `--` within a slide creates animation
  - properties may be set for the slide, such as using `name: second` with the markdown allows for the
    name to be used as `{{name}}` within the markdown

--
* Refer to the [Remark.js](http://remarkjs.com) site for more details: https://github.com/gnab/remark

---
name: modification
# Input File Slide Modification
It is possible to modify the slide properties via the presentation input file by adding a `[.\Slides]` sub-block.
* The sub-block must be named using the slide number or the name property.
* If using the slide number includes the title and all  `--` and `---` breaks.
* The following slided modifies the this slide to align the text at the right.
* Comments are also able to be added to the slide that are viewable in presentation mode.

---
```text
[presentation]
  [./file_set_demo]
    type = FileSet
    title = '**FileSet:** Build and modify slides directly from a markdown file'
    file = test.md
    [./Slides]
      [./test]
        class = 'right, middle'
        comments = 'Add comments via the input, rather than in the markdown itself'
      [../]
    [../]
  [../]
[]
```

---
name: test
# Modification Test
This slide was modified via the input file to align the text to the middle and right. It also has
additional comments added via the input file, press 'p' to view these comments.

---
name: images
# Images
Images are included with normal markdown syntax, but can also be modified.

![A an image caption](example_01.png width:200px)

---
name: columns
# Columns
.left[
Left
]
.right[
Right
]

---
# GitHub File Extraction

[Kernel.C](https://github.com/idaholab/moose/blob/devel/framework/src/kernels/Kernel.C)

---

[C++ Function Extraction](https://github.com/idaholab/moose/blob/devel/framework/src/kernels/Kernel.C#computeResidual)

---

[C++ Function Prototype Extraction](https://github.com/idaholab/moose/blob/devel/framework/include/kernels/Kernel.h#computeOffDiagJacobian)

[C++ Template Functions](https://github.com/idaholab/moose/blob/devel/framework/include/materials/Material.h#Material::getMaterialProperty)

[C++ Template Function Prototypes](https://github.com/idaholab/moose/blob/devel/framework/include/materials/Material.h#getMaterialProperty)

---

[Input File Blocks](https://github.com/idaholab/moose/blob/devel/test/tests/adaptivity/initial_adapt/initial_adapt.i#Kernels)

---

[Input File Sub-Blocks](https://github.com/idaholab/moose/blob/devel/test/tests/adaptivity/initial_marker/initial_marker.i#Markers)
