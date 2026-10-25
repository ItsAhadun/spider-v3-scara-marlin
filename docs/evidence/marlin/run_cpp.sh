#!/bin/sh
# Preprocessor runs. -undef drops system-specific predefines (e.g. lowercase 'linux') so they cannot
# collide with MB()/ENABLED() argument names. -std=gnu11 for named variadic macros (V...).
set -u
D="$(cd "$(dirname "$0")" && pwd)"
cd "$D"
for cfg in a_uart b_default c_no_eeprom_settings; do
  gcc -E -dM -undef -std=gnu11 -I pp "test_${cfg}.c" > "dM_${cfg}.txt" 2> "stderr_dM_${cfg}.txt"; echo "dM ${cfg} exit=$?"
  gcc -E -dD -undef -std=gnu11 -I pp "test_${cfg}.c" > "dD_${cfg}.txt" 2> "stderr_dD_${cfg}.txt"; echo "dD ${cfg} exit=$?"
done
# Cross-check with clang (final macro table only)
for cfg in a_uart b_default; do
  clang -E -dM -undef -std=gnu11 -I pp "test_${cfg}.c" > "clang_dM_${cfg}.txt" 2> "stderr_clang_dM_${cfg}.txt"; echo "clang dM ${cfg} exit=$?"
done
