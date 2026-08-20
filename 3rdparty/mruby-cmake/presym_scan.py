#!/usr/bin/env python3
"""
presym_scan.py - Scan mruby preprocessed .pi files for presym markers,
extract all unique symbol names, and generate output files.

Equivalent to Presym in mruby's presym.rb.

Modes:
  list    (default) : scan .pi files → presym.list
  headers           : read presym.list → id.h + table.h

Usage:
  python3 presym_scan.py --mode list --output <list_file> <pi_files...>
  python3 presym_scan.py --mode headers --input-list <list_file> --output-dir <dir>
"""

import re
import sys
import os
import argparse

# Force UTF-8 output on Windows to avoid charmap UnicodeEncodeError.
if hasattr(sys.stdout, "reconfigure"):
    sys.stdout.reconfigure(encoding="utf-8", errors="replace")
if hasattr(sys.stderr, "reconfigure"):
    sys.stderr.reconfigure(encoding="utf-8", errors="replace")


# ─── Constants (mirrors presym.rb) ───────────────────────────────────────────

OPERATORS = {
    "!": "not",   "%": "mod",   "&": "and",   "*": "mul",
    "+": "add",   "-": "sub",   "/": "div",   "<": "lt",
    ">": "gt",    "^": "xor",   "`": "tick",  "|": "or",
    "~": "neg",   "!=": "neq",  "!~": "nmatch","&&": "andand",
    "**": "pow",  "+@": "plus", "-@": "minus","<<": "lshift",
    "<=": "le",   "==": "eq",   "=~": "match", ">=": "ge",
    ">>": "rshift","[]": "aref", "||": "oror", "<=>": "cmp",
    "===": "eqq", "[]=": "aset",
}

# [prefix, suffix] -> [macro_prefix, macro_suffix]
SYMBOL_TO_MACRO = [
    (("$",  ""),  ("GV", "")),
    (("@@", ""),  ("CV", "")),
    (("@",  ""),  ("IV", "")),
    (("",   "!"), ("",   "_B")),
    (("",   "?"), ("",   "_Q")),
    (("",   "="), ("",   "_E")),
    (("",   ""),  ("",   "")),
]

# C escape sequence maps
ESCAPE_MAP = {
    "a": "\a", "b": "\b", "e": "\033", "f": "\f",
    "n": "\n", "r": "\r", "t": "\t", "v": "\v",
}
ESCAPE_REVERSE = {v: k for k, v in ESCAPE_MAP.items()}

# Regex: match a C string literal "..." (with escape sequences)
C_STR_LITERAL_RE = re.compile(r'"(?:[^\\"]|\\.)*"')

# Regex: match <@! ... !@> blocks
PRESYM_BLOCK_RE = re.compile(r'<@!\s*(.*?)\s*!@>', re.DOTALL)


# ─── C escape sequence unescaping ────────────────────────────────────────────

def unescape_c_string(s: str) -> str:
    """Unescape a C-style string content (without surrounding quotes)."""
    result = []
    i = 0
    while i < len(s):
        if s[i] == "\\" and i + 1 < len(s):
            i += 1
            c = s[i]
            if c in ESCAPE_MAP:
                result.append(ESCAPE_MAP[c])
            elif c == "x":
                hex_str = ""
                j = i + 1
                while j < len(s) and j - i <= 2 and s[j] in "0123456789ABCDEFabcdef":
                    hex_str += s[j]
                    j += 1
                if hex_str:
                    result.append(chr(int(hex_str, 16)))
                i = j - 1
            elif c in "01234567":
                octal = c
                j = i + 1
                while j < len(s) and j - i < 3 and s[j] in "01234567":
                    octal += s[j]
                    j += 1
                result.append(chr(int(octal, 8)))
                i = j - 1
            else:
                result.append(c)
        else:
            result.append(s[i])
        i += 1
    return "".join(result)


# ─── Scanning (.pi → symbols) ────────────────────────────────────────────────

def scan_pi_file(filepath: str, verbose: bool = False) -> list:
    """Scan a single .pi file, return list of symbol strings."""
    symbols = []
    try:
        with open(filepath, "r", encoding="utf-8", errors="surrogateescape") as f:
            content = f.read()
    except OSError as e:
        print(f"Warning: cannot read {filepath}: {e}", file=sys.stderr)
        return symbols

    marker_count = 0
    for block_match in PRESYM_BLOCK_RE.finditer(content):
        marker_count += 1
        part = block_match.group(1)
        literals = C_STR_LITERAL_RE.findall(part)
        if not literals:
            continue
        literals = [l[1:-1] for l in literals]
        symbols.append("".join(unescape_c_string(l) for l in literals))

    if verbose:
        base = os.path.basename(filepath)
        print(f"  {base}: {marker_count} markers → {len(symbols)} symbols")

    return symbols


