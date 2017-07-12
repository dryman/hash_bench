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


typedef uint64_t (*HashFunc)(void* key, void* context, OPHash hasher);
typedef void (*RunKey)(int size, HashFunc hash_func,
                       void* context, OPHash hasher);
static void run_short_keys(int size, HashFunc hash_func,
                           void* context, OPHash hasher);
static void run_mid_keys(int size, HashFunc hash_func,
                         void* context, OPHash hasher);
static void run_long_keys(int size, HashFunc hash_func,
                          void* context, OPHash hasher);

static void run_int32(int size, HashFunc hash_func,
                      void* context, OPHash hasher);
static void run_int64(int size, HashFunc hash_func,
                      void* context, OPHash hasher);

static void print_timediff(const char* info,
                           struct timeval start, struct timeval end);

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

uint64_t rhh_put(void* key, void* context, OPHash hasher)
{
  RobinHoodHash* rhh = (RobinHoodHash*)context;
  uint64_t val = 0;
  RHHInsertCustom(rhh, hasher, key, &val);
  return 0;
}

uint64_t rhh_get(void* key, void* context, OPHash hasher)
{
  RobinHoodHash* rhh = (RobinHoodHash*)context;
  return *(uint64_t*)RHHGetCustom(rhh, hasher, key);
}

void help(char* program)
{
  printf
    ("usage: %s [-n power_of_2] [-r repeat] [-k keytype] [-h]\n"
     "Options:\n"
     "  -n num     Number of elements measured in power of 2.\n"
     "             -n 20 => run 2^20 = 1 million elements.\n"
     "             defaults to 20\n"
     "  -r repeat  Repeat the benchmar for `repeat` times.\n"
     "  -k keytype keytype = s_string, m_string, l_string,\n"
     "             int32, int64\n"
     "             s_string: 6 bytes, m_string: 32 bytes,\n"
     "             l_string: 256 bytes\n"
     "  -h         print help.\n"
     ,program);
  exit(1);
}

int main(int argc, char* argv[])
{
  int num_power, opt;
  int repeat = 1;
  num_power = 20;
  RunKey key_func = run_short_keys;
  int k_len = 6;
  uint64_t num;
  OPHeap* heap;
  RobinHoodHash* rhh;
  double load = 0.8;
  struct timeval start, mid, end;
  OPHash hasher = &OPDefaultHash;

  while ((opt = getopt(argc, argv, "n:r:k:l:h")) > -1)
    {
      switch (opt)
        {
        case 'n':
          num_power = atoi(optarg);
          break;
        case 'r':
          repeat = atoi(optarg);
          break;
        case 'k':
          if (!strcmp("s_string", optarg))
            {
              key_func = run_short_keys;
              k_len = 6;
            }
          else if (!strcmp("m_string", optarg))
            {
              key_func = run_mid_keys;
              k_len = 32;
            }
          else if (!strcmp("l_string", optarg))
            {
              key_func = run_long_keys;
              k_len = 256;
            }
          else if (!strcmp("int32", optarg))
            {
              key_func = run_int32;
              //hasher = TomasWangIntHash;
              k_len = 4;
            }
          else if (!strcmp("int64", optarg))
            {
              key_func = run_int64;
              hasher = TomasWangInt64Hash;
              k_len = 8;
            }
          else
            help(argv[0]);
          break;
        case 'l':
          load = atof(optarg);
          break;
        case 'h':
        case '?':
        default:
            help(argv[0]);
        }
    }
  num = 1UL << num_power;
  printf("running elements %" PRIu64 "\n", num);

  op_assert(OPHeapNew(&heap), "Create OPHeap\n");

  for (int i = 0; i < repeat; i++)
    {
      printf("attempt %d\n", i + 1);
      op_assert(RHHNew(heap, &rhh, num,
                       load, k_len, 8), "Create RobinHoodHash\n");
      gettimeofday(&start, NULL);
      key_func(num_power, rhh_put, rhh, hasher);
      printf("insert finished\n");
      gettimeofday(&mid, NULL);
      key_func(num_power, rhh_get, rhh, hasher);
      gettimeofday(&end, NULL);

      print_timediff("Insert time: ", start, mid);
      print_timediff("Query time: ", mid, end);

      RHHDestroy(rhh);
    }

  OPHeapDestroy(heap);
  return 0;

}

