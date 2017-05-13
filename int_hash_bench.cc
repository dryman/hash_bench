/* int_hash_bench.c ---
 *
 * Filename: int_hash_bench.c
 * Description:
 * Author: Felix Chern
 * Maintainer:
 * Copyright: (c) 2017 Felix Chern
 * Created: Mon Apr  3 22:07:32 2017 (-0700)
 * Version:
 * Package-Requires: ()
 * Last-Updated:
 *           By:
 *     Update #: 0
 * URL:
 * Doc URL:
 * Keywords:
 * Compatibility:
 *
 */

/* Commentary:
 *
 *
 *
 */

/* Change Log:
 *
 *
 */

/* This program is free software: you can redistribute it and/or modify
 * it under the terms of the GNU Lesser General Public License as published by
 * the Free Software Foundation, either version 3 of the License, or (at
 * your option) any later version.
 *
 * This program is distributed in the hope that it will be useful, but
 * WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU
 * Lesser General Public License for more details.
 *
 * You should have received a copy of the GNU Lesser General Public License
 * along with this program.  If not, see <http://www.gnu.org/licenses/>.
 */

/* Code: */

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <sys/time.h>
#include <sys/types.h>
#include <inttypes.h>
#include <unistd.h>
#include "opic/common/op_assert.h"
#include "opic/op_malloc.h"
#include "opic/hash/robin_hood.h"
#include "khash.h"

#include <fstream>
#include <string>
#include <unordered_map>
#include <unordered_set>
#include <google/dense_hash_map>
#include <google/sparse_hash_map>
#include <google/dense_hash_set>
#include <google/sparse_hash_set>
#include <libcuckoo/cuckoohash_map.hh>

static void print_timediff(const char* info,
                           struct timeval start, struct timeval end);

typedef void (*HashFunc)(int num, unsigned int pause);

//KHASH_MAP_INIT_STR(str, uint64_t)
KHASH_MAP_INIT_INT(khash_int32, uint64_t)

// A side effect to force hash get not optimized out.
static uint64_t side_effect;

uint64_t TomasWangInt64Hash(void* key_generic, size_t size)
{
  uint64_t key;
  key = *(uint64_t*)key_generic;
  key = (~key) + (key << 21); // key = (key << 21) - key - 1;
  key = key ^ (key >> 24);
  key = (key + (key << 3)) + (key << 8); // key * 265
  key = key ^ (key >> 14);
  key = (key + (key << 2)) + (key << 4); // key * 21
  key = key ^ (key >> 28);
  key = key + (key << 31);
  return key;
}

uint64_t TomasWangIntHash(void* key_generic, size_t size)
{
  uint64_t key;
  key = *(uint32_t*)key_generic;
  key += ~(key << 15);
  key ^=  (key >> 10);
  key +=  (key << 3);
  key ^=  (key >> 6);
  key += ~(key << 11);
  key ^=  (key >> 16);
  return key;
}

void rhh_map_int32(int num, unsigned int pause)
{
  OPHeap* heap;
  RobinHoodHash* rhh;
  struct timeval i_start, i_end, q_start, q_end;

  op_assert(OPHeapNew(&heap), "Create OPHeap\n");
  op_assert(RHHNew(heap, &rhh, num,
                   0.25, 4, 8), "Create RobinHoodHash\n");

  gettimeofday(&i_start, NULL);
  for (uint32_t i = 0; i < num; i++)
    {
      uint64_t val = i;
      RHHPutCustom(rhh, TomasWangIntHash, &i, &val);
    }
  gettimeofday(&i_end, NULL);
  printf("insert finished\n");
  sleep(pause);

  gettimeofday(&q_start, NULL);
  for (uint32_t i = 0; i < num; i++)
    {
      uint64_t* val;
      val = (uint64_t*)RHHGetCustom(rhh, TomasWangIntHash, &i);
      side_effect+= *val;
    }
  gettimeofday(&q_end, NULL);

  print_timediff("Insert time: ", i_start, i_end);
  print_timediff("Query time: ", q_start, q_end);
  RHHDestroy(rhh);
  OPHeapDestroy(heap);
}

