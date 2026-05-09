file(GLOB PLUGINS_C
    *.c
)

file(GLOB PLUGINS_ASM
    *.S
)

set(PLUGINS_OBJS
    ${PLUGINS_C}
    ${PLUGINS_ASM}
    PARENT_SCOPE
)