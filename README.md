[![Build Status](https://api.travis-ci.com/kstenschke/lspng.svg?branch=main)](https://travis-ci.com/kstenschke/lspng)
[![Language grade: C/C++](https://img.shields.io/lgtm/grade/cpp/g/kstenschke/lspng.svg?logo=lgtm&logoWidth=18)](https://lgtm.com/projects/g/kstenschke/lspng/context:cpp)
[![CodeFactor](https://www.codefactor.io/repository/github/kstenschke/lspng/badge)](https://www.codefactor.io/repository/github/kstenschke/lspng)


# lspng - Sort PNGs by Luminance

## Table of Contents

* [What does it do?](#what-does-it-do)
* [Command: ``--version`` or ``-v``](#command---version-or--v)
* [Option: ``--desc`` or ``-d``](#option---desc-or--d)
* [Option: ``--amount_digits_min=`` or ``-a=``](#option---amount_digits_min-or--a)
* [Build from source](#build-from-source)
* [Install](#install)
* [Conventions](#conventions)
* [Changelog](#changelog)
* [Used third party libraries](#used-third-party-libraries)
* [Author and License](#author-and-license)


## What does it do?

Running ``lspng`` prefixes each PNG image in that path with its relative 
luminance-rank. 
Sorting those files alphabetically than also sorts them by luminance.


## Command: ``--version`` or ``-v``

Print information about installed version of ``lspng``, its license and
author(s). 


## Option: ``--desc`` or ``-d``

By default lspng sorts PNGs ordered from lightest to darkest.
Running ``lspng -d`` will sort/rename the files descending instead, that is from
darkest to lightest.


## Option: ``--amount_digits_min=`` or ``-a=``

Running ``lspng -a=3`` will set a minimum prefix length of three digits.
Instead of e.g. `0_foo.png` the lightest PNG image than will be named 
`000_foo.png`.
 

## Build from source

Initially fetch dependencies:``git submodule update --init --recursive``
  
Than: ``cmake CMakeLists.txt; make``


## Install

Build from source, than: ``sudo make install``


## Conventions

The source code of **lspng** follows the Google C++ Style Guide, 
see: https://google.github.io/styleguide/cppguide.html    

**lspng** follows the [Semantic Versioning](https://semver.org) Scheme.


## Changelog

See [CHANGELOG.md](CHANGELOG.md)


## Used third party libraries

| Application                          | License                                                                                                                                                                                                                            |
| ------------------------------------ | ---------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------- |
| [The CImg Library](https://cimg.eu/) | Distributed under the [CeCILL-C](http://www.cecill.info/licences/Licence_CeCILL-C_V1-en.txt) (close to the GNU LGPL) or [CeCILL](http://www.cecill.info/licences/Licence_CeCILL_V2-en.txt) (compatible with the GNU GPL) licenses. |


## Author and License

**lspng** was written by Kay Stenschke and is licensed under the 
[GNU General Public License V3.0](https://www.gnu.org/licenses/licenses.html#GPL)  

```
Permissions of this strong copyleft license are conditioned on making available 
complete source code of licensed works and modifications, which include larger 
works using a licensed work, under the same license. Copyright and license 
notices must be preserved. Contributors provide an express grant of patent 
rights.
```
