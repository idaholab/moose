## Update MOOSE

MOOSE does not use traditional versioning, is under heavy development, and is being updated
continuously. Therefore, it is important that you continue to update MOOSE as you use it to develop your
application(s), we recommend weekly updates.

To update MOOSE use the following commands.

```bash
cd ~/projects/moose
git fetch origin
git rebase origin/master
```

Then return to your application, re-compile, and test.

```bash
cd ~/projects/YourAppName
make -j4
./run_tests -j4
```
