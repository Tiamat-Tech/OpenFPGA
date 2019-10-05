/**********************************************************
 * MIT License
 *
 * Copyright (c) 2018 LNIS - The University of Utah
 *
 * Permission is hereby granted, free of charge, to any person obtaining a copy
 * of this software and associated documentation files (the "Software"), to deal
 * in the Software without restriction, including without limitation the rights
 * to use, copy, modify, merge, publish, distribute, sublicense, and/or sell
 * copies of the Software, and to permit persons to whom the Software is
 * furnished to do so, subject to the following conditions:
 *
 * The above copyright notice and this permission notice shall be included in all
 * copies or substantial portions of the Software.
 *
 * THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND, EXPRESS OR
 * IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF MERCHANTABILITY,
 * FITNESS FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT. IN NO EVENT SHALL THE
 * AUTHORS OR COPYRIGHT HOLDERS BE LIABLE FOR ANY CLAIM, DAMAGES OR OTHER
 * LIABILITY, WHETHER IN AN ACTION OF CONTRACT, TORT OR OTHERWISE, ARISING FROM,
 * OUT OF OR IN CONNECTION WITH THE SOFTWARE OR THE USE OR OTHER DEALINGS IN THE
 * SOFTWARE.
 ***********************************************************************/

/************************************************************************
 * Filename:    circuit_library_utils.cpp
 * Created by:   Xifan Tang
 * Change history:
 * +-------------------------------------+
 * |  Date       |    Author   | Notes
 * +-------------------------------------+
 * | 2019/09/27  |  Xifan Tang | Created 
 * +-------------------------------------+
 ***********************************************************************/

/************************************************************************
 * Function to perform fundamental operation for the circuit library 
 * These functions are not universal methods for the CircuitLibrary class
 * They are made to ease the development in some specific purposes
 * Please classify such functions in this file
 ***********************************************************************/

/* Header files should be included in a sequence */
/* Standard header files required go first */
#include <algorithm>

#include "vtr_assert.h"

#include "util.h"

#include "circuit_library_utils.h"

/********************************************************************
 * Get the model id of a SRAM model that is used to configure 
 * a circuit model
 *******************************************************************/
std::vector<CircuitModelId> find_circuit_sram_models(const CircuitLibrary& circuit_lib,
                                                     const CircuitModelId& circuit_model) {
  /* SRAM model id is stored in the sram ports of a circuit model */
  std::vector<CircuitPortId> sram_ports = circuit_lib.model_ports_by_type(circuit_model, SPICE_MODEL_PORT_SRAM);
  std::vector<CircuitModelId> sram_models;
  
  /* Create a list of sram models, but avoid duplicated model ids */
  for (const auto& sram_port : sram_ports) {
    CircuitModelId sram_model = circuit_lib.port_tri_state_model(sram_port);
    VTR_ASSERT( true == circuit_lib.valid_model_id(sram_model) );
    if (sram_models.end() != std::find(sram_models.begin(), sram_models.end(), sram_model)) {
      continue;  /* Already in the list, skip the addition */
    }
    /* Not in the list, add it */
    sram_models.push_back(sram_model);
  }
  
  return sram_models;
}

/********************************************************************
 * Find regular (not mode select) sram ports of a circuit model
 *******************************************************************/
std::vector<CircuitPortId> find_circuit_regular_sram_ports(const CircuitLibrary& circuit_lib,
                                                           const CircuitModelId& circuit_model) {
  std::vector<CircuitPortId> sram_ports = circuit_lib.model_ports_by_type(circuit_model, SPICE_MODEL_PORT_SRAM, true);
  std::vector<CircuitPortId> regular_sram_ports;

  for (const auto& port : sram_ports) {
    if (true == circuit_lib.port_is_mode_select(port)) {
      continue;
    }
    regular_sram_ports.push_back(port);
  }

  return regular_sram_ports;
}