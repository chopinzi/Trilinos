/*
//@HEADER
// *****************************************************************************
//
//  HPCGraph: Graph Computation on High Performance Computing Systems
//              Copyright (2016) Sandia Corporation
//
// Under the terms of Contract DE-AC04-94AL85000 with Sandia Corporation,
// the U.S. Government retains certain rights in this software.
//
// Redistribution and use in source and binary forms, with or without
// modification, are permitted provided that the following conditions are
// met:
//
// 1. Redistributions of source code must retain the above copyright
// notice, this list of conditions and the following disclaimer.
//
// 2. Redistributions in binary form must reproduce the above copyright
// notice, this list of conditions and the following disclaimer in the
// documentation and/or other materials provided with the distribution.
//
// 3. Neither the name of the Corporation nor the names of the
// contributors may be used to endorse or promote products derived from
// this software without specific prior written permission.
//
// THIS SOFTWARE IS PROVIDED BY SANDIA CORPORATION "AS IS" AND ANY
// EXPRESS OR IMPLIED WARRANTIES, INCLUDING, BUT NOT LIMITED TO, THE
// IMPLIED WARRANTIES OF MERCHANTABILITY AND FITNESS FOR A PARTICULAR
// PURPOSE ARE DISCLAIMED. IN NO EVENT SHALL SANDIA CORPORATION OR THE
// CONTRIBUTORS BE LIABLE FOR ANY DIRECT, INDIRECT, INCIDENTAL, SPECIAL,
// EXEMPLARY, OR CONSEQUENTIAL DAMAGES (INCLUDING, BUT NOT LIMITED TO,
// PROCUREMENT OF SUBSTITUTE GOODS OR SERVICES; LOSS OF USE, DATA, OR
// PROFITS; OR BUSINESS INTERRUPTION) HOWEVER CAUSED AND ON ANY THEORY OF
// LIABILITY, WHETHER IN CONTRACT, STRICT LIABILITY, OR TORT (INCLUDING
// NEGLIGENCE OR OTHERWISE) ARISING IN ANY WAY OUT OF THE USE OF THIS
// SOFTWARE, EVEN IF ADVISED OF THE POSSIBILITY OF SUCH DAMAGE.
//
// Questions?  Contact  George M. Slota   (gmslota@sandia.gov)
//                      Siva Rajamanickam (srajama@sandia.gov)
//                      Kamesh Madduri    (madduri@cse.psu.edu)
//
// *****************************************************************************
//@HEADER
*/

#include <mpi.h>
#include <omp.h>
#include <stdio.h>
#include <stdlib.h>
#include <stdint.h>

#include "fast_map.h"
#include "util.h"

extern int procid, nprocs;
extern bool verbose, debug;

bool is_init(color_fast_map* map)
{
  return (map->capacity > 0);
}

void init_map(color_fast_map* map)
{
  map->arr = NULL;
  map->capacity = 0;
  map->hashing = false;
}

void init_map(color_fast_map* map, uint64_t init_size)
{
  map->arr = (uint64_t*)malloc(init_size*2*sizeof(uint64_t));
  if (map->arr == NULL)
    color_throw_err("init_map(), unable to allocate resources\n", procid);

  map->capacity = init_size;
  map->hashing = true;
  
//#pragma omp parallel for
  for (uint64_t i = 0; i < map->capacity; ++i)
    map->arr[i*2] = NULL_KEY;
}

void init_map_nohash(color_fast_map* map, uint64_t init_size)
{
  map->arr = (uint64_t*)malloc(init_size*sizeof(uint64_t));
  if (map->arr == NULL)
    color_throw_err("init_map_nohash(), unable to allocate resources\n", procid);

  map->capacity = init_size;
  map->hashing = false;
  
//#pragma omp parallel for
  for (uint64_t i = 0; i < map->capacity; ++i)
    map->arr[i] = NULL_KEY;
}

void set_map_id(color_fast_map* map)
{
//#pragma omp parallel for
  for (uint64_t i = 0; i < map->capacity; ++i)
    map->arr[i] = i;
}

void clear_map(color_fast_map* map)
{
  free(map->arr);
  map->capacity = 0;
}
