freecell-solver-range-parallel-solve 1 32000 1 \
--method soft-dfs -to 0123456789 -step 500 --st-name 1 -nst \
--method random-dfs -seed 2 -to 0[01][23456789] -step 500 --st-name 3 -nst \
--method random-dfs -seed 1 -to 0[0123456789] -step 500 --st-name 4 -nst \
--method random-dfs -seed 3 -to 0[01][23467] -step 500 --st-name 5 -nst \
--method random-dfs -seed 4 -to 0[0123467] -step 500 --st-name 9 -nst \
--method random-dfs -to [01][23456789] -seed 8 -step 500 --st-name 10 -nst \
--method a-star -asw 0.2,0.8,0,0,0 -step 500 --st-name 11 -nst \
--method random-dfs -to [01][23456789] -seed 268 -step 500 --st-name 12 -nst \
--method random-dfs -to [0123456789] -seed 142 -step 500 --st-name 15 -nst \
--method random-dfs -to [01][23456789] -seed 5 -step 500 --st-name 17 -nst \
--method a-star -to 0123467 -asw 0.5,0,0.3,0,0 -step 500 --st-name 18 -nst \
--method random-dfs -seed 105 -step 500 --st-name 20 -nst \
--method a-star -asw 0.5,0,0.5,0,0 -step 500 --st-name 21 -nst \
--method soft-dfs -to 013[2456789] -step 500 --st-name 22 -nst \
--method random-dfs -to 0123467 -dto 16,0[123467] -step 500 --st-name 24 \
--prelude "387@24,367@5,431@9,357@10,344@24,351@3,345@5,345@12,350@9,336@18,607@4,348@24,853@17,437@5,358@20,300@11,350@20,347@15,453@10,569@20,219@22,226@21,123@12,700@24,328@9,635@10,933@18,629@21,271@18,243@9,166@12,350@5,456@1,301@11,1497@12,920@15,350@24,1400@10,3813@24,4453@12,4712@17,5534@1"