def scan_all(pi_files: list, verbose: bool = False) -> list:
    """Scan all .pi files, deduplicate, sort."""
    presym_set = set()
    for pf in pi_files:
        for sym in scan_pi_file(pf, verbose=verbose):
            presym_set.add(sym)
    result = sorted(presym_set, key=lambda s: (len(s.encode("utf-8")), s))
    if verbose:
        q_count = sum(1 for s in result if s.endswith("?"))
        b_count = sum(1 for s in result if s.endswith("!"))
        e_count = sum(1 for s in result if s.endswith("="))
        iv_count = sum(1 for s in result if s.startswith("@") and not s.startswith("@@"))
        print(f"Total: {len(result)} unique symbols (plain + {q_count}? {b_count}! {e_count}= {iv_count}@)")
    return result


# ─── Header generation (presym.list → id.h + table.h) ────────────────────────

def load_presym_list(filepath: str) -> list:
    """Load a sorted presym list file (one symbol per line)."""
    syms = []
    with open(filepath, "r", encoding="utf-8") as f:
        for line in f:
            line = line.strip()
            if line:
                syms.append(line)
    return syms


def write_id_header(presyms: list, output_dir: str):
    """Generate id.h: enum mruby_presym (mirrors presym.rb write_id_header)."""
    # Collect non-empty prefixes / suffixes
    all_prefixes = sorted(
        set(k[0] for k, _ in SYMBOL_TO_MACRO if k[0]),
        key=len, reverse=True
    )
    all_suffixes = sorted(
        set(k[1] for k, _ in SYMBOL_TO_MACRO if k[1]),
        key=len, reverse=True
    )

    prefix_re = "|".join(re.escape(p) for p in all_prefixes)
    suffix_re = "|".join(re.escape(s) for s in all_suffixes)

    # Build pattern: name first char must be [^\W\d] (== Ruby [\w&&\D])
    if prefix_re and suffix_re:
        sym_pattern = re.compile(rf"^({prefix_re})?([^\W\d]\w*)({suffix_re})?$")
    elif prefix_re:
        sym_pattern = re.compile(rf"^({prefix_re})?([^\W\d]\w*)$")
    elif suffix_re:
        sym_pattern = re.compile(rf"^([^\W\d]\w*)({suffix_re})?$")
    else:
        sym_pattern = re.compile(r"^([^\W\d]\w*)$")

    macro_map = {k: v for k, v in SYMBOL_TO_MACRO}

    filepath = os.path.join(output_dir, "id.h")
    os.makedirs(output_dir, exist_ok=True)

    with open(filepath, "w", encoding="utf-8", newline="\n") as f:
        f.write("enum mruby_presym {\n")
        unmatched = []
        for num, sym in enumerate(presyms, 1):
            m = sym_pattern.match(sym)
            if m:
                pref = m.group(1) or ""
                name = m.group(2) or ""
                suff = m.group(3) or ""
                key = (pref, suff)
                if key in macro_map:
                    affix_pref, affix_suff = macro_map[key]
                    # Ruby: affixes * 'SYM'  -> joins [affix_pref, affix_suff] with "SYM"
                    # e.g. ["", "_Q"] * "SYM" = "SYM_Q"
                    #      ["GV", ""] * "SYM" = "GVSYM"
                    macro_prefix = "SYM".join([affix_pref, affix_suff])
                    f.write(f"  MRB_{macro_prefix}__{name} = {num},\n")
                    continue

            if sym in OPERATORS:
                f.write(f"  MRB_OPSYM__{OPERATORS[sym]} = {num},\n")
                continue

            unmatched.append(sym)
            print(f"Warning: unmatched symbol '{sym}'", file=sys.stderr)
            f.write(f"  /* UNMATCHED: {sym} */\n")

        f.write("};\n")
        f.write("\n")
        f.write(f"#define MRB_PRESYM_MAX {len(presyms)}\n")

    if unmatched:
        print(f"Warning: {len(unmatched)} unmatched symbols", file=sys.stderr)
    q_syms = [s for s in presyms if s.endswith("?")][:5]
    if q_syms:
        print(f"  Sample '?' symbols: {q_syms}")
    e_syms = [s for s in presyms if s.endswith("=") and s not in OPERATORS][:5]
    if e_syms:
        print(f"  Sample 'xxx=' symbols: {e_syms}")
    b_syms = [s for s in presyms if s.endswith("!") and s not in OPERATORS][:5]
    if b_syms:
        print(f"  Sample 'xxx!' symbols: {b_syms}")

    print(f"Generated: {filepath} ({len(presyms)} symbols)")


