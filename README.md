# lspng - Sort PNGs by Luminance

Running ``lspng`` prefixes each PNG image in that path with its relative 
luminance-rank. 
Sorting those files alphabetically than also sorts them by luminance.


## Option: ``--desc`` or ``-d``

By default lspng sorts PNGs ordered from lightest to darkest.
Running ``lspng -d`` will sort/rename the files descending instead, that is from
darkest to lightest.


## Option: amount_digits

Running ``lspng a=3`` will enforce a minmum prefix length of three digits.
Instead of e.g. `0_foo.png` the lightest PNG image than will be renamed to 
`000_foo.png`.
 
