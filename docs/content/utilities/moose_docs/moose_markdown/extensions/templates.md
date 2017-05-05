# TemplateExtension

The TemplateExtension provides the capability of applying the html from converted markdown to an
arbitrary [jinja2](http://jinja.pocoo.org/) template. This extension is utilized to create this
website by warping the relatively basic html provided by the markdown into a template, using
[materialize](http://materializecss.com/).

The configuration options for this extension are included in \ref{TemplateExtension}. The most critical of the provided options is the "template" options. This specifies the the template to
utilize. If creating a new template the file must reside in the "templates" directory next to the
"moosedocs.py" executable.

!extension TemplateExtension

This extension also enables automatic linking based on markdown filenames,
which is especially useful when linking to generated pages. The syntax is identical to creating
links with traditional markdown; however, the markdown file is provided as the source rather
than the html.  Additionally, the source path does not need to be complete, but must be unique.

* `[/Diffusion.md]`: [/Diffusion.md]
* `[/Kernels/index..md]`: [systems/Kernels/index.md]
* `[Diffusion](/Diffusion.md)`: [Diffusion](/Diffusion.md)
