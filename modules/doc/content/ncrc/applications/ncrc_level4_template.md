# {{ApplicationName}} Source

Source code for {{ApplicationName}} can be found at [https://github.inl.gov/ncrc/{{binary}}](https://github.inl.gov/ncrc/{{binary}})

## SSH Keys

Create your SSH public/private key and install it on GitHub. Instructions for doing so can be found
on GitHub at: [Add SSH Keys](https://github.inl.gov/settings/keys). If you are unsure how to log
into [https://github.inl.gov](https://github.inl.gov), please peruse through the
[Transitional Guide](https://github.com/idaholab/moose/wiki/NCRC-github.inl.gov-transition-guide).

## Cloning the Repo

Once you have added your SSH key, you can clone the repository in the following ways:

- You do not wish to use a fork:

  ```bash
  git clone git@github.inl.gov:ncrc/{{binary}}.git
  ```

- You created a fork:

  ```bash
  git clone git@github.inl.gov:<your user id>/{{binary}}.git
  ```
