# {{ApplicationName}} Source

!alert tip title=Existing {{ApplicationName}} HPC GitLab Users
For existing users who access code via [hpcgitlab.hpc.inl.gov](https://hpcgitlab.hpc.inl.gov),
please see the [Transitional Guide](https://github.com/idaholab/moose/wiki/NCRC-github.inl.gov-transition-guide)
for help on accessing the new home for [NCRC/{{ApplicationName}}](https://github.inl.gov/ncrc/{{binary}}). New users can safely ignore this notice.


Source code for {{ApplicationName}} can be found at [https://github.inl.gov/ncrc/{{binary}}](https://github.inl.gov/ncrc/{{binary}})

While logged in to [ncrc/{{binary}}](https://github.inl.gov/ncrc/{{binary}}) you should at
this time create a fork of the repository. This is only required once, and is accomplished by
clicking 'Fork' at the top right. You will be presented with a new page asking how you would like
{{ApplicationName}} forked:

| Owner* | Repository name* |
| :- | :- |
| ^Select an owner | {{binary}} |

Click 'Select an owner' and choose yourself. Take note of how GitHub identifies you
(`first-last` name). Then click 'Create fork' near the bottom. GitHub will begin creating a
personalized fork of {{ApplicationName}}. When finished, GitHub will bring you to your fork of
{{ApplicationName}}.

# SSH Keys

Before you can clone your fork, you must create your SSH public/private key. Detailed instructions
for doing so can be found on GitHub at:
[Generating a new SSH key](https://docs.github.com/en/enterprise-server@3.6/authentication/connecting-to-github-with-ssh/generating-a-new-ssh-key-and-adding-it-to-the-ssh-agent#generating-a-new-ssh-key).

The gist of creating a key is as follows:

- Open a terminal
- Enter the following command

  ```bash
  ssh-keygen -t ed25519
  ```
- Hit Enter for every question, thus creating a passphrase-less key

Entering a passphrase (or a blank response) is your preference. You'll need to enter this passphrase
each time you perform a push/pull operation with your fork.


With your key created, and the terminal window still open, copy the contents of your public key:

```bash
cat ~/.ssh/id_ed25519.pub
```

!alert tip
Only copy the results of the command. Do not include the command, or the following prompt.

Head on over to [GitHub.inl.gov](https://github.inl.gov/settings/keys) and click 'New SSH key'.
Enter any title you wish. Most enter something that identifies the computer which generated the key
(e.g. workstation) or something similar.

Then paste the contents you earlier copied into the 'key' field.

# Cloning the Repo

Once you have added your SSH key, you should be able to clone your fork. At this time, choose a
location you wish to operate from. Such as `~/projects`, and clone {{ApplicationName}}:

```bash
mkdir -p ~/projects
cd ~/projects
git clone git@github.inl.gov:first_name-last_name/{{binary}}.git
```