void rhh_map_int64(int num, unsigned int pause)
{
  OPHeap* heap;
  RobinHoodHash* rhh;
  struct timeval i_start, i_end, q_start, q_end;

  op_assert(OPHeapNew(&heap), "Create OPHeap\n");
  op_assert(RHHNew(heap, &rhh, num,
                   0.95, 8, 8), "Create RobinHoodHash\n");

  gettimeofday(&i_start, NULL);
  for (uint64_t i = 0; i < num; i++)
    {
      uint64_t val = i;
      RHHPutCustom(rhh, TomasWangInt64Hash, &i, &val);
    }
  gettimeofday(&i_end, NULL);
  printf("insert finished\n");
  sleep(pause);

  gettimeofday(&q_start, NULL);
  for (uint64_t i = 0; i < num; i++)
    {
      uint64_t* val;
      val = (uint64_t*)RHHGetCustom(rhh, TomasWangInt64Hash, &i);
      side_effect+= *val;
    }
  gettimeofday(&q_end, NULL);

  print_timediff("Insert time: ", i_start, i_end);
  print_timediff("Query time: ", q_start, q_end);
  RHHDestroy(rhh);
  OPHeapDestroy(heap);
}

void rhh_set_int32(int num, unsigned int pause)
{
  OPHeap* heap;
  RobinHoodHash* rhh;
  struct timeval i_start, i_end, q_start, q_end;

  op_assert(OPHeapNew(&heap), "Create OPHeap\n");
  op_assert(RHHNew(heap, &rhh, num,
                   0.95, 4, 0), "Create RobinHoodHash\n");

  gettimeofday(&i_start, NULL);
  for (uint32_t i = 0; i < num; i++)
    {
      RHHPutCustom(rhh, TomasWangIntHash, &i, NULL);
    }
  gettimeofday(&i_end, NULL);
  printf("insert finished\n");
  sleep(pause);

  gettimeofday(&q_start, NULL);
  for (uint32_t i = 0; i < num; i++)
    {
      if (RHHGetCustom(rhh, TomasWangIntHash, &i))
        side_effect++;
    }
  gettimeofday(&q_end, NULL);

  print_timediff("Insert time: ", i_start, i_end);
  print_timediff("Query time: ", q_start, q_end);
  RHHDestroy(rhh);
  OPHeapDestroy(heap);
}

void rhh_set_int64(int num, unsigned int pause)
{
  OPHeap* heap;
  RobinHoodHash* rhh;
  struct timeval i_start, i_end, q_start, q_end;

  op_assert(OPHeapNew(&heap), "Create OPHeap\n");
  op_assert(RHHNew(heap, &rhh, num,
                   0.95, 8, 0), "Create RobinHoodHash\n");

  gettimeofday(&i_start, NULL);
  for (uint64_t i = 0; i < num; i++)
    {
      RHHPutCustom(rhh, TomasWangIntHash, &i, NULL);
    }
  gettimeofday(&i_end, NULL);
  printf("insert finished\n");
  sleep(pause);

  gettimeofday(&q_start, NULL);
  for (uint64_t i = 0; i < num; i++)
    {
      if (RHHGetCustom(rhh, TomasWangIntHash, &i))
        side_effect++;
    }
  gettimeofday(&q_end, NULL);

  print_timediff("Insert time: ", i_start, i_end);
  print_timediff("Query time: ", q_start, q_end);
  RHHDestroy(rhh);
  OPHeapDestroy(heap);
}

