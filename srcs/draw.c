/*
 * This file is part of the ldm distribution (https://github.com/valkheim/ldm)
 * Copyright (c) 2018 Charles Paulet.
 *
 * This program is free software: you can redistribute it and/or modify
 * it under the terms of the GNU General Public License as published by
 * the Free Software Foundation, version 3.
 *
 * This program is distributed in the hope that it will be useful, but
 * WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE. See the GNU
 * General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License
 * along with this program. If not, see <http://www.gnu.org/licenses/>.
*/

#include <string.h>
#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>

#include "config.h"
#include "window.h"
#include "main.h"
#include "draw.h"

xcb_gcontext_t main_ctx;
xcb_gcontext_t ctxs[CTXS_NUMBER];

static void text_draw(int16_t const x1, int16_t const y1, const char * const label)
{
  xcb_void_cookie_t const cookie_text = xcb_image_text_8_checked(c, strlen(label), win, font_ctx, x1, y1, label);
  xcb_generic_error_t const * error = xcb_request_check(c, cookie_text);
  if (error)
  {
    fprintf(stderr, "ERROR: can't paste text : %d\n", error->error_code);
    return;
  }
}

void draw(void)
{
  xcb_rectangle_t rectangles[] = {
  /* x, y, wdt, hgt */
    {0, 0, BORDER_WIDTH, screen->height_in_pixels},
    {(int16_t)(screen->width_in_pixels - BORDER_WIDTH), 0, BORDER_WIDTH, screen->height_in_pixels},
    {0, 0, screen->width_in_pixels, BORDER_WIDTH},
    {0, (int16_t)(screen->height_in_pixels - BORDER_WIDTH), screen->width_in_pixels, BORDER_WIDTH},
  };

  xcb_poly_fill_rectangle(c, win, main_ctx, sizeof(rectangles) / sizeof(rectangles[0]), rectangles);
  xcb_image_text_8(c, 5, win, ctxs[CTX_TEXT], 40, 40, "hello");
  char * text = "Press ESC key to exit...";
  text_draw(10, 100 - 10, text);
  xcb_flush(c);
}

static void *color_border(void *arg)
{
  t_draw_options * const colors = (t_draw_options * const)arg;
  pthread_mutex_lock(&lock_ctxs);
  main_ctx = ctxs[colors->from];
  pthread_mutex_unlock(&lock_ctxs);
  draw();
  usleep(200000);
  pthread_mutex_lock(&lock_ctxs);
  main_ctx = ctxs[colors->to];
  pthread_mutex_unlock(&lock_ctxs);
  draw();
  free(colors);
  return NULL;
}

void draw_borders(t_draw_options *args)
{
  pthread_t tid;
  int const err = pthread_create(&tid, NULL, &color_border, (void *)args);
  if (err != 0)
    printf("\ncan't create thread :[%s]", strerror(err));
  else
    pthread_detach(tid);
}
