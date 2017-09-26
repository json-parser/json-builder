
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

#include <stdio.h>
#include <assert.h>
#include <string.h>

#include <vector>
using namespace std;

void test_file (const char * filename, int * num_failed);
void test_buf (char * buffer, size_t size, int * num_failed);
bool json_equal (json_value * a, json_value * b);

int main (int argc, char * argv [])
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

   return 0;
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

void test_buf (char * buffer, size_t size, int * num_failed)
{
   json_value * value, * value2;
   json_settings settings = { 0 };
   char error [128];

   settings.value_extra = json_builder_extra;

   if (! (value = json_parse_ex (&settings, buffer, size, error)))
   {
      /* json-parser failed!  That's not what we were supposed to be testing.
       */
      assert (0);
      return;
   }

   size_t measured = json_measure (value);
   printf ("measured len: %d\n", (int) measured);

   char * buf = (char *) malloc (measured);
   json_serialize (buf, value);

   size_t serialized = (int) strlen (buf) + 1;
   printf ("serialized len: %d\n", serialized);

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
   else if (!json_equal (value, value2))
   {
      printf ("Changed after re-parse\n");
      ++ *num_failed;
   }
   else
   {
      printf ("success\n");
   }

   json_value_free (value);
   json_value_free (value2);
}

bool json_equal (json_value * a, json_value * b)
{
    vector <json_value *> stack;

    stack.push_back (a);
    stack.push_back (b);

    while (stack.size () > 0)
    {
        json_value * rhs = stack.back ();
        stack.pop_back ();

        json_value * lhs = stack.back ();
        stack.pop_back ();

        if (lhs->type != rhs->type)
            return false;

        switch (lhs->type)
        {
            case json_none:
                break;

            case json_object:

                if (lhs->u.object.length != rhs->u.object.length)
                    return false;

                for (int i = 0; i < lhs->u.object.length; ++ i)
                {
                    if (lhs->u.object.values [i].name_length !=
                        rhs->u.object.values [i].name_length)
                    {
                        return false;
                    }

                    if (memcmp (lhs->u.object.values [i].name,
                                rhs->u.object.values [i].name,
                                lhs->u.object.values [i].name_length) != 0)
                    {
                        return false;
                    }

                    stack.push_back (lhs->u.object.values [i].value);
                    stack.push_back (rhs->u.object.values [i].value);
                }

                break;

            case json_array:

                if (lhs->u.array.length != rhs->u.array.length)
                    return false;

                for (int i = 0; i < lhs->u.array.length; ++ i)
                {
                    stack.push_back (lhs->u.array.values [i]);
                    stack.push_back (rhs->u.array.values [i]);
                }

                break;

            case json_integer:

                if (lhs->u.integer != rhs->u.integer)
                    return false;

                break;

            case json_double:

                /* TODO */

                /* if (lhs->u.dbl != rhs->u.dbl)
                    return false; */

                break;

            case json_string:

                if (lhs->u.string.length != rhs->u.string.length)
                    return false;

                if (memcmp (lhs->u.string.ptr,
                            rhs->u.string.ptr,
                            lhs->u.string.length) != 0)
                {
                    return false;
                }

                break;

            case json_boolean:

                if (lhs->u.boolean != rhs->u.boolean)
                    return false;

                break;

            case json_null:
                break;
        };
    }

    return true;
}

