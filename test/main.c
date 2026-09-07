
/* vim: set et ts=3 sw=3 sts=3 ft=c:
 *
 * Copyright (C) 2014 James McLaughlin.  All rights reserved.
 * https://github.com/udp/json-builder
 *
 * Redistribution and use in source and binary forms, with or without
 * modification, are permitted provided that the following conditions
 * are met:
 *
 * 1. Redistributions of source code must retain the above copyright
 *   notice, this list of conditions and the following disclaimer.
 *
 * 2. Redistributions in binary form must reproduce the above copyright
 *   notice, this list of conditions and the following disclaimer in the
 *   documentation and/or other materials provided with the distribution.
 *
 * THIS SOFTWARE IS PROVIDED BY THE AUTHOR AND CONTRIBUTORS ``AS IS'' AND
 * ANY EXPRESS OR IMPLIED WARRANTIES, INCLUDING, BUT NOT LIMITED TO, THE
 * IMPLIED WARRANTIES OF MERCHANTABILITY AND FITNESS FOR A PARTICULAR PURPOSE
 * ARE DISCLAIMED.  IN NO EVENT SHALL THE AUTHOR OR CONTRIBUTORS BE LIABLE
 * FOR ANY DIRECT, INDIRECT, INCIDENTAL, SPECIAL, EXEMPLARY, OR CONSEQUENTIAL
 * DAMAGES (INCLUDING, BUT NOT LIMITED TO, PROCUREMENT OF SUBSTITUTE GOODS
 * OR SERVICES; LOSS OF USE, DATA, OR PROFITS; OR BUSINESS INTERRUPTION)
 * HOWEVER CAUSED AND ON ANY THEORY OF LIABILITY, WHETHER IN CONTRACT, STRICT
 * LIABILITY, OR TORT (INCLUDING NEGLIGENCE OR OTHERWISE) ARISING IN ANY WAY
 * OUT OF THE USE OF THIS SOFTWARE, EVEN IF ADVISED OF THE POSSIBILITY OF
 * SUCH DAMAGE.
 */

#include "../json-builder.h"

#include <assert.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

void test_file (const char * filename, int * num_failed);
void test_buf (const char * buffer, size_t size, int * num_failed);
int json_equal (const json_value * a, const json_value * b);

int main (void)
{
   int num_failed = 0;

   test_file ("valid-0000.json", &num_failed);
   test_file ("valid-0001.json", &num_failed);
   test_file ("valid-0002.json", &num_failed);
   test_file ("valid-0003.json", &num_failed);
   test_file ("valid-0004.json", &num_failed);
   test_file ("valid-0005.json", &num_failed);
   test_file ("valid-0006.json", &num_failed);
   test_file ("valid-0007.json", &num_failed);
   test_file ("valid-0008.json", &num_failed);
   test_file ("valid-0009.json", &num_failed);
   test_file ("valid-0010.json", &num_failed);
   test_file ("valid-0011.json", &num_failed);
   test_file ("valid-0012.json", &num_failed); 

   printf ("Total failed tests: %d\n", num_failed);

   return num_failed ? EXIT_FAILURE : EXIT_SUCCESS;
}

void test_file (const char * filename, int * num_failed)
{
   FILE * file;
   size_t size;
   char * buffer;

   printf ("Running test: %s\n", filename);

   if (! (file = fopen (filename, "r")))
   {
      fprintf (stderr, "  Error opening file\n");
      return;
   }

   fseek (file, 0, SEEK_END);
   size = ftell (file);
   fseek (file, 0, SEEK_SET);

   if (! (buffer = (char *) malloc (size)))
   {
      fprintf (stderr, "  Error allocating memory\n");

      fclose (file);
      return;
   }

   if (fread (buffer, 1, size, file) != size)
   {
      fprintf (stderr, "  Error reading file\n");

      fclose (file);
      return;
   }

   fclose (file);

   test_buf (buffer, size, num_failed);

   free (buffer);
}

