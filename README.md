The serializing counterpart to [json-parser](http://github.com/udp/json-parser).

As with json-parser: BSD licensed, _almost_ ANSI C89 apart from a single use of [snprintf](http://linux.die.net/man/3/snprintf).

Usage
-----

Quick example (docs coming soon):

    json_value * arr = json_array_new(0);
    json_array_push(arr, json_string_new("Hello world!"));
    json_array_push(arr, json_integer_new(128));

    char * buf = malloc(json_measure(arr));
    json_serialize(buf, arr);

    printf("%s\n", buf);

> [ "Hello world!", 128 ]

json-builder is fully interoperable with json-parser:

    char json[] = "[ 1, 2, 3 ]";

    json_settings settings = {};
    settings.value_extra = json_builder_extra;  /* space for json-builder state */

    char error[128];
    json_value * arr = json_parse_ex(&settings, json, strlen(json), error);

    /* Now serialize it again.
     */
    char * buf = malloc(json_measure(arr));
    json_serialize(buf, arr);

    printf("%s\n", buf);

> [ 1, 2, 3 ]

Note that values created by or modified by json-builder must be freed with
`json_builder_free` instead of `json_value_free`, otherwise the memory of the
builder state will be leaked.


Modes
-----

* `json_serialize_mode_multiline` — Generate multi-line JSON, for example:
```
[
  1,
  2,
  3
]
```

* `json_serialize_mode_single_line` — Generate JSON on a single line, for example:
```
[ 1, 2, 3 ]
```

* `json_serialize_mode_packed` — Generate JSON as tightly packed as possible, for example:
```
[1,2,3]
```


Options
-------

* `json_serialize_opt_CRLF` — use CR/LF (Windows) line endings

* `json_serialize_opt_pack_brackets` — do not leave spaces around brackets (e.g. `[ 1, 2 ]` becomes `[1, 2]`)

* `json_serialize_opt_no_space_after_comma` — do not leave spaces after commas

* `json_serialize_opt_no_space_after_colon` — do not leave spaces after colons (inside objects)

* `json_serialize_opt_use_tabs` — indent using tabs instead of spaces when in multi-line mode

* `indent_size` — the number of tabs or spaces to indent with in multi-line mode


