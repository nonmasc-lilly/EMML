INCLUDE "bool.mc"

LABEL end START
    EXIT 2
END

LABEL _start START
    ALLOC 10 INT HELLO
    SET 0 HELLO 0
    SET 1 HELLO 23
    JUMP end IF = .GET 0 HELLO. %true \ false \
    EXIT .GET 1 HELLO.
END

