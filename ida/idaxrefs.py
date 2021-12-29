import idautils

def xrefsrange(start, stop):
    xrefs = {}
    types = {2, 4, 5}
    for addr in range(start, stop, 1):
        for ref in idautils.XrefsTo(addr):
            insn = idautils.DecodeInstruction(ref.frm)
            for op in [insn.Op1, insn.Op2, insn.Op3, insn.Op4, insn.Op5, insn.Op6]:
                if op.type in types:
                    xrefs.setdefault(addr-start, []).append(ref.frm + op.offb)
    return xrefs

def pprint_xrefs(xr, name):
    print(f'std::map<DWORD, std::vector<DWORD>> {name} = {{')
    for offset, xrefs in xr.items():
        print(f'\t{{ 0x{offset:x}, {{', end='')
        print(', '.join(f'0x{ref:08x}' for ref in xrefs), end='')
        print('}},')
    print('};')


pprint_xrefs(xrefsrange(0x7C1078, 0x7C1178), 'xrefs_start')
pprint_xrefs(xrefsrange(0x857708, 0x8577FE), 'xrefs_end')
