# Creating Presentations

The documentation system for MOOSE includes the ability to create html based presentations from
[MOOSE flavored markdown](moose_flavored_markdown.md). The presentations are generated using the [reveal.js](http://lab.hakim.se/reveal-js/) framework.

## Creating Content

Creating a presentation begins by creating a markdown file with the desired content. For example, the file `~/projects/moose/docs/examples/presentation.md` contained in MOOSE is a simple presentation to demonstrate the capability and syntax.

## Building the Presentation

First, a copy of [reveal.js](http://lab.hakim.se/reveal-js/) must be installed.

```bash
cd ~/projects
git clone https://github.com/hakimel/reveal.js.git
```

The `~/projects/moose/docs/moosedocs.py python utility is used for converting the markdown
that was created into a presentation.

```bash
cd ~/projects/moose/docs
./moosedocs presentation examples/presentation.md
```

This will create an index.html file in your reveal.js checkout that contains the presentation. Run `./moosedocs presentation --help` for additional options when running this
script.

To view the slides, simply open the index.html file created. On OSX you can preform the following.

```bash
open ~/projects/reveal.js/index.html
```
