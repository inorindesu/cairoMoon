/* 
 * Code of cairoMoon.
 *
 * Copyright (C) 2013  Inori Sakura <inorindesu@gmail.com>
 * 
 * This work is free. You can redistribute it and/or modify it under the
 * terms of the Do What The Fuck You Want To Public License, Version 2,
 * as published by Sam Hocevar. See the COPYING file for more details.
 */

#define _POSIX_C_SOURCE 200809L /* for strdup */

#include <cairo.h>
#include <cairo-pdf.h>
#include <lua.h>
#include <lualib.h>
#include <lauxlib.h>
#include <stdio.h>
#include <string.h>
#include <stdbool.h>
#include <stdlib.h>

char CONTEXT_KEY[] =  "CAIROMOON_CONTEXT";

/*
 * lua-cairo glue
 */
int cairomoon_moveto(lua_State* state)
{
  double x = lua_tonumber(state, 1);
  double y = lua_tonumber(state, 2);
  lua_pop(state, 2);
  
  lua_pushstring(state, CONTEXT_KEY);
  lua_gettable(state, LUA_REGISTRYINDEX);
  cairo_t* context = lua_touserdata(state, -1);
  lua_pop(state, 1);

  cairo_move_to(context, x, y);
  return 0;
}

/*
 * if a function is expected to be used as follows
 * in lua:
 *
 * a, b = a_c_function(c, d, e)
 *
 * then a C function should be created:
 *
 * int function_interanl_name(lua_State* s)
 * {
 *   char* param1 = lua_tostring(state, 1);
 *   int param2 = lua_tointeger(state, 2);
 *   double param3 = lua_tonumber(state, 3);
 *   // do lots of works!
 *   // assumes:
 *   // char* result1;  
 *   // int result2;
 *   lua_pushstring(state, result1);   // a
 *   lua_pushinteger(state, result2);  // b
 *   return 2;
 * }
 */

int cairomoon_lineto(lua_State* state)
{
  double x = lua_tonumber(state, 1);
  double y = lua_tonumber(state, 2);
  lua_pop(state, 2);
  
  lua_pushstring(state, CONTEXT_KEY);
  lua_gettable(state, LUA_REGISTRYINDEX);
  cairo_t* context = lua_touserdata(state, -1);
  lua_pop(state, 1);

  cairo_line_to(context, x, y);
  return 0;
}

int cairomoon_rel_moveto(lua_State* state)
{
  double x = lua_tonumber(state, 1);
  double y = lua_tonumber(state, 2);
  lua_pop(state, 2);
  
  lua_pushstring(state, CONTEXT_KEY);
  lua_gettable(state, LUA_REGISTRYINDEX);
  cairo_t* context = lua_touserdata(state, -1);
  lua_pop(state, 1);

  cairo_rel_move_to(context, x, y);
  return 0;
}

int cairomoon_rel_lineto(lua_State* state)
{
  double x = lua_tonumber(state, 1);
  double y = lua_tonumber(state, 2);
  lua_pop(state, 2);
  
  lua_pushstring(state, CONTEXT_KEY);
  lua_gettable(state, LUA_REGISTRYINDEX);
  cairo_t* context = lua_touserdata(state, -1);
  lua_pop(state, 1);

  cairo_rel_line_to(context, x, y);
  return 0;
}

int cairomoon_close(lua_State* state)
{
  lua_pushstring(state, CONTEXT_KEY);
  lua_gettable(state, LUA_REGISTRYINDEX);
  cairo_t* context = lua_touserdata(state, -1);
  lua_pop(state, 1);
  
  cairo_close_path(context);
  return 0;
}

/*
 * lua-cairo glue ends here.
 */

typedef enum RetStat_
  {
    OK,
    ERR_IO,
    ERR_PARAM,
    ERR_LUA_COLLECT,
    ERR_LUA_EXEC,
    ERR_STACK_COUNT
  }RetStat;

lua_State* prepare_vm()
{
  lua_State* state = luaL_newstate();

  lua_pushcfunction(state, cairomoon_moveto);
  lua_setglobal(state, "moveTo");
  lua_pushcfunction(state, cairomoon_lineto);
  lua_setglobal(state, "lineTo");
  lua_pushcfunction(state, cairomoon_close);
  lua_setglobal(state, "close");
  lua_pushcfunction(state, cairomoon_rel_moveto);
  lua_setglobal(state, "relMoveTo");
  lua_pushcfunction(state, cairomoon_rel_lineto);
  lua_setglobal(state, "relLineTo");

  return state;
}

