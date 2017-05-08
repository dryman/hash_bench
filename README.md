Hash Table Benchmark
====================

INSTALL
-------

Dependencies:

* [log4c][log4c]
* [opic robin hood hash][opic]
* [sparse hash][sparse]
* [libcuckoo][cuckoo]
* [autoconf][autoconf]
* [automake][automake]

[log4c]: http://log4c.sourceforge.net
[opic]: https://github.com/dryman/opic
[sparse]: https://github.com/sparsehash/sparsehash
[cuckoo]: https://github.com/efficient/libcuckoo
[autoconf]: https://www.gnu.org/software/autoconf/autoconf.html
[automake]: https://www.gnu.org/software/automake/

```
autoreconf -vif
./configure
make
```

SYNOPSIS
--------

```
 ± ./hash_bench -h
usage: ./hash_bench [-n power_of_2] [-r repeat] [-k keytype]
  [-i impl] [-m mode] [-f file] [-p] [-h]
Options:
  -n num     Number of elements measured in power of 2.
             -n 20 => run 2^20 = 1 million elements.
             defaults to 20
  -r repeat  Repeat the benchmar for `repeat` times.
  -k keytype keytype = short_string, mid_string, long_string or
             long_int
             short_string: 6 bytes, mid_string: 32 bytes,
             long_string: 256 bytes, long_int: 8 bytes
             For now only robin_hood hash supports long_int benchmark
  -i impl    impl = robin_hood, dense_hash_map, sparse_hash_map,
                    std_unordered_map, cuckoo, khash
  -m mode    mode = in_memory, serialize, deserialize, or de_no_cache
             in_memory: benchmark hash map creation time and query time
               supported impl: all
             serialize: hash_map creation time and serialization time
               supported impl: robin_hood, sparse_hash_map
                               std_unordered_map
             deserialize: deserialize hash map then query for 2^n times
               supported impl: robin_hood, sparse_hash_map
                               std_unordered_map
             de_no_cache: measures bare deserialization performance
               supported impl: robin_hood
  -p pause   Pause between insertion and query so that we have time to
             capture the memory it use.
  -f file    file used in serialize, deserialize and deserialize_cached
             mode.
  -h         print help.
```

LICENSE and atributions
-----------------------

* OPIC robin hood hash is released by Felix Chern, under [LGPL v3][rhh_license].
* dense hash map and sparse hash map were release by google, Copyright (c) 2005,
  Google Inc. Under [3 clause BSD license][sparse_license].
* libcuckoo: Copyright (C) 2013, Carnegie Mellon University and Intel
  Corporation. Under [apache license 2.0][cuckoo_license].
* klib/khash is distributed under [MIT/X11 license][klib_license].

[rhh_license]: https://github.com/dryman/opic/blob/master/COPYING.LESSER
[sparse_license]: https://github.com/sparsehash/sparsehash/blob/master/COPYING
[cuckoo_license]: https://github.com/efficient/libcuckoo/blob/master/LICENSE
[klib_license]: https://github.com/attractivechaos/klib
