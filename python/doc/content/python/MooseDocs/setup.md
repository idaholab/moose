# MooseDocs Setup

Any MOOSE-based application can use the MooseDocs system to create custom websites. If MOOSE is
installed then the necessary dependencies exist. Also, if you have recently created an application
with stork (after Aug. 15, 2018) then your application will have the files for a simple website
created. The sections below will aid in setting up your application if you do not have the
dependencies or the necessary documentation files.

To build a live website for your application, run the following:

```bash
cd ~/projects/your_animal/doc
./moosedocs.py build --serve
```

When this command completes a message will be printed and the site will be hosted at
[http://127.0.0.1:8000](http://127.0.0.1:8000). Note, when new pages are added the build command will
need to be re-executed.

This executable contains command-line based help, which can be accessed using the "-h" flag as
follows.

```
cd ~/projects/your_animal/docs
./moosedocs.py -h
```

The configuration file contains information on how to build the website, additional details regarding
this file may be found at [MooseDocs/config.md].

Once you have a basic website running the next step is to document your code, it is best to refer to
the MOOSE instructions for documentation (see [framework/documenting.md]). In general, applications
mimic the MOOSE process.

## Dependencies

If you are not using a MOOSE package or your package is old, then the following packages must be
installed, which can be done using [pip](https://pip.pypa.io/en/stable/).

```bash
pip install --user pybtex livereload==2.5.1 pylatexenc anytree pandas
```

## Creating Documentation Files

If you have an existing application without a "doc" directory, then a few files must be created
before you can begin building a website.

First create a doc directory that contains the homepage for your application.

```bash
cd ~/project/your_animal
mkdir doc
mkdir doc/content
echo "# YourAnimalApp" > doc/content/index.md
```

Second, create the MooseDocs executable and configuration file. This is done by copying the files
used by the stork script.

```
cp ~/projects/moose/stork/doc/moosedocs.py ~/projects/your_animal/doc
cp ~/projects/moose/stork/doc/config.yml ~/projects/your_animal/doc
```
