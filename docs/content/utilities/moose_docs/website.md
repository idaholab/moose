# Creating a Web-site

The primary motivation behind the [MOOSE documentation system](utilities/moose_docs/index.md) was
to create a customizable, maintainable web-site for the MOOSE framework and modules. However, it was
important to the the MOOSE developers that this system be available to application developers as
well to enable MOOSE-based applications to be well-documented using a common syntax.

The first step for creating a web-site is to document your code, which is detailed here:
[Documenting Code](utilities/moose_docs/code.md). Once this is complete then you should continue with the steps listed below.

!include docs/content/utilities/moose_docs/config.md

For example the file shown below is the website configuration file from [MOOSE]. The most important
portion of this file is the "MooseDocs.extensions.template" entry. This provides the template for
the resulting html, which in this case is setup for website display.

!listing docs/website.yml

## Creating Web-Site Content
Adding content to the web-site is as simple as creating a markdown file with content. Generally,  the markdown files are located in the "docs/content" directory of your application.

## Viewing and Deploying a Web-site
To view or build a website the "build" sub-command of the `moosedocs.py` executable is utilized.
A complete list of options for this command run with the "-h" flag.
```text
./moosedocs build -h
```

When you want to view your documentation website locally, you must serve the site to a local
server. This is done as:
```text
./moosedocs.py build --serve
```

This will generate the website content and serve it on a local server, by default 127.0.0.1:8000,
which can be viewed in a web-browser. Most importantly this sever is live, if you edit a markdown
file it will automatically be re-generated.

To deploy the website, run the script without the "--serve" option, which will create the html and
copy all needed files to the location provided by the "--site-dir" option. The contents can then be
copied to an external server for hosting.

```text
./moosedocs.py build
```

## Including App Documentation
MooseDocs allows for the inclusion of documentation from other applications. This includes the
ability to include documentation from the framework and/or modules in your own application.
To include other sources of markdown, you must define a "contents" YAML file
(see `./moosedocs.py build -h`). When building, a list of files is gathered based on the content
of this file, and a complete website file tree is created from the combined content. This file
tree can be viewed by running the "build" command with the "--dump" option, which will show the
tree but not actually perform a build.

The [MOOSE] test application ("moose_test") includes an example file, as shown in
\ref{moose_test_content}. At the top level are arbitrary labels (e.g., "framework") that contain
information about what markdown files to include or exclude. Beneath each label the following
items can be defined to describe the markdown files that should be included.

* **base**<br>
The "base" should be a path, relative to the repository root, that will serve as the base directory
when the content is copied to the website file tree.

* **include**<br>
The "include" keyword should include a list of filenames or glob patterns that should be
included in the markdown file tree.

* **exclude**<br>
The "exclude" keyword should be a list of filenames or glob patterns that should be
removed from the already created list of files from the "include" option.

!listing test/docs/content.yml id=moose_test_content caption=Example YAML file used to add framework content to an applications documentation.
