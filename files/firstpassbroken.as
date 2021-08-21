.extern         vall
.entry          vall
bgt:           add, $3       $5,    $9
LOOP:           ori $9 ,-5, $2
                la vall
                jm Next
Next: Next:      move $20,$4,$6
                bg $4 ,$2,END
                la K
                sw, $0,4
                bne $50,$9,LOOP
                call vall
                jmp $4
.extern LE
LE:             st
STR:            .asciz aBcd"
LIST:           .db   27056 -9
                .dh 27056
K:              .dw 31,-12
.extern          K