# Documentation Setup and Configuration

MOOSE contains a single-source documentation system for creating websites, slideshows, and pdfs
using [markdown](https://en.wikipedia.org/wiki/Markdown) syntax, including a custom syntax aimed to simply the
process: [Moose Flavored Markdown](moose_markdown/index.md).

---

## Dependencies
If you are using a current MOOSE package then a majority of the setup is complete, only the large file storage system needs
to be installed, which can be done by running the following command:

```bash
git lfs install
```

If you are working on documentation in MOOSE or an application that has existing documentation then
you will need to run the following command from within the repository to download any existing images.

```bash
git lfs pull
```


### Manual Setup
If you are not using a MOOSE package, then the following steps may be performed to install the necessary dependencies
for using the documentation system.

(1) Install Git LFS
Install the latest Git Large File Storage package for your system following the getting started instruction from [git-lfs.github.com/](https://git-lfs.github.com/). If you are working on documentation in MOOSE or an application that has existing documentation then
you will need to run the following command from within the repository to download any existing images.

```bash
git lfs pull
```

(1) Create a new python environment.

This will create a special python environment suitable for using the documentation system, this is done so that an existing
python configuration you have will not be damaged.

```bash
conda create --name docs --clone root
```

(2) Activate the new environment.

This command will need to be executed within any terminal that you plan to perform documentation related tasks.

```bash
source activate docs
```

(3) Install the additional python packages.

```bash
pip install python-markdown-math bs4 pybtex jinja2 livereload lxml pylatexenc
```

## Configuration
If you are adding documentation to an existing project then you will need to add a few files to configure the
documentation system to work properly for your application.

First, you need the main executable ("moosedocs.py") within your application. This is done by copying the file from MOOSE:

```bash
cp ~/projects/moose/docs/moosedocs.py ~/projects/your_application_name/doc
```

At this point you are ready to begin creating a configuration file for one of the various forms
of documentation that MooseDocs is capable of producing:

* [Creating a Web-Site](moose_docs/website.md)
* [Building a Presentation](moose_docs/presentation.md)
* [Generating a PDF](moose_docs/pdf.md)