void umap_int32(int num, unsigned int pause)
{
  struct timeval i_start, i_end, q_start, q_end;
  auto umap = new std::unordered_map<uint32_t, uint64_t>(num);

  gettimeofday(&i_start, NULL);
  for (uint32_t i = 0; i < num; i++)
    {
      umap->insert(std::make_pair(i, i));
    }

  gettimeofday(&i_end, NULL);
  printf("insert finished\n");
  sleep(pause);

  gettimeofday(&q_start, NULL);
  for (uint32_t i = 0; i < num; i++)
    {
      side_effect += umap->at(i);
    }
  gettimeofday(&q_end, NULL);

  delete umap;

  print_timediff("Insert time: ", i_start, i_end);
  print_timediff("Query time: ", q_start, q_end);
}

void umap_int64(int num, unsigned int pause)
{
  struct timeval i_start, i_end, q_start, q_end;
  auto umap = new std::unordered_map<uint64_t, uint64_t>(num);

  gettimeofday(&i_start, NULL);
  for (uint64_t i = 0; i < num; i++)
    {
      umap->insert(std::make_pair(i, i));
    }

  gettimeofday(&i_end, NULL);
  printf("insert finished\n");
  sleep(pause);

  gettimeofday(&q_start, NULL);
  for (uint64_t i = 0; i < num; i++)
    {
      side_effect += umap->at(i);
    }
  gettimeofday(&q_end, NULL);

  delete umap;

  print_timediff("Insert time: ", i_start, i_end);
  print_timediff("Query time: ", q_start, q_end);
}

void uset_int32(int num, unsigned int pause)
{
  struct timeval i_start, i_end, q_start, q_end;
  auto uset = new std::unordered_set<uint32_t>(num);

  gettimeofday(&i_start, NULL);
  for (uint32_t i = 0; i < num; i++)
    {
      uset->insert(i);
    }

  gettimeofday(&i_end, NULL);
  printf("insert finished\n");
  sleep(pause);

  gettimeofday(&q_start, NULL);
  for (uint32_t i = 0; i < num; i++)
    {
      auto search = uset->find(i);
      if (search != uset->end())
        side_effect ++;
    }
  gettimeofday(&q_end, NULL);

  delete uset;

  print_timediff("Insert time: ", i_start, i_end);
  print_timediff("Query time: ", q_start, q_end);
}

void uset_int64(int num, unsigned int pause)
{
  struct timeval i_start, i_end, q_start, q_end;
  auto uset = new std::unordered_set<uint64_t>(num);

  gettimeofday(&i_start, NULL);
  for (uint64_t i = 0; i < num; i++)
    {
      uset->insert(i);
    }

  gettimeofday(&i_end, NULL);
  printf("insert finished\n");
  sleep(pause);

  gettimeofday(&q_start, NULL);
  for (uint64_t i = 0; i < num; i++)
    {
      auto search = uset->find(i);
      if (search != uset->end())
        side_effect ++;
    }
  gettimeofday(&q_end, NULL);

  delete uset;

  print_timediff("Insert time: ", i_start, i_end);
  print_timediff("Query time: ", q_start, q_end);
}

void dh_map_int32(int num, unsigned int pause)
{
  struct timeval i_start, i_end, q_start, q_end;
  auto dhm = new google::dense_hash_map<uint32_t, uint64_t>(num);
  //dhm->max_load_factor(0.80);
  //dhm->resize(num);
  dhm->set_empty_key(~0U);
  dhm->set_deleted_key(~0U-1);

  gettimeofday(&i_start, NULL);
  for (uint32_t i = 0; i < num; i++)
    {
      dhm->insert(std::make_pair(i, i));
    }
  gettimeofday(&i_end, NULL);
  printf("insert finished\n");
  sleep(pause);

  gettimeofday(&q_start, NULL);
  for (uint32_t i = 0; i < num; i++)
    {
      auto search = dhm->find(i);
      side_effect += search->second;
    }
  gettimeofday(&q_end, NULL);

  print_timediff("Insert time: ", i_start, i_end);
  print_timediff("Query time: ", q_start, q_end);

  delete dhm;
}

