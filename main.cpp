#include <ptlib.h>
#include "telekarma.h"

/**
 * The PCREATE_PROCESS macro...
 * 1. Defines the main() function.
 * 2. Creates an instance of TeleKarma.
 * 3. Calls instance->PreInitialise() which is inherited from PProcess.
 * 4. Calls instance->InternalMain() which is inherited from PProcess.
 * 5. instance->InternalMain() calls instance->Main() which must be
 *    defined in TeleKarma.
 */
PCREATE_PROCESS(TeleKarma);