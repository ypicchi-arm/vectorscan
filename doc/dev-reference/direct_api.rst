.. _direct_api:

####################
Direct API extension
####################

Even though pure literal searches are fast, there is still some overhead.
In tight loops where both the pattern and the data are small (say, a
4-character pattern with a 32-character data buffer), this overhead can
become noticeable. In such cases, the functions provided by the Direct API
offer a minimal-overhead alternative, at the cost of a reduced set of
functionality.

Each type of call is designed for a specific pattern type:
    - Strings
    - Pairs of two characters
    - Single characters
Each type comes in a ``single`` search and ``set`` search variant, depending
on whether you need to search for one or multiple patterns.

For each case, compile, search, and free functions are provided.

All search functions are case-sensitive.

The single string search has an additional specialization based on the length
of the pattern. If the pattern is "short", ie shorter than or equal to
:c:member:`HS_SHORT_PATTERN_THRESHOLD` characters—then
:c:func:`hs_compile_short_literal_search` may be used instead.