void run_short_keys(int size, HashFunc hash_func,
                    void* context, OPHash hasher)
{
  op_assert(size >= 12, "iteration size must > 2^12\n");
  int i_bound = 1 << (size - 12);
  char uuid [] = "!!!!!!";
  uint64_t counter = 0;
  for (int i = 0; i < i_bound; i++)
    {
      for (int j = 2, val = counter >> 12; j < 6; j++, val>>=6)
        {
          uuid[j] = 0x21 + (val & 0x3F);
        }
      for (int j = 0; j < 64; j++)
        {
          uuid[1] = 0x21 + j;
          for (int k = 0; k < 64; k++)
            {
              uuid[0] = 0x21 + k;
              counter++;
              hash_func(uuid, context, hasher);
            }
        }
    }
}

void run_mid_keys(int size, HashFunc hash_func,
                  void* context, OPHash hasher)
{
  op_assert(size >= 12, "iteration size must > 2^12\n");
  int i_bound = 1 << (size - 12);
  char uuid [] = "!!!!!!--!!!!!!--!!!!!!--!!!!!!--";
  uint64_t counter = 0;
  for (int i = 0; i < i_bound; i++)
    {
      for (int j = 2, val = counter >> 12; j < 6; j++, val>>=6)
        {
          uuid[j] = 0x21 + (val & 0x3F);
          uuid[j+8] = 0x21 + (val & 0x3F);
          uuid[j+16] = 0x21 + (val & 0x3F);
          uuid[j+24] = 0x21 + (val & 0x3F);
        }
      for (int j = 0; j < 64; j++)
        {
          uuid[1] = 0x21 + j;
          uuid[1+8] = 0x21 + j;
          uuid[1+16] = 0x21 + j;
          uuid[1+24] = 0x21 + j;
          for (int k = 0; k < 64; k++)
            {
              uuid[0] = 0x21 + k;
              uuid[0+8] = 0x21 + k;
              uuid[0+16] = 0x21 + k;
              uuid[0+24] = 0x21 + k;
              counter++;
              hash_func(uuid, context, hasher);
            }
        }
    }
}

void run_long_keys(int size, HashFunc hash_func,
                   void* context, OPHash hasher)
{
  op_assert(size >= 12, "iteration size must > 2^12\n");
  int i_bound = 1 << (size - 12);
  char uuid [] =
    "!!!!!!--!!!!!!--!!!!!!--!!!!!!--"
    "!!!!!!--!!!!!!--!!!!!!--!!!!!!--"
    "!!!!!!--!!!!!!--!!!!!!--!!!!!!--"
    "!!!!!!--!!!!!!--!!!!!!--!!!!!!--"
    "!!!!!!--!!!!!!--!!!!!!--!!!!!!--"
    "!!!!!!--!!!!!!--!!!!!!--!!!!!!--"
    "!!!!!!--!!!!!!--!!!!!!--!!!!!!--"
    "!!!!!!--!!!!!!--!!!!!!--!!!!!!--";
  uint64_t counter = 0;
  for (int i = 0; i < i_bound; i++)
    {
      for (int j = 2, val = counter >> 12; j < 6; j++, val>>=6)
        {
          for (int k = 0; k < 256; k+=8)
            uuid[j+k] = 0x21 + (val & 0x3F);
        }
      for (int j = 0; j < 64; j++)
        {
          for (int h = 0; h < 256; h+=8)
            uuid[h+1] = 0x21 + j;
          for (int k = 0; k < 64; k++)
            {
              for (int h = 0; h < 256; h+=8)
                uuid[h] = 0x21 + k;
              counter++;
              hash_func(uuid, context, hasher);
            }
        }
    }
}

void run_int32(int size, HashFunc hash_func,
               void* context, OPHash hasher)
{
  op_assert(size >= 12, "iteration size must > 2^12\n");
  uint32_t i_bound = 1 << size;
  for (uint32_t i = 0; i < i_bound; i++)
    {
      hash_func(&i, context, hasher);
    }
}

void run_int64(int size, HashFunc hash_func,
               void* context, OPHash hasher)
{
  op_assert(size >= 12, "iteration size must > 2^12\n");
  uint64_t i_bound = 1 << size;
  for (uint64_t i = 0; i < i_bound; i++)
    {
      hash_func(&i, context, hasher);
    }
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
