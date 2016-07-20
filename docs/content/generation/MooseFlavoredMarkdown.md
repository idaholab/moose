<!-- content/generation/MooseFlavoredMarkdown.md -->

test

# Moose Flavored Markdown

## Automatic Links

Moose Flavored Markdown is capable of automtically creating links based on Markdown filenames, which is
especially useful when linking to generated pages.

* `[auto::/Kernels/Diffusion.md]`: [auto::/Kernels/Diffusion.md]
* `[auto::framework/Kernels/Overview.md]`: [auto::framework/Kernels/Overview.md]
* `[auto::Testing](/Kernels/Diffusion.md)`: [Testing](auto::/Kernels/Diffusion.md)

## Including MOOSE Source Files
It is possible to include complete or partial C++ or input files from the local MOOSE repository. The following sections detail the custom
markdown syntax to needed, including the application of special settings in the form of key, value pairings that are supplied within
the custom markdown. A complete list of available settings is provided in the [Settings](MooseFlavoredMarkdown.md#optional-settings) of the included code.

### Complete Files
You can include complete files from the repository using the `![]()` syntax similar to that used in images. For example, the following
includes the complete code as shown.

```markdown
![Diffusion.C](framework/src/kernels/Diffusion.C max-height=200px strip-extra-newlines=True)
```

![Diffusion.C](framework/src/kernels/Diffusion.C max-height=200px strip-extra-newlines=True)

### Single Line Match
It is possible to show a single line of a file by a snippet that allows the line to be located within
the file. If multiple matches occur only the first match will be shown. For example, the call to
`addClassDescription` can be shown by adding the following.

```markdown
![](framework/src/kernels/Diffusion.C line=addClassDescription)
```

![](framework/src/kernels/Diffusion.C line=addClassDescription)

### Range Line match
Code starting and ending on lines containing a string is also possible by using the 'start' and 'end'
options. If 'start' is omitted then the snippet will start at the beginning of the file. Similarly, if 'end'
is omiited the snippet will include the remainder of the file content.

```markdown
![](test/tests/kernels/simple_diffusion/simple_diffusion.i start=Kernels end=Executioner)
```

![](test/tests/kernels/simple_diffusion/simple_diffusion.i start=Kernels end=Executioner)

### Class Methods
By including a method name, in C++ syntax fashion, it is possible to include specific methods from C++ classes in MOOSE. For example,
the following limits the included code to the `computeQpResidual` method.

```markdown
![Diffusion.C::computeQpResidual](framework/src/kernels/Diffusion.C::computeQpResidual)
```

<!--
![Diffusion.C::computeQpResidual](framework/src/kernels/Diffusion.C::computeQpResidual)
-->

This method uses the clang parser directly, which can be slow. Thus, in general source code should be
included using the line and range match methods above and this method reserved for cases where those methods
fail to capture the necessary code.


### Input File Block
By including a block name the included content will be limited to the content matching the supplied name. Notice that the supplied name may be approximate; however, if it is not unique only the first match will appear.

```markdown
![simple_diffusion.i](test/tests/kernels/simple_diffusion/simple_diffusion.i::Kernels)
```

![simple_diffusion.i](test/tests/kernels/simple_diffusion/simple_diffusion.i::Kernels repo_link=True)


### Optional Settings
The following options may be passed to control how the output is formatted.


| Option               | Default | Description |
| -------------------- | ------- | ----------- |
| strip_header         | True    | Toggles the removal of the MOOSE copyright header. |
| repo_link            | True    | Include a link to the source code on GitHub ("label" must be True). |
| label                | True    | Include a label with the filename before the code content block. |
| overflow-y           | Scroll  | The action to take when the text overflow the html container (see [overflow-y](http://www.w3schools.com/cssref/css3_pr_overflow-y.asp)). |
| max-hieght           | 500px   | The maximum height of the code window (see [max-height](http://www.w3schools.com/cssref/pr_dim_max-height.asp)). |
| strip-extra-newlines | False   | Remove exessive newlines from the included code. |

## Slideshows
