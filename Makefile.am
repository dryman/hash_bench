#SUBDIRS = opic
noinst_PROGRAMS = hash_bench robin_only

hash_bench_SOURCES = hash_bench.cc
#hash_bench_CXXFLAGS = -I$(top_builddir)/opic -I/usr/local/include
hash_bench_CXXFLAGS = -I/usr/local/include
hash_bench_LDADD = /usr/local/lib/libopic.a @atomic_LIBS@ @log4c_LIBS@
#hash_bench_LDFLAGS = -static
#hash_bench_LDADD = $(top_builddir)/opic/opic/libopic.a \
#hash_bench_LDFLAGS = -L/usr/local/lib/ -lopic @atomic_LIBS@ @log4c_LIBS@
# -lboost_thread-mt \
#  -lboost_iostreams -lboost_serialization
#hash_bench_LDFLAGS = -static -static-libgcc -static-libstdc++

robin_only_SOURCES = robin_only.c
robin_only_CFLAGS = -I/usr/local/include
robin_only_LDADD = /usr/local/lib/libopic.a @atomic_LIBS@ @log4c_LIBS@