void dh_map_int64(int num, unsigned int pause)
{
  struct timeval i_start, i_end, q_start, q_end;
  auto dhm = new google::dense_hash_map<uint64_t, uint64_t>(num);
  //dhm->max_load_factor(0.80);
  //dhm->resize(num);
  dhm->set_empty_key(~0ULL);
  dhm->set_deleted_key(~0ULL-1);

  gettimeofday(&i_start, NULL);
  for (uint64_t i = 0; i < num; i++)
    {
      dhm->insert(std::make_pair(i, i));
    }
  gettimeofday(&i_end, NULL);
  printf("insert finished\n");
  sleep(pause);

  gettimeofday(&q_start, NULL);
  for (uint64_t i = 0; i < num; i++)
    {
      auto search = dhm->find(i);
      side_effect += search->second;
    }
  gettimeofday(&q_end, NULL);

  print_timediff("Insert time: ", i_start, i_end);
  print_timediff("Query time: ", q_start, q_end);

  delete dhm;
}

void sh_map_int32(int num, unsigned int pause)
{
  struct timeval i_start, i_end, q_start, q_end;
  auto shm = new google::sparse_hash_map<uint32_t, uint64_t>(num);
  shm->set_deleted_key(~0U);

  gettimeofday(&i_start, NULL);
  for (uint32_t i = 0; i < num; i++)
    {
      shm->insert(std::make_pair(i, i));
    }
  gettimeofday(&i_end, NULL);
  printf("insert finished\n");
  sleep(pause);

  gettimeofday(&q_start, NULL);
  for (uint32_t i = 0; i < num; i++)
    {
      auto search = shm->find(i);
      side_effect += search->second;
    }
  gettimeofday(&q_end, NULL);

  print_timediff("Insert time: ", i_start, i_end);
  print_timediff("Query time: ", q_start, q_end);

  delete shm;
}

void sh_map_int64(int num, unsigned int pause)
{
  struct timeval i_start, i_end, q_start, q_end;
  auto shm = new google::sparse_hash_map<uint64_t, uint64_t>(num);
  shm->set_deleted_key(~0U);

  gettimeofday(&i_start, NULL);
  for (uint64_t i = 0; i < num; i++)
    {
      shm->insert(std::make_pair(i, i));
    }
  gettimeofday(&i_end, NULL);
  printf("insert finished\n");
  sleep(pause);

  gettimeofday(&q_start, NULL);
  for (uint64_t i = 0; i < num; i++)
    {
      auto search = shm->find(i);
      side_effect += search->second;
    }
  gettimeofday(&q_end, NULL);

  print_timediff("Insert time: ", i_start, i_end);
  print_timediff("Query time: ", q_start, q_end);

  delete shm;
}


void ckoo_int32(int num, unsigned int pause)
{
  struct timeval i_start, i_end, q_start, q_end;
  auto ckoo = new cuckoohash_map<uint32_t, uint64_t>(num);

  printf("cuckoohash_map in memory\n");
  gettimeofday(&i_start, NULL);
  for (uint32_t i = 0; i < num; i++)
    {
      ckoo->insert(i, i);
    }
  gettimeofday(&i_end, NULL);
  printf("insert finished\n");
  sleep(pause);

  gettimeofday(&q_start, NULL);
  for (uint32_t i = 0; i < num; i++)
    {
      side_effect += ckoo->find(i);
    }
  gettimeofday(&q_end, NULL);

  print_timediff("Insert time: ", i_start, i_end);
  print_timediff("Query time: ", q_start, q_end);

  delete ckoo;
}

