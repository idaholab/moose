  [libMesh](https://github.com/libMesh/libmesh) is an open source finite element library that MOOSE heavily relies on.  Because of this tight dependency, a version of libMesh that has been vetted by the MOOSE team is distributed directly with MOOSE.  From time to time the MOOSE team will send notices out (using the moose-users mailing list) telling you that libMesh has been changed you need to rebuild it as detailed below.

## First, Update MOOSE

To update your clone of MOOSE, first make sure you know the name of the remote that points to the official version of MOOSE. If you are following our developer instructions it's likely `upstream`, if you are following the user instructions, it may be named `origin`. You can always run `git remote -v` to see the names of your remote and where they point. Look for the version that contains `idaholab/moose`. Next make sure you know the name of the branch you are tracking. If you are following the stable version of MOOSE it'll be `master`, otherwise it may be `devel`:

 * Change directories into the root of your repository
 * Run `git fetch <remote>`
 * Run `git rebase <remote>/<branch>`

## Update libMesh

After you have updated MOOSE go into the "moose" directory (if you're not there already) and run:

```bash
scripts/update_and_rebuild_libmesh.sh
```

You will then need to rebuild your application (or module) by going into the appropriate directory and running `make -j <jobs>`

## Dealing with an update failure when using the GitHub repo
If your connection gets interrupted or fails in any way, you might receive errors when running the script a subsequent time.  If this occurs, simply remove the libmesh directory an run the script once more.  DO NOT delete your libmesh directory if you are using the internal SVN repository.

```
rm -rf libmesh
scripts/update_and_rebuild_libmesh.sh
```

## libMesh Documentation

The libMesh documentation can be found [here](http://libmesh.sourceforge.net/doxygen/classes.php)

## See also

- [FParser Automatic Differentiation](FParserAutomaticDifferentiation)
- [FParser Just In Time Compilation](FParserJustInTimeCompilation)
Getting Started
Blog
Wiki
GitHub
About
Contact

