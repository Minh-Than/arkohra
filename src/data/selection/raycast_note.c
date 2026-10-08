#include <math.h>
#include "raycast_note.h"

int raycast_note_compare_distance_asc(const void *a, const void *b)
{
  const RaycastNote *ray_a = (const RaycastNote *)a;
  const RaycastNote *ray_b = (const RaycastNote *)b;
  float hit_dist_a = ray_a->collision.distance;
  float hit_dist_b = ray_b->collision.distance;
  if (fabsf(hit_dist_a - hit_dist_b) > 1e-6) return hit_dist_a - hit_dist_b;
  return 0;
}

int raycast_note_compare_data(const void *a, const void *b)
{
  const RaycastNote *ray_a = (const RaycastNote *)a;
  const RaycastNote *ray_b = (const RaycastNote *)b;
  return ray_a->data == ray_b->data ? 0 : 1;
}
