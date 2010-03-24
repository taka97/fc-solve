#!/bin/sh

# The "Cool Jives" Preset
# An autogenerated preset.

freecell-solver-range-parallel-solve 1 32000 1 \
--method soft-dfs -to 0123456789 -step 500 --st-name 1 -nst \
--method soft-dfs -to 0123467 -step 500 --st-name 2 -nst \
--method random-dfs -seed 2 -to 0[01][23456789] -step 500 --st-name 3 -nst \
--method random-dfs -seed 1 -to 0[0123456789] -step 500 --st-name 4 -nst \
--method random-dfs -seed 3 -to 0[01][23467] -step 500 --st-name 5 -nst \
--method random-dfs -seed 4 -to 0[0123467] -step 500 --st-name 9 -nst \
--method random-dfs -to [01][23456789] -seed 8 -step 500 --st-name 10 -nst \
--method random-dfs -to [01][23456789] -seed 268 -step 500 --st-name 12 -nst \
--method a-star -asw 0.2,0.3,0.5,0,0 -step 500 --st-name 16 -nst \
--method a-star -to 0123467 -asw 0.5,0,0.3,0,0 -step 500 --st-name 18 -nst \
--method soft-dfs -to 0126394875 -step 500 --st-name 19 \
--prelude "350@2,350@5,350@9,350@12,350@2,350@10,350@3,350@9,350@5,350@18,350@2,350@5,350@4,350@10,350@4,350@12,1050@9,700@18,350@10,350@5,350@2,350@10,1050@16,350@2,700@4,350@10,1050@2,1400@3,350@18,1750@5,350@16,350@18,700@4,1050@12,2450@5,1400@18,1050@2,1400@10,6300@1,4900@12,8050@18"


