## Update MOOSE and Conda

MOOSE does not use traditional versioning, is under heavy development, and is being updated
continuously. Therefore, it is important that you continue to update MOOSE as you use it to develop your
application(s), we recommend weekly updates.

If you are using our Conda environment, you should always perform an update to both Conda +and+ your MOOSE repository. If you update one, always update the other:

```bash
conda activate moose
conda update --all
```

To update your MOOSE repository use the following commands.

```bash
cd ~/projects/moose
git fetch origin
git rebase origin/master
```

Then return to your application, re-compile, and test.

```bash
cd ~/projects/YourAppName
make clobberall
make -j4
./run_tests -j4
```
