file(GLOB_RECURSE WORKHORSE_CORE_C
    export/*.c
    import/*.c
    workhorse/**/*.c
    workhorse/*.c
    plugin/*.c
    *.c
)

set(WORKHORSE_CORE_OBJS
    ${WORKHORSE_CORE_C}
    PARENT_SCOPE
)
