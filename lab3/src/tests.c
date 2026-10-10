#include <assert.h>
#include <limits.h>
#include <stdio.h>

#include "find_min_max.h"

int main(void) {
  int values[] = {8, -3, 15, 0, 7};
  struct MinMax result = GetMinMax(values, 0, 5);
  assert(result.min == -3 && result.max == 15);

  result = GetMinMax(values, 1, 4);
  assert(result.min == -3 && result.max == 15);

  result = GetMinMax(values, 3, 5);
  assert(result.min == 0 && result.max == 7);

  int one[] = {42};
  result = GetMinMax(one, 0, 1);
  assert(result.min == 42 && result.max == 42);

  int equal[] = {5, 5, 5};
  result = GetMinMax(equal, 0, 3);
  assert(result.min == 5 && result.max == 5);

  int extremes[] = {INT_MIN, 0, INT_MAX};
  result = GetMinMax(extremes, 0, 3);
  assert(result.min == INT_MIN && result.max == INT_MAX);

  result = GetMinMax(values, 2, 2);
  assert(result.min == INT_MAX && result.max == INT_MIN);

  puts("All GetMinMax tests passed");
  return 0;
}
