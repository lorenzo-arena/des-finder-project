## DES Finder project

This project was made for the "parallel computing" course at the Computer Engineering University of Florence.

### Goal

The main goal is to write two version of the same program which must find a password from a dictionary, given an hash and a salt.

The algorithm used for hashing passwords is one of the first used for storing password in Unix systems, [crypt(3)](https://www.man7.org/linux/man-pages/man3/crypt.3.html).

### Requirements

This project can be built using [meson](https://mesonbuild.com/) and ninja.

To execute a build you need to navigate to the folder "sequential" or "parallel" and use the following commands:

```bash
$ meson builddir
$ cd builddir
$ ninja
```

