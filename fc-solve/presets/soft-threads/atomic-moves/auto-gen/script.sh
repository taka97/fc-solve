freecell-solver-range-parallel-solve 1 32000 100 \
--method soft-dfs -to 0123456789 -step 500 --st-name 1 -nst \
--method soft-dfs -to 0123467 -step 500 --st-name 2 -nst \
--method random-dfs -seed 2 -to 0[01][23456789] -step 500 --st-name 3 -nst \
--method random-dfs -seed 1 -to 0[0123456789] -step 500 --st-name 4 -nst \
--method random-dfs -seed 3 -to 0[01][23467] -step 500 --st-name 5 -nst \
--method random-dfs -seed 4 -to 0[0123467] -step 500 --st-name 9 -nst \
--method random-dfs -to [01][23456789] -seed 8 -step 500 --st-name 10 -nst \
--method random-dfs -to [01][23456789] -seed 268 -step 500 --st-name 12 \
--prelude "350@2,350@5,350@9,350@12,350@2,350@10,350@3,350@9,350@5,350@4,350@2,350@5,350@10,350@3,350@2,350@4,350@5,350@2,700@5,350@12,1050@9,350@10,350@2,350@10,1050@5,350@10,350@12,700@2,700@1,700@4,700@12,1400@2,700@9,350@10,700@3,700@4,700@2,5250@5,1050@10,1750@3,1400@1,1400@10,5600@1,4900@12,23450@2"


