#include <stdio.h>
#include <time.h>
#include <stdint.h>

#include "rt_utils.h"

void *feedb_start(void)
{
  return NULL;
}

int64_t feedb_endcycle(void *h, void *ph)
{
  wait_next_activation(ph);

  return 0;
}

void feedb_print_stats(const char *prefix, int id)
{
}

void feedb_init(void)
{
}
