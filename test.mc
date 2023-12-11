INCLUDE "bool.mc"

LABEL end
    EXIT 0

LABEL _start START
    DECLARE SHORT C
    SET C 20
    REGISTER A = ./ C 2. 10
    JUMP end IF C
END

