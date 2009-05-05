while gets() do
    if $_ =~ /\A( +)fcs_move_set_type *\( *temp_move *, *(FCS_MOVE_TYPE\w+) *\) *;/
        indent = $1
        type = $2
        src = 0
        dest = 0
        num_cards = 0
        do_loop = true
        while do_loop and gets() do
            if $_ =~ /fcs_move_set_src_\w+ *\( *temp_move *, *(.*?) *\) *;/
                src = $1
            elsif $_ =~ /(?:fcs_move_set_dest_\w+|fcs_move_set_foundation) *\( *temp_move *, *(.*?) *\) *;/
                dest = $1
            elsif $_ =~ /fcs_move_set_num_cards_in_seq *\( *temp_move *, *(.*?) *\);/
                num_cards = $1
            elsif $_ =~ /fcs_move_stack_push *\( *moves *, *temp_move *\) *;/
                puts <<"EOF"
#{indent}fcs_move_stack_push_params(moves,
#{indent}    #{type},
#{indent}    /* src = */            #{src},
#{indent}    /* dest = */           #{dest},
#{indent}    /* num_cards = */      #{num_cards}
#{indent});
EOF
                do_loop = false
            end
        end
    else
        puts $_
    end
end
