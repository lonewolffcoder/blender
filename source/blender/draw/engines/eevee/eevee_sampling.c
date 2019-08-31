/*
 * This program is free software; you can redistribute it and/or
 * modify it under the terms of the GNU General Public License
 * as published by the Free Software Foundation; either version 2
 * of the License, or (at your option) any later version.
 *
 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License
 * along with this program; if not, write to the Free Software Foundation,
 * Inc., 51 Franklin Street, Fifth Floor, Boston, MA 02110-1301, USA.
 *
 * Copyright 2019, Blender Foundation.
 */

/** \file
 * \ingroup EEVEE
 */

#include "eevee_private.h"

#include "BLI_rand.h"

/**
 * Special ball distribution:
 * Point are distributed in a way that when they are orthogonally
 * projected into any plane, the resulting distribution is (close to)
 * a uniform disc distribution.
 */
void EEVEE_sample_ball(int sample_ofs, float radius, float rsample[3])
{
  double ht_point[3];
  double ht_offset[3] = {0.0, 0.0, 0.0};
  uint ht_primes[3] = {2, 3, 7};

  BLI_halton_3d(ht_primes, ht_offset, sample_ofs, ht_point);

  float omega = ht_point[1] * 2.0f * M_PI;

  rsample[2] = ht_point[0] * 2.0f - 1.0f; /* cos theta */

  float r = sqrtf(fmaxf(0.0f, 1.0f - rsample[2] * rsample[2])); /* sin theta */

  rsample[0] = r * cosf(omega);
  rsample[1] = r * sinf(omega);

  radius *= sqrt(sqrt(ht_point[2]));
  mul_v3_fl(rsample, radius);
}

void EEVEE_sample_rectangle(int sample_ofs,
                            const float x_axis[3],
                            const float y_axis[3],
                            float size_x,
                            float size_y,
                            float rsample[3])
{
  double ht_point[2];
  double ht_offset[2] = {0.0, 0.0};
  uint ht_primes[2] = {2, 3};

  BLI_halton_2d(ht_primes, ht_offset, sample_ofs, ht_point);

  /* Change ditribution center to be 0,0 */
  ht_point[0] = (ht_point[0] > 0.5f) ? ht_point[0] - 1.0f : ht_point[0];
  ht_point[1] = (ht_point[1] > 0.5f) ? ht_point[1] - 1.0f : ht_point[1];

  zero_v3(rsample);
  madd_v3_v3fl(rsample, x_axis, (ht_point[0] * 2.0f) * size_x);
  madd_v3_v3fl(rsample, y_axis, (ht_point[1] * 2.0f) * size_y);
}

void EEVEE_sample_ellipse(int sample_ofs,
                          const float x_axis[3],
                          const float y_axis[3],
                          float size_x,
                          float size_y,
                          float rsample[3])
{
  double ht_point[2];
  double ht_offset[2] = {0.0, 0.0};
  uint ht_primes[2] = {2, 3};

  BLI_halton_2d(ht_primes, ht_offset, sample_ofs, ht_point);

  /* Uniform disc sampling. */
  float omega = ht_point[1] * 2.0f * M_PI;
  float r = sqrtf(ht_point[0]);
  ht_point[0] = r * cosf(omega) * size_x;
  ht_point[1] = r * sinf(omega) * size_y;

  zero_v3(rsample);
  madd_v3_v3fl(rsample, x_axis, ht_point[0]);
  madd_v3_v3fl(rsample, y_axis, ht_point[1]);
}