void test_buf (const char * buffer, size_t size, int * num_failed)
{
   json_value * value = 0, * value2 = 0;
   json_settings settings = { 0 };
   char error [json_error_max];
   size_t measured = 0;
   char * buf = 0;
   size_t serialized = 0;
   int equality = 0;

   settings.value_extra = json_builder_extra;

   if (! (value = json_parse_ex (&settings, buffer, size, error)))
   {
      /* json-parser failed!  That's not what we were supposed to be testing.
       */
      assert (0);
      return;
   }

   measured = json_measure (value);
   printf ("measured len: %d\n", (int) measured);

   buf = (char *) malloc (measured);
   if (!buf)
   {
      fprintf (stderr, "  Error allocating memory\n");
      json_value_free (value);
      return;
   }
   json_serialize (buf, value);

   serialized = strlen (buf) + 1;
   printf ("serialized len: %lu\n", (unsigned long) serialized);

   printf ("serialized:\n%s\n", buf);

   if (serialized > measured)
   {
      printf ("Serialized more than measured\n");
      ++ *num_failed;
   }
   else if (! (value2 = json_parse_ex (&settings, buf, strlen(buf), error)))
   {
      printf ("Failed to re-parse: %s\n", error);
      ++ *num_failed;
   }
   else
   {
      equality = json_equal (value, value2);
      switch (equality)
      {
         case 1:
            printf ("success\n");
            break;
         case 0:
            printf ("Changed after re-parse\n");
            ++ *num_failed;
            break;
         case -1:
            printf ("Memory allocation failure\n");
            ++ *num_failed;
            break;
         case -2:
            printf ("Child objects measured larger than their parents\n");
            ++ *num_failed;
            break;
         default:
            printf ("Memory corruption\n");
            ++ *num_failed;
            break;
      }
   }

   free(buf);
   json_value_free (value);
   json_value_free (value2);
}

int json_equal (const json_value * a, const json_value * b)
{
   size_t stack_size = 0;
   size_t stack_capacity = 0;
   const json_value ** stack = 0;
   unsigned int i = 0;

   #define stack_append(p) \
      if (stack_size >= stack_capacity)\
      {\
         const json_value ** stack_realloc = (const json_value **) realloc(stack, sizeof(const json_value *) * (stack_capacity += 10));\
         if (!stack_realloc)\
         {\
            goto allocation_failure;\
         }\
         stack = stack_realloc;\
      }\
      stack [stack_size] = (p);\
      ++stack_size;

   stack_append (a);
   stack_append (b);

   while (stack_size > 0)
   {
      const json_value * rhs = stack [--stack_size];
      const json_value * lhs = stack [--stack_size];
      size_t measured_rhs = 0;
      size_t measured_lhs = 0;

      if (lhs->type != rhs->type)
         goto unequal;

      measured_rhs = json_measure (rhs);
      measured_lhs = json_measure (lhs);

      switch (lhs->type)
      {
         case json_none:
            break;

         case json_object:

            if (lhs->u.object.length != rhs->u.object.length)
               goto unequal;

            for (i = 0; i < lhs->u.object.length; ++ i)
            {
               size_t measured_rhs_sub = 0;
               size_t measured_lhs_sub = 0;
               if (lhs->u.object.values [i].name_length !=
                   rhs->u.object.values [i].name_length)
               {
                  goto unequal;
               }

               if (memcmp (lhs->u.object.values [i].name,
                           rhs->u.object.values [i].name,
                           lhs->u.object.values [i].name_length) != 0)
               {
                  goto unequal;
               }

               measured_rhs_sub = json_measure (rhs->u.object.values [i].value);
               measured_lhs_sub = json_measure (lhs->u.object.values [i].value);
               if (measured_lhs_sub >= measured_lhs || measured_rhs_sub >= measured_rhs)
               {
                  goto measurement_failure;
               }

               stack_append (lhs->u.object.values [i].value);
               stack_append (rhs->u.object.values [i].value);
            }

            break;

         case json_array:

            if (lhs->u.array.length != rhs->u.array.length)
               goto unequal;

            for (i = 0; i < lhs->u.array.length; ++ i)
            {
               size_t measured_rhs_sub = json_measure (rhs->u.array.values [i]);
               size_t measured_lhs_sub = json_measure (lhs->u.array.values [i]);
               if (measured_lhs_sub >= measured_lhs || measured_rhs_sub >= measured_rhs)
               {
                  goto measurement_failure;
               }

               stack_append (lhs->u.array.values [i]);
               stack_append (rhs->u.array.values [i]);
            }

            break;

         case json_integer:

            if (lhs->u.integer != rhs->u.integer)
               goto unequal;

            break;

         case json_double:

            /* TODO */

            /* if (lhs->u.dbl != rhs->u.dbl)
               goto unequal; */

            break;

         case json_string:

            if (lhs->u.string.length != rhs->u.string.length)
               goto unequal;

            if (memcmp (lhs->u.string.ptr,
                        rhs->u.string.ptr,
                        lhs->u.string.length) != 0)
            {
               goto unequal;
            }

            break;

         case json_boolean:

            if (lhs->u.boolean != rhs->u.boolean)
               goto unequal;

            break;

         case json_null:
            break;
      }
   }

/*equal:*/
   free(stack);
   return 1;

unequal:
   free(stack);
   return 0;

allocation_failure:
   free(stack);
   return -1;

measurement_failure:
   free(stack);
   return -2;
}
