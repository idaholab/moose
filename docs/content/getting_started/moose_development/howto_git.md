For the most part if you need help with git, Google is your best resource.  This page is intended for tips that we find useful for working with the MOOSE framework.

 - "Downloading" a single Pull Request
```
git fetch upstream pull/<#>/head:<new_br_name>
git checkout <new_br_name>
```

- A Basic .gitconfig file (put this in your home directory)

```
[user]
        name = <Your Name>
        email = <Your Email Address>

[color]
        diff = auto
        status = auto
        branch = auto
        interactive = auto
        ui = true
        pager = true

[alias]
        co = checkout
        di = diff
        st = status
        ci = commit
        stat = status
        br = branch

[push]
        default = current
```

- Listing most recent branches

Once you have a lot of branches going, it can sometimes be hard to keep track of which one is which, and which ones you worked in most recently. This bash alias sets up the command "brls" (for branch ls) that will list all your branches in order of most recently accessed. Place in your .bashrc_local file (or whatever init file you are using).

```bash
alias brls="git for-each-ref --count=30 --sort=-committerdate refs/heads/ --format='%(refname:short)'"
```

- Using bash git completion and special prompts

To enable bash completion for git (so you can tab-complete 'git co' or 'git br' commands) download [git_completion.sh](http://mooseframework.org/static/media/uploads/files/git_completion.sh) to your home directory, and append the following line to the end of your ~/.bash_profile (or ~/.bashrc)
```bash
source ~/git_completion.sh
```

If you would like to have the name of the current branch in your prompt, download [git_prompt.sh](http://mooseframework.org/static/media/uploads/files/git_prompt.sh) to your home directory, and append the following lines to the end of your ~/.bash_profile (or ~/.bashrc)
```bash
source ~/git_prompt.sh
export PS1='$(__git_ps1 "(%s)")$ '
```
Assuming your prompt is just '$ ', adding this line to your .bash_profile will cause it to look like '(master)$ ' whenever you cd into a directory containing a git repo in which the master branch is checked out.

