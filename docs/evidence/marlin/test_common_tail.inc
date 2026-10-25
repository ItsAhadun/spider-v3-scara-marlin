/* ---- shared tail: include Marlin boards.h + pins.h exactly as Marlin does (boards first, then pins) ---- */
#include "core/boards.h"
#include "pins/pins.h"

/* ---- self-checks of the stubs / selection ---- */
#if !MB(FYSETC_SPIDER_V2_2)
  #error "MB(FYSETC_SPIDER_V2_2) did not evaluate true"
#endif
#if MB(FYSETC_SPIDER) || MB(FYSETC_S6) || MB(FYSETC_S6_V2_0)
  #error "MB() matched a sibling board"
#endif
#define _T_EMPTY
#define _T_ONE 1
#define _T_ZERO 0
#if !ENABLED(_T_EMPTY) || !ENABLED(_T_ONE) || ENABLED(_T_ZERO) || ENABLED(_T_UNDEFINED) || !DISABLED(_T_UNDEFINED) || !ANY(_T_UNDEFINED,_T_ONE) || ALL(_T_ONE,_T_UNDEFINED)
  #error "macros.h stub self-test failed"
#endif
#undef _T_EMPTY
#undef _T_ONE
#undef _T_ZERO
