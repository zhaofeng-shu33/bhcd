# Bayesian Hierarchical Community Detection
[![Build Status](https://travis-ci.com/zhaofeng-shu33/bhcd.svg?branch=master)](https://travis-ci.com/zhaofeng-shu33/bhcd)
[![Windows](https://ci.appveyor.com/api/projects/status/github/zhaofeng-shu33/bhcd?branch=master&svg=true)](https://ci.appveyor.com/project/zhaofeng-shu33/bhcd)

Origin

This repository comes from [bhcd](https://github.com/blundellc/bhcd). The main difference is that this repository uses CMake build system while the original one uses `autoconfig`. The build instructions has some problems even with Unix system. We use CMake to make the algorithm cross-platform, supporting Windows and Unix.

## How to build

You need to install glib and gsl as dependency.
Then using standard cmake way to build the project for your operating system.


## How to run

### Run the executable program
```sh
$ cat >tiny.gml <<EOF
graph [
    sparse 0
    node [ id 0 label "a" ]
    node [ id 1 label "b" ]
    node [ id 2 label "c" ]
    edge [ source 0 target 1 weight 1 ]
    edge [ source 1 target 0 weight 1 ]
]
EOF
$ ./src/bhcd/bhcd -S -p tiny tiny.gml
seed: 2a23b6bb
output prefix: tiny
time: 3.010000e-04s
tree: logprob: -1.92e+00 (0:4,1:2/0:4,1:0) #intern: 2 -1.92e+00:{-1.03e+00:{a , b }, c }
$ ./scripts/bhcd_plot tiny.tree tiny.fit tiny.pdf
-4.82e+00:{-1.66e-01:{a, b}, c}
```
Then tiny.pdf will have a plot of the clustering.

### Run all tests

## Windows Support

Originally the code does not work on Windows. The bug comes from length of `unsigned long` is 4 bytes on Windows. See [ulong](https://github.com/microsoft/vcpkg/issues/8321) for detail. [This commit](https://github.com/zhaofeng-shu33/bhcd/commit/be90243ed9810fea8db05689fa5915f95c1d9a0a) fixes this problem.

## Reference Paper

Blundell, C., & Teh, Y. W. (2013). Bayesian hierarchical community discovery. In Advances in Neural Information Processing Systems (pp. 1601-1609).

```bibtex
@inproceedings{blundell2013bayesian,
  title={Bayesian hierarchical community discovery},
  author={Blundell, Charles and Teh, Yee Whye},
  booktitle={Advances in Neural Information Processing Systems},
  pages={1601--1609},
  year={2013}
}
```