def write_table_header(presyms: list, output_dir: str):
    """Generate table.h: presym_length_table[] and presym_name_table[]."""
    filepath = os.path.join(output_dir, "table.h")
    os.makedirs(output_dir, exist_ok=True)

    with open(filepath, "w", encoding="utf-8", newline="\n") as f:
        f.write("static const uint16_t presym_length_table[] = {\n")
        for sym in presyms:
            byte_len = len(sym.encode("utf-8"))
            f.write(f"  {byte_len},\t/* {sym} */\n")
        f.write("};\n")
        f.write("\n")
        f.write("static const char * const presym_name_table[] = {\n")
        for sym in presyms:
            c_str = ""
            for ch in sym:
                if ch in ESCAPE_REVERSE:
                    c_str += "\\" + ESCAPE_REVERSE[ch]
                elif ch == '"':
                    c_str += '\\"'
                elif ch == '\\':
                    c_str += '\\\\'
                elif '\x01' <= ch <= '\x1f' or ord(ch) > 0x7f:
                    for b in ch.encode("utf-8"):
                        c_str += f'\\x{b:02x}'
                else:
                    c_str += ch
            f.write(f'  "{c_str}",\n')
        f.write("};\n")

    print(f"Generated: {filepath}")


# ─── Main ────────────────────────────────────────────────────────────────────

def main():
    parser = argparse.ArgumentParser(
        description="mruby presym scanner and header generator"
    )
    sub = parser.add_subparsers(dest="mode", required=True)

    # Subcommand: list (scan .pi → presym.list)
    p_list = sub.add_parser("list", help="Scan .pi files → presym.list")
    p_list.add_argument("--output", "-o", required=True,
                        help="Output list file (one symbol per line)")
    p_list.add_argument("--verbose", "-v", action="store_true",
                        help="Print per-file symbol counts")
    p_list.add_argument("pi_files", nargs="*",
                        help="Preprocessed .pi files to scan")

    # Subcommand: headers (presym.list → id.h + table.h)
    p_hdr = sub.add_parser("headers", help="Generate id.h + table.h from presym.list")
    p_hdr.add_argument("--input-list", required=True,
                        help="Input presym list file")
    p_hdr.add_argument("--output-dir", required=True,
                        help="Output directory for id.h and table.h")

    # Subcommand: debug-pi (dump markers from a single .pi file)
    p_dbg = sub.add_parser("debug-pi", help="Dump <@! ... !@> markers from a .pi file")
    p_dbg.add_argument("pi_file", help="A single .pi file to inspect")
    p_dbg.add_argument("--limit", type=int, default=20,
                        help="Max markers to print (default: 20)")

    args = parser.parse_args()

    if args.mode == "list":
        if not args.pi_files:
            print("Error: no .pi files provided", file=sys.stderr)
            sys.exit(1)

        presyms = scan_all(args.pi_files, verbose=args.verbose)

        with open(args.output, "w", encoding="utf-8", newline="\n") as f:
            for sym in presyms:
                f.write(sym + "\n")

        print(f"Exported {len(presyms)} presyms to {args.output}")

    elif args.mode == "headers":
        presyms = load_presym_list(args.input_list)
        if not presyms:
            print("Error: empty or missing presym list", file=sys.stderr)
            sys.exit(1)

        write_id_header(presyms, args.output_dir)
        write_table_header(presyms, args.output_dir)

    elif args.mode == "debug-pi":
        with open(args.pi_file, "r", encoding="utf-8", errors="surrogateescape") as f:
            content = f.read()
        markers = list(PRESYM_BLOCK_RE.finditer(content))
        print(f"File: {args.pi_file}")
        print(f"Total markers found: {len(markers)}")
        print()
        for i, m in enumerate(markers[:args.limit]):
            raw = m.group(0)
            inner = m.group(1)
            if len(inner) > 120:
                inner = inner[:120] + "..."
            print(f"[{i}] {inner}")
        if len(markers) > args.limit:
            print(f"... ({len(markers) - args.limit} more markers)")


if __name__ == "__main__":
    main()
