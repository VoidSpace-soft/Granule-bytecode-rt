# Granule-bytecode-rt
a bytecode runtime made using C.
free to use in any way in any language.

commands -><br>
- 0x10: prints all bytes to stdout until 0x0 terminator
- -  0x12: any output in 0x10 starting with 0x12 will point to identifier of an intvar
- 0x11: define int variable with 32 bit value and 1 byte identifier
- 0x14: error
- 0x27-0x2A: int eq, not eq, more, less
- 0x2B-0x2C: str eq and not eq
- 0x25: clear stdout
- 0x21: print str variable
- 0x20: define str var (max 254 characters)
- 0x26: execute next code till terminator 0xEF if specified int var != 0
- 0x30: execute next code till terminator 0xEF if specified int var == 0
- 0x22: jump tp specific address in bytecode.
- 0x31: jump without logging jump addr.
- 0x32: clear callstack.
- 0x23: return to last jump addr.
- 0x24: read from stdin.
- 0x13: increment an intvar
- 0x33: decrement an intvar
