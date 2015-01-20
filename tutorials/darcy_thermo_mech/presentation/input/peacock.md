# Peacock: The MOOSE GUI

 - Peacock is a graphical front-end for any MOOSE-based application
 - You will first want to put `moose/gui` into your `PATH` by adding the following to your `.bash_profile`

```bash
export PATH=$PATH:~/projects/moose/gui
```
 - Peacock automatically locates a MOOSE-based application by looking "up" from the directory it's started in
 - You can pass an input file on the command-line:

```bash
peacock something.i
```
 - It can also be used to open existing output files:

```bash
peacock result_out.e
```

 - Peacock automatically updates with any new syntax as you develop an application.  No need to restart it!
---

![](input/peacock.png)


[(www.mooseframework.org/wiki/Peacock)](http://www.mooseframework.com/wiki/Peacock)
