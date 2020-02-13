#ifndef BUILD_DECODER_MODULES_H
#define BUILD_DECODER_MODULES_H

/********************************************************************
 * Include header files that are required by function declaration
 *******************************************************************/
#include "module_manager.h"
#include "mux_library.h"
#include "circuit_library.h"

/********************************************************************
 * Function declaration
 *******************************************************************/

/* begin namespace openfpga */
namespace openfpga {

void build_mux_local_decoder_modules(ModuleManager& module_manager,
                                     const MuxLibrary& mux_lib,
                                     const CircuitLibrary& circuit_lib);

} /* end namespace openfpga */

#endif
