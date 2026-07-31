"""
presym_pp.py - Cross-platform preprocessor wrapper for mruby presym scanning.

Runs the C compiler in preprocessor-only mode with -DMRB_PRESYM_SCANNING
and captures stdout to a .pi file.  This avoids all the quoting / argument
escaping issues that plague CMake's add_custom_command on MSVC.

Usage:
  python3 presym_pp.py --compiler <cc> --output <pi_file> -- <source> [extra_flags...]
"""

import subprocess
import sys
import argparse
import os


def main():
    parser = argparse.ArgumentParser(
        description="Run C preprocessor for mruby presym scanning"
    )
    parser.add_argument("--compiler", required=True,
                        help="C compiler executable")
    parser.add_argument("--output", "-o", required=True,
                        help="Output .pi file")
    parser.add_argument("--source", required=True,
                        help="Source .c file")
    parser.add_argument("extra_flags", nargs="*",
                        help="Extra flags passed to compiler (e.g. -I..., /I...)")
    args = parser.parse_args()

    # Determine preprocessor flags per compiler type
    compiler_name = os.path.basename(args.compiler).lower()
    if compiler_name == "cl.exe" or compiler_name == "cl":
        # MSVC: /EP = preprocess to stdout (no #line), suppress compilation
        pp_flags = ["/EP", "/DMRB_PRESYM_SCANNING"]
    else:
        # GCC / Clang: -E = preprocess only
        pp_flags = ["-E", "-DMRB_PRESYM_SCANNING"]

    cmd = [args.compiler] + pp_flags + args.extra_flags + [args.source]

    try:
        proc = subprocess.run(
            cmd,
            stdout=subprocess.PIPE,
            stderr=subprocess.PIPE,
        )
    except FileNotFoundError:
        print(f"Error: compiler not found: {args.compiler}", file=sys.stderr)
        sys.exit(1)

    if proc.returncode != 0:
        # Decode stderr for readable error messages
        try:
            err = proc.stderr.decode("utf-8", errors="replace")
        except Exception:
            err = proc.stderr.decode("ascii", errors="replace")
        print(err, file=sys.stderr)
        sys.exit(proc.returncode)

    # Write preprocessed output to .pi file.
    # Use binary mode + latin-1 decode to avoid code-page issues on Windows
    # (e.g. Chinese GBK/CP936). The markers <@! "..." !@> are pure ASCII.
    raw = proc.stdout
    try:
        text = raw.decode("utf-8", errors="surrogateescape")
    except Exception:
        text = raw.decode("latin-1", errors="replace")

    os.makedirs(os.path.dirname(args.output) or ".", exist_ok=True)
    with open(args.output, "w", encoding="utf-8", newline="\n") as f:
        f.write(text)

    print(f"Preprocessed: {args.source} -> {args.output}")


if __name__ == "__main__":
    main()
