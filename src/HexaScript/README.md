# What is this?

Very basic scripting language to evaluate expressions like

```
:quit
:set little-endian
:set filetype=elf
:mark "something"
```

Multi line expressions are not supported. With the exception of `:set`, each
line could be taught as function calls.

So the example aboe would roughly map to the following C++ snippet.

```
quit();
little_endian = true;
filetype = "elf";
mark("something");
```

There is no RTTI, therefore functions and variables must be registered.