RetStat draw_according_to_lua(char* codePath, char** errorMsg)
{
  /*
   * create lua VM with drawing functions
   */
  lua_State* state = prepare_vm();
  *errorMsg = NULL;
  int stackCount = lua_gettop(state);

  /*
   * run the code for the function and target pdf location
   */
  if(luaL_dofile(state, codePath) == true)
    {
      /*error in code*/
      *errorMsg = strdup(lua_tostring(state, -1));
      lua_close(state);
      return ERR_LUA_COLLECT;
    }
  
  if (lua_gettop(state) != stackCount) return ERR_STACK_COUNT;

  /*
   * retrieve output path of pdf (outputPath), fill, stroke
   * check for drawing function (drawPath)
   */
  lua_getglobal(state, "outputPath");
  const char* fileName_ = lua_tostring(state, -1);
  char* fileName;
  bool stroke = false;
  bool fill = false;
  if (fileName_ == NULL)
    {
      lua_close(state);
      return ERR_PARAM;
    }
  fileName = strdup(fileName_);
  
  lua_getglobal(state, "stroke");
  if(!lua_isboolean(state, -1))
    {
      lua_close(state);
      return ERR_PARAM;
    }
  stroke = lua_toboolean(state, -1);

  lua_getglobal(state, "fill");
  if(!lua_isboolean(state, -1))
    {
      lua_close(state);
      return ERR_PARAM;
    }
  fill = lua_toboolean(state, -1);

  lua_getglobal(state, "drawPath");
  if (!lua_isfunction(state, -1))
    {
      lua_close(state);
      return ERR_PARAM;
    }
  lua_pop(state, 4);

  if (lua_gettop(state) != stackCount) return ERR_STACK_COUNT;
  
  /*
   * boundary test
   */
  cairo_surface_t* s = cairo_pdf_surface_create_for_stream(NULL, NULL, 1, 1);
  cairo_t* context = cairo_create(s);
  
  lua_pushstring(state, CONTEXT_KEY);
  lua_pushlightuserdata(state, context);
  lua_settable(state, LUA_REGISTRYINDEX);

  lua_getglobal(state, "drawPath");
  if(lua_pcall(state, 0, 0, 0))
    {
      *errorMsg = strdup(lua_tostring(state, -1));
      lua_close(state);
      return ERR_LUA_EXEC;
    }

  if (lua_gettop(state) != stackCount) return ERR_STACK_COUNT;
  
  double x1, x2, y1, y2;
  cairo_path_t* path = cairo_copy_path(context);
  cairo_path_extents(context, &x1, &y1, &x2, &y2);
  cairo_destroy(context);
  cairo_surface_destroy(s);

  /*
   * actuall draw
   */
  s = cairo_pdf_surface_create(fileName, x2 - x1 + 10, y2 - y1 + 10);
  context = cairo_create(s);
  
  cairo_translate(context, - x1 + 5, - y1 + 5);
  cairo_append_path(context, path);

  if (fill)
    {
      cairo_fill(context);
    }
  else if(stroke && fill)
    {
      cairo_stroke_preserve(context);
      cairo_fill(context);
    }
  else if (stroke)
    {
      cairo_stroke(context);
    }

  cairo_destroy(context);
  cairo_surface_destroy(s);

  /*
   * cleanup
   */
  lua_close(state);
  if (lua_gettop(state) != stackCount) return ERR_STACK_COUNT;
  return OK;
}

int main(int argc, char** argv)
{
  for(int i = 1; i < argc; i++)
    {
      /* test file existence */
      FILE* fp = fopen(argv[i], "r");
      if (fp == NULL)
        {
          fprintf(stdout, "Item '%s' is not a lua code.\n", argv[i]);
          continue;
        }
      fclose(fp);

      /* run the file */
      char* error = NULL;
      RetStat state = draw_according_to_lua(argv[i], &error);
      switch(state)
        {
        case ERR_IO:
          fprintf(stderr, "ERROR when doing IO\n");
          break;
        case ERR_PARAM:
          fprintf(stderr, "ERROR when loading code: some required data are missing\n");
          break;
        case ERR_LUA_COLLECT:
          fprintf(stderr, "ERROR when loading code: parsing error\n");
          fprintf(stderr, "%s", error);
          break;
        case ERR_STACK_COUNT:
          fprintf(stderr, "ERROR on stack count state\n");
          break;
        case ERR_LUA_EXEC:
          fprintf(stderr, "ERROR when running drawPath:\n");
          fprintf(stderr, "%s", error);
          break;
        default:
          break;
        }
      if (error != NULL)
        free(error);
    }
  return 0;
}
