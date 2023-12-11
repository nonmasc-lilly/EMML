INCLUDE "bool.mc"

LABEL end
    EXIT 0

LABEL _start START
    DECLARE SHORT C
    SET C 10
    REGISTER A = C 10
    JUMP end IF C
END