void kh_int32(int num, unsigned int pause)
{
  struct timeval i_start, i_end, q_start, q_end;
  khash_t(khash_int32) *kh;
  int absent;
  khint_t khint;

  kh = kh_init(khash_int32);

  gettimeofday(&i_start, NULL);

  for (uint32_t i=0; i < num; i++)
    {
      uint64_t val = i;
      khint = kh_put(khash_int32, kh, i, &absent);
      kh_value(kh, khint) = val;
    }

  gettimeofday(&i_end, NULL);

  printf("insert finished\n");
  sleep(pause);

  gettimeofday(&q_start, NULL);
  for (uint32_t i=0; i < num; i++)
    {
      khint = kh_get(khash_int32, kh, i);
      if (khint != kh_end(kh))
        {
          side_effect += kh_value(kh, khint);
        }
    }
  gettimeofday(&q_end, NULL);

  kh_destroy(khash_int32, kh);

  print_timediff("Insert time: ", i_start, i_end);
  print_timediff("Query time: ", q_start, q_end);
}

void help(char* program)
{
  printf
    ("usage: %s [-n power_of_2] [-r repeat] [-k keytype]\n"
     "  [-i impl] [-h]\n"
     "Options:\n"
     "  -n num     Number of elements\n"
     "  -r repeat  Repeat the benchmar for `repeat` times.\n"
     "  -i impl    impl = rhh_map, rhh_set, dense_hash_map, dense_hash_set,\n"
     "                    sparse_hash_map, sparse_hash_set, std_unordered_map\n"
     "                    std_unordered_set, cuckoo_map, khash_map\n"
     "  -p pause   Pause between insertion and query so that we have time to\n"
     "             capture the memory it uses.\n"
     "  -h         print help.\n"
     ,program);
  exit(1);
}

#define FIND_IMPL(IMPL, FUN) \
  (!strcmp(#IMPL, optarg)) { FUN = IMPL; break;}

int main(int argc, char* argv[])
{
  int num, opt;
  int repeat = 1;
  unsigned int pause = 0;
  HashFunc fun = NULL;

  num = 1000;

  while ((opt = getopt(argc, argv, "n:r:k:i:f:m:p:h")) > -1)
    {
      switch (opt)
        {
        case 'n':
          num = atoi(optarg);
          break;
        case 'r':
          repeat = atoi(optarg);
          break;
        case 'i':
          if FIND_IMPL(rhh_map_int32, fun);
          if FIND_IMPL(rhh_map_int64, fun);
          if FIND_IMPL(rhh_set_int32, fun);
          if FIND_IMPL(rhh_set_int64, fun);
          if FIND_IMPL(umap_int32, fun);
          if FIND_IMPL(umap_int64, fun);
          if FIND_IMPL(uset_int32, fun);
          if FIND_IMPL(uset_int64, fun);
          if FIND_IMPL(dh_map_int32, fun);
          if FIND_IMPL(dh_map_int64, fun);
          if FIND_IMPL(sh_map_int32, fun);
          if FIND_IMPL(sh_map_int64, fun);
          if FIND_IMPL(ckoo_int32, fun);
          if FIND_IMPL(kh_int32, fun);
          help(argv[0]);
          break;
        case 'p':
          pause = atoi(optarg);
          break;
        case 'h':
        case '?':
        default:
            help(argv[0]);
        }
    }

  if (!fun)
  {
    help(argv[0]);
  }
  printf("running elements %" PRIu64 "\n", num);

  for (int i = 0; i < repeat; i++)
    {
      printf("attempt %d\n", i + 1);
      fun(num, pause);
    }
  printf("side effect: %" PRIu64 "\n", side_effect);

  return 0;
}


void print_timediff(const char* info, struct timeval start, struct timeval end)
{
  long int second = end.tv_sec - start.tv_sec;
  unsigned int usec;
  if (end.tv_usec < start.tv_usec)
    {
      second--;
      usec = end.tv_usec + 1000000 - start.tv_usec;
    }
  else
    usec = end.tv_usec - start.tv_usec;

  printf("%s%ld.%06u\n", info, second, usec);
}


/* hash_bench.c ends here */
