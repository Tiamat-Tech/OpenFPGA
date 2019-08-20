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
 * Filename:    circuit_library.cpp
 * Created by:   Xifan Tang
 * Change history:
 * +-------------------------------------+
 * |  Date       |    Author   | Notes
 * +-------------------------------------+
 * | 2019/08/07  |  Xifan Tang | Created 
 * +-------------------------------------+
 ***********************************************************************/

#include <numeric>
#include <algorithm>

#include "vtr_assert.h"

#include "port_parser.h"
#include "circuit_library.h"

/************************************************************************
 * Member functions for class CircuitLibrary
 ***********************************************************************/

/************************************************************************
 * Constructors
 ***********************************************************************/
CircuitLibrary::CircuitLibrary() {
  return;
}

/************************************************************************
 * Public Accessors : aggregates
 ***********************************************************************/
CircuitLibrary::model_range CircuitLibrary::models() const {
  return vtr::make_range(model_ids_.begin(), model_ids_.end());
}

CircuitLibrary::circuit_port_range CircuitLibrary::ports(const CircuitModelId& model_id) const {
  return vtr::make_range(port_ids_[model_id].begin(), port_ids_[model_id].end());
}

/* Find circuit models in the same type (defined by users) and return a list of ids */
std::vector<CircuitModelId> CircuitLibrary::models_by_type(const enum e_spice_model_type& type) const {
  std::vector<CircuitModelId> type_ids;
  for (auto id : models()) {
    /* Skip unmatched types */
    if (type != model_type(id)) {
      continue;
    }
    /* Matched type, update the vector */
    type_ids.push_back(id);
  }
  return type_ids;
}

/* Find the ports of a circuit model by a given type, return a list of qualified ports */
std::vector<CircuitPortId> CircuitLibrary::ports_by_type(const CircuitModelId& model_id, 
                                                         const enum e_spice_model_port_type& type) const {
  std::vector<CircuitPortId> port_ids;
  for (const auto& port_id : ports(model_id)) {
    /* We skip unmatched ports */
    if ( type != port_type(model_id, port_id) ) {
      continue; 
    }
    port_ids.push_back(port_id);
  } 
  return port_ids;
}

/* Find the ports of a circuit model by a given type, return a list of qualified ports 
 * with an option to include/exclude global ports
 */
std::vector<CircuitPortId> CircuitLibrary::ports_by_type(const CircuitModelId& model_id, 
                                                         const enum e_spice_model_port_type& type,
                                                         const bool& include_global_port) const {
  std::vector<CircuitPortId> port_ids;
  for (const auto& port_id : ports(model_id)) {
    /* We skip unmatched ports */
    if ( type != port_type(model_id, port_id) ) {
      continue; 
    }
    /* We skip global ports if specified */
    if ( (false == include_global_port)
      && (true == port_is_global(model_id, port_id)) ) {
      continue; 
    }
    port_ids.push_back(port_id);
  } 
  return port_ids;
}


/* Create a vector for all the ports whose directionality is input
 * This includes all the ports other than whose types are OUPUT or INOUT 
 */
std::vector<CircuitPortId> CircuitLibrary::input_ports(const CircuitModelId& model_id) const {
  std::vector<CircuitPortId> input_ports;
  for (const auto& port_id : ports(model_id)) {
    /* We skip output ports */
    if ( false == is_input_port(model_id, port_id) ) {
      continue; 
    }
    input_ports.push_back(port_id);
  } 
  return input_ports;
}

/* Create a vector for all the ports whose directionality is output
 * This includes all the ports whose types are OUPUT or INOUT 
 */
std::vector<CircuitPortId> CircuitLibrary::output_ports(const CircuitModelId& model_id) const {
  std::vector<CircuitPortId> output_ports;
  for (const auto& port_id : ports(model_id)) {
    /* We skip input ports */
    if ( false == is_output_port(model_id, port_id) ) {
      continue; 
    }
    output_ports.push_back(port_id);
  } 
  return output_ports;
}

/* Create a vector for the pin indices, which is bounded by the size of a port
 * Start from 0 and end to port_size - 1
 */
std::vector<size_t> CircuitLibrary::pins(const CircuitModelId& model_id, const CircuitPortId& circuit_port_id) const {
  std::vector<size_t> pin_range(port_size(model_id, circuit_port_id));
  /* Create a vector, with sequentially increasing numbers */
  std::iota(pin_range.begin(), pin_range.end(), 0);
  return pin_range;
}


/************************************************************************
 * Public Accessors : Basic data query on Circuit Models
 ***********************************************************************/
/* Get the number of circuit models */
size_t CircuitLibrary::num_models() const {
  return model_ids_.size();
}

/* Access the type of a circuit model */
enum e_spice_model_type CircuitLibrary::model_type(const CircuitModelId& model_id) const {
  /* validate the model_id */
  VTR_ASSERT(valid_model_id(model_id));
  return model_types_[model_id]; 
}

/* Access the name of a circuit model */
std::string CircuitLibrary::model_name(const CircuitModelId& model_id) const {
  /* validate the model_id */
  VTR_ASSERT(valid_model_id(model_id));
  return model_names_[model_id]; 
}

/* Access the prefix of a circuit model */
std::string CircuitLibrary::model_prefix(const CircuitModelId& model_id) const {
  /* validate the model_id */
  VTR_ASSERT(valid_model_id(model_id));
  return model_prefix_[model_id]; 
}

/* Access the path + file of user-defined verilog netlist of a circuit model */
std::string CircuitLibrary::model_verilog_netlist(const CircuitModelId& model_id) const {
  /* validate the model_id */
  VTR_ASSERT(valid_model_id(model_id));
  return model_verilog_netlists_[model_id]; 
}

/* Access the path + file of user-defined spice netlist of a circuit model */
std::string CircuitLibrary::model_spice_netlist(const CircuitModelId& model_id) const {
  /* validate the model_id */
  VTR_ASSERT(valid_model_id(model_id));
  return model_spice_netlists_[model_id]; 
}

/* Access the is_default flag (check if this is the default circuit model in the type) of a circuit model */
bool CircuitLibrary::model_is_default(const CircuitModelId& model_id) const {
  /* validate the model_id */
  VTR_ASSERT(valid_model_id(model_id));
  return model_is_default_[model_id]; 
}

/* Access the dump_structural_verilog flag of a circuit model */
bool CircuitLibrary::dump_structural_verilog(const CircuitModelId& model_id) const {
  /* validate the model_id */
  VTR_ASSERT(valid_model_id(model_id));
  return dump_structural_verilog_[model_id]; 
}

/* Access the dump_explicit_port_map flag of a circuit model */
bool CircuitLibrary::dump_explicit_port_map(const CircuitModelId& model_id) const {
  /* validate the model_id */
  VTR_ASSERT(valid_model_id(model_id));
  return dump_explicit_port_map_[model_id]; 
}

/* Access the design technology type of a circuit model */
enum e_spice_model_design_tech CircuitLibrary::design_tech_type(const CircuitModelId& model_id) const {
  /* validate the model_id */
  VTR_ASSERT(valid_model_id(model_id));
  return design_tech_types_[model_id]; 
}

/* Access the is_power_gated flag of a circuit model */
bool CircuitLibrary::is_power_gated(const CircuitModelId& model_id) const {
  /* validate the model_id */
  VTR_ASSERT(valid_model_id(model_id));
  return is_power_gated_[model_id]; 
}

/* Return a flag showing if inputs are buffered for a circuit model */
bool CircuitLibrary::is_input_buffered(const CircuitModelId& model_id) const {
  /* validate the model_id */
  VTR_ASSERT(valid_model_id(model_id));
  return buffer_existence_[model_id][INPUT]; 
}

/* Return a flag showing if outputs are buffered for a circuit model */
bool CircuitLibrary::is_output_buffered(const CircuitModelId& model_id) const {
  /* validate the model_id */
  VTR_ASSERT(valid_model_id(model_id));
  return buffer_existence_[model_id][OUTPUT]; 
}

/* Return a flag showing if intermediate stages of a LUT are buffered for a circuit model */
bool CircuitLibrary::is_lut_intermediate_buffered(const CircuitModelId& model_id) const {
  /* validate the model_id */
  VTR_ASSERT(valid_model_id(model_id));
  /* validate the circuit model type is LUT */
  VTR_ASSERT(SPICE_MODEL_LUT == model_type(model_id));
  return buffer_existence_[model_id][LUT_INTER_BUFFER]; 
}

/* Find the type of pass-gate logic for a circuit model (recursive function)
 * Two cases to be considered:
 * 1. this is a pass-gate circuit model, just find the data and return
 * 2. this circuit model includes a pass-gate, find the link to pass-gate circuit model and go recursively
 */
enum e_spice_model_pass_gate_logic_type CircuitLibrary::pass_gate_logic_type(const CircuitModelId& model_id) const {
  /* validate the model_id */
  VTR_ASSERT(valid_model_id(model_id));
  
  /* Return the data if this is a pass-gate circuit model */
  if (SPICE_MODEL_PASSGATE == model_type(model_id)) {
    return pass_gate_logic_types_[model_id];
  }

  /* Otherwise, we need to make sure this circuit model contains a pass-gate */
  CircuitModelId pgl_model_id = pass_gate_logic_model_ids_[model_id];
  VTR_ASSERT( CircuitModelId::INVALID() != pgl_model_id );
  return pass_gate_logic_type(pgl_model_id);
}

/* Return the multiplex structure of a circuit model */
enum e_spice_model_structure CircuitLibrary::mux_structure(const CircuitModelId& model_id) const {
  /* validate the model_id */
  VTR_ASSERT(valid_model_id(model_id));
  /* validate the circuit model type is MUX */
  VTR_ASSERT( (SPICE_MODEL_MUX == model_type(model_id))
           || (SPICE_MODEL_LUT == model_type(model_id)) );
  return mux_structure_[model_id]; 
}


size_t CircuitLibrary::mux_num_levels(const CircuitModelId& model_id) const {
  /* validate the model_id */
  VTR_ASSERT(valid_model_id(model_id));
  /* validate the circuit model type is MUX */
  VTR_ASSERT( (SPICE_MODEL_MUX == model_type(model_id))
           || (SPICE_MODEL_LUT == model_type(model_id)) );

  return mux_num_levels_[model_id];
}

/* Return if additional constant inputs are required for a circuit model 
 * Only applicable for MUX circuit model 
 */
bool CircuitLibrary::mux_add_const_input(const CircuitModelId& model_id) const {
  /* validate the model_id */
  VTR_ASSERT(valid_model_id(model_id));
  /* validate the circuit model type is MUX */
  VTR_ASSERT( (SPICE_MODEL_MUX == model_type(model_id))
           || (SPICE_MODEL_LUT == model_type(model_id)) );
  /* A -1 value for the const values means there is no const inputs */
  return ( size_t(-1) != mux_const_input_values_[model_id] );
}

/* Return if additional constant inputs are required for a circuit model 
 * Only applicable for MUX circuit model 
 */
size_t CircuitLibrary::mux_const_input_value(const CircuitModelId& model_id) const {
  /* validate the model_id */
  VTR_ASSERT(valid_model_id(model_id));
  /* validate the circuit model type is MUX */
  VTR_ASSERT( (SPICE_MODEL_MUX == model_type(model_id))
           || (SPICE_MODEL_LUT == model_type(model_id)) );
  /* A -1 value for the const values means there is no const inputs */
  /* A 0 value for the const values means it is logic 0 */
  /* A 1 value for the const values means it is logic 1 */
  return mux_const_input_values_[model_id];
}

/************************************************************************
 * Public Accessors : Basic data query on Circuit Porst
 ***********************************************************************/

/* identify if this port is an input port */
bool CircuitLibrary::is_input_port(const CircuitModelId& model_id, const CircuitPortId& circuit_port_id) const {
  /* validate the model_id and circuit_port_id */
  VTR_ASSERT(valid_circuit_port_id(model_id, circuit_port_id));
  /* Only SPICE_MODEL_OUTPUT AND INOUT are considered as outputs */
  return ( (SPICE_MODEL_PORT_OUTPUT != port_type(model_id, circuit_port_id)) 
        && (SPICE_MODEL_PORT_INOUT  != port_type(model_id, circuit_port_id)) );
}

/* identify if this port is an output port */
bool CircuitLibrary::is_output_port(const CircuitModelId& model_id, const CircuitPortId& circuit_port_id) const {
  /* validate the model_id and circuit_port_id */
  VTR_ASSERT(valid_circuit_port_id(model_id, circuit_port_id));
  /* Only SPICE_MODEL_OUTPUT AND INOUT are considered as outputs */
  return ( (SPICE_MODEL_PORT_OUTPUT == port_type(model_id, circuit_port_id)) 
        || (SPICE_MODEL_PORT_INOUT  == port_type(model_id, circuit_port_id)) );
}

/* Given a name and return the port id */
CircuitPortId CircuitLibrary::port(const CircuitModelId& model_id, const std::string& name) const {
  /* validate the model_id */
  VTR_ASSERT(valid_model_id(model_id));
  /* Walk through the ports and try to find a matched name */
  CircuitPortId ret = CircuitPortId::INVALID();
  size_t num_found = 0;
  for (auto port_id : ports(model_id)) {
    if (0 != name.compare(port_prefix(model_id, port_id))) {
      continue; /* Not the one, go to the next*/
    }
    ret = port_id; /* Find one */
    num_found++;
  }
  /* Make sure we will not find two ports with the same name */
  VTR_ASSERT( (0 == num_found) || (1 == num_found) );
  return ret;
}

/* Access the type of a port of a circuit model */
size_t CircuitLibrary::num_ports(const CircuitModelId& model_id) const {
  /* validate the model_id */
  VTR_ASSERT(valid_model_id(model_id));
  return port_ids_[model_id].size();
}

/* Access the type of a port of a circuit model 
 * with an option to include/exclude global ports 
 * when counting 
 */
size_t CircuitLibrary::num_ports_by_type(const CircuitModelId& model_id,
                                         const enum e_spice_model_port_type& port_type, 
                                         const bool& include_global_port) const {
  /* validate the model_id */
  VTR_ASSERT(valid_model_id(model_id));
  return ports_by_type(model_id, port_type, include_global_port).size();
}

/* Access the type of a port of a circuit model */
enum e_spice_model_port_type CircuitLibrary::port_type(const CircuitModelId& model_id, 
                                                       const CircuitPortId& circuit_port_id) const {
  /* validate the circuit_port_id */
  VTR_ASSERT(valid_circuit_port_id(model_id, circuit_port_id));
  return port_types_[model_id][circuit_port_id];
}

/* Access the type of a port of a circuit model */
size_t CircuitLibrary::port_size(const CircuitModelId& model_id, 
                                 const CircuitPortId& circuit_port_id) const {
  /* validate the circuit_port_id */
  VTR_ASSERT(valid_circuit_port_id(model_id, circuit_port_id));
  return port_sizes_[model_id][circuit_port_id];
}

/* Access the prefix of a port of a circuit model */
std::string CircuitLibrary::port_prefix(const CircuitModelId& model_id, 
                                        const CircuitPortId& circuit_port_id) const {
  /* validate the circuit_port_id */
  VTR_ASSERT(valid_circuit_port_id(model_id, circuit_port_id));
  return port_prefix_[model_id][circuit_port_id];
}

/* Access the lib_name of a port of a circuit model */
std::string CircuitLibrary::port_lib_name(const CircuitModelId& model_id, 
                                          const CircuitPortId& circuit_port_id) const {
  /* validate the circuit_port_id */
  VTR_ASSERT(valid_circuit_port_id(model_id, circuit_port_id));
  return port_lib_names_[model_id][circuit_port_id];
}

/* Access the inv_prefix of a port of a circuit model */
std::string CircuitLibrary::port_inv_prefix(const CircuitModelId& model_id, 
                                            const CircuitPortId& circuit_port_id) const {
  /* validate the circuit_port_id */
  VTR_ASSERT(valid_circuit_port_id(model_id, circuit_port_id));
  return port_inv_prefix_[model_id][circuit_port_id];
}

/* Return the default value of a port of a circuit model */
size_t CircuitLibrary::port_default_value(const CircuitModelId& model_id, 
                                        const CircuitPortId& circuit_port_id) const {
  /* validate the circuit_port_id */
  VTR_ASSERT(valid_circuit_port_id(model_id, circuit_port_id));
  return port_default_values_[model_id][circuit_port_id];
}


/* Return a flag if the port is used in mode-selection purpuse of a circuit model */
bool CircuitLibrary::port_is_mode_select(const CircuitModelId& model_id, 
                                         const CircuitPortId& circuit_port_id) const {
  /* validate the circuit_port_id */
  VTR_ASSERT(valid_circuit_port_id(model_id, circuit_port_id));
  return port_is_mode_select_[model_id][circuit_port_id];
}

/* Return a flag if the port is a global one of a circuit model */
bool CircuitLibrary::port_is_global(const CircuitModelId& model_id, 
                                    const CircuitPortId& circuit_port_id) const {
  /* validate the circuit_port_id */
  VTR_ASSERT(valid_circuit_port_id(model_id, circuit_port_id));
  return port_is_global_[model_id][circuit_port_id];
}

/* Return a flag if the port does a reset functionality in a circuit model */
bool CircuitLibrary::port_is_reset(const CircuitModelId& model_id, 
                                   const CircuitPortId& circuit_port_id) const {
  /* validate the circuit_port_id */
  VTR_ASSERT(valid_circuit_port_id(model_id, circuit_port_id));
  return port_is_reset_[model_id][circuit_port_id];
}

/* Return a flag if the port does a set functionality in a circuit model */
bool CircuitLibrary::port_is_set(const CircuitModelId& model_id, 
                                 const CircuitPortId& circuit_port_id) const {
  /* validate the circuit_port_id */
  VTR_ASSERT(valid_circuit_port_id(model_id, circuit_port_id));
  return port_is_set_[model_id][circuit_port_id];
}

/* Return a flag if the port enables a configuration in a circuit model */
bool CircuitLibrary::port_is_config_enable(const CircuitModelId& model_id, 
                                           const CircuitPortId& circuit_port_id) const {
  /* validate the circuit_port_id */
  VTR_ASSERT(valid_circuit_port_id(model_id, circuit_port_id));
  return port_is_config_enable_[model_id][circuit_port_id];
}

/* Return a flag if the port is used during programming a FPGA in a circuit model */
bool CircuitLibrary::port_is_prog(const CircuitModelId& model_id, 
                                  const CircuitPortId& circuit_port_id) const {
  /* validate the circuit_port_id */
  VTR_ASSERT(valid_circuit_port_id(model_id, circuit_port_id));
  return port_is_prog_[model_id][circuit_port_id];
}


/************************************************************************
 * Public Accessors : Methods to find circuit model 
 ***********************************************************************/
/* Find a circuit model by a given name and return its id */
CircuitModelId CircuitLibrary::model(const char* name) const {
  std::string name_str(name);
  return model(name_str);
}

/* Find a circuit model by a given name and return its id */
CircuitModelId CircuitLibrary::model(const std::string& name) const { 
  CircuitModelId ret = CircuitModelId::INVALID();
  size_t num_found = 0;
  for (model_string_iterator it = model_names_.begin();
       it != model_names_.end();
       it++) {
    /* Bypass unmatched names */
    if ( 0 != name.compare(*it) ) {
      continue;
    }
    /* Find one and record it 
     * FIXME: I feel that we may have a better way in getting the CircuitModelId
     */
    ret = CircuitModelId(it - model_names_.begin()); 
    num_found++;
  }
  VTR_ASSERT((0 == num_found) || (1 == num_found));
  return ret;
}

/* Get the CircuitModelId of a default circuit model with a given type */
CircuitModelId CircuitLibrary::default_model(const enum e_spice_model_type& type) const {
  /* Default circuit model id is the first element by type in the fast look-up */
  CircuitModelId default_id = model_lookup_[size_t(type)].front();
  VTR_ASSERT(true == model_is_default(default_id));
  return default_id;
}

/************************************************************************
 * Public Accessors: Timing graph 
 ***********************************************************************/
/* Given the source and sink port information, find the edge connecting the two ports */
CircuitEdgeId CircuitLibrary::edge(const CircuitModelId& model_id,
                                   const CircuitPortId& from_port, const size_t from_pin,
                                   const CircuitPortId& to_port, const size_t to_pin) {
  /* validate the circuit_pin_id */
  VTR_ASSERT(valid_circuit_pin_id(model_id, from_port, from_pin));
  VTR_ASSERT(valid_circuit_pin_id(model_id, to_port, to_pin));
  /* Walk through the edge list until we find the one */
  for (auto edge : edge_ids_[model_id]) { 
    if ( (from_port == edge_src_port_ids_[model_id][edge])
      && (from_pin  == edge_src_pin_ids_[model_id][edge])
      && (to_port == edge_sink_port_ids_[model_id][edge])
      && (to_pin  == edge_sink_pin_ids_[model_id][edge]) ) {
      return edge;
    }
  }
  /* Reach here it means we find nothing! */
  return CircuitEdgeId::INVALID();
}

/************************************************************************
 * Public Mutators 
 ***********************************************************************/
/* Add a circuit model to the library, and return it Id */
CircuitModelId CircuitLibrary::add_model() {
  /* Create a new id*/
  CircuitModelId model_id = CircuitModelId(model_ids_.size());
  /* Update the id list */
  model_ids_.push_back(model_id);
  
  /* Initialize other attributes */
  /* Fundamental information */
  model_types_.push_back(NUM_CIRCUIT_MODEL_TYPES);
  model_names_.emplace_back();
  model_prefix_.emplace_back();
  model_verilog_netlists_.emplace_back();
  model_spice_netlists_.emplace_back();
  model_is_default_.push_back(false);

  /* Verilog generator options */ 
  dump_structural_verilog_.push_back(false);
  dump_explicit_port_map_.push_back(false);
 
  /* Design technology information */ 
  design_tech_types_.push_back(NUM_CIRCUIT_MODEL_DESIGN_TECH_TYPES);
  is_power_gated_.push_back(false);
  
  /* Buffer existence */
  buffer_existence_.emplace_back();
  buffer_model_names_.emplace_back();
  buffer_model_ids_.emplace_back();
  buffer_location_maps_.emplace_back();

  /* Pass-gate-related parameters */
  pass_gate_logic_model_names_.emplace_back();
  pass_gate_logic_model_ids_.emplace_back();

  /* Port information */
  port_ids_.emplace_back();
  port_types_.emplace_back();
  port_sizes_.emplace_back();
  port_prefix_.emplace_back();
  port_lib_names_.emplace_back();
  port_inv_prefix_.emplace_back();
  port_default_values_.emplace_back();
  port_is_mode_select_.emplace_back();
  port_is_global_.emplace_back();
  port_is_reset_.emplace_back();
  port_is_set_.emplace_back();
  port_is_config_enable_.emplace_back();
  port_is_prog_.emplace_back();
  port_model_names_.emplace_back();
  port_model_ids_.emplace_back();
  port_inv_model_names_.emplace_back();
  port_inv_model_ids_.emplace_back();
  port_tri_state_maps_.emplace_back();
  port_lut_frac_level_.emplace_back();
  port_lut_output_masks_.emplace_back();
  port_sram_orgz_.emplace_back();

  /* Timing graphs */
  edge_ids_.emplace_back();
  port_in_edge_ids_.emplace_back();
  port_out_edge_ids_.emplace_back();
  edge_src_port_ids_.emplace_back();
  edge_src_pin_ids_.emplace_back();
  edge_sink_port_ids_.emplace_back();
  edge_sink_pin_ids_.emplace_back();
  edge_timing_info_.emplace_back();

  /* Delay information */
  delay_types_.emplace_back();
  delay_in_port_names_.emplace_back();
  delay_out_port_names_.emplace_back();
  delay_values_.emplace_back();

  /* Buffer/Inverter-related parameters */
  buffer_types_.push_back(NUM_CIRCUIT_MODEL_BUF_TYPES);
  buffer_sizes_.push_back(-1);
  buffer_num_levels_.push_back(-1);
  buffer_f_per_stage_.push_back(-1);

  /* Pass-gate-related parameters */
  pass_gate_logic_types_.push_back(NUM_CIRCUIT_MODEL_PASS_GATE_TYPES);
  pass_gate_logic_sizes_.emplace_back();

  /* Multiplexer-related parameters */
  mux_structure_.push_back(NUM_CIRCUIT_MODEL_STRUCTURE_TYPES);
  mux_num_levels_.push_back(-1);
  mux_const_input_values_.push_back(-1);
  mux_use_local_encoder_.push_back(false);
  mux_use_advanced_rram_design_.push_back(false);

  /* LUT-related parameters */
  lut_is_fracturable_.push_back(false);

  /* Gate-related parameters */
  gate_types_.push_back(NUM_SPICE_MODEL_GATE_TYPES);

  /* RRAM-related design technology information */
  rram_res_.emplace_back();
  wprog_set_.emplace_back();
  wprog_reset_.emplace_back();

  /* Wire parameters */
  wire_types_.push_back(NUM_WIRE_MODEL_TYPES);
  wire_rc_.emplace_back();
  wire_num_levels_.push_back(-1);

  /* Update circuit port fast look-up */
  model_port_lookup_.emplace_back();

  /* Invalidate fast look-up*/
  invalidate_model_lookup();

  return model_id;
}

/* Set the type of a Circuit Model */
void CircuitLibrary::set_model_type(const CircuitModelId& model_id, 
                                            const enum e_spice_model_type& type) {
  /* validate the model_id */
  VTR_ASSERT(valid_model_id(model_id));
  model_types_[model_id] = type;
  /* Build the fast look-up for circuit models */
  build_model_lookup();
  return;
}

/* Set the name of a Circuit Model */
void CircuitLibrary::set_model_name(const CircuitModelId& model_id, const std::string& name) {
  /* validate the model_id */
  VTR_ASSERT(valid_model_id(model_id));
  model_names_[model_id] = name;
  return;
}

/* Set the prefix of a Circuit Model */
void CircuitLibrary::set_model_prefix(const CircuitModelId& model_id, const std::string& prefix) {
  /* validate the model_id */
  VTR_ASSERT(valid_model_id(model_id));
  model_prefix_[model_id] = prefix;
  return;
}

/* Set the verilog_netlist of a Circuit Model */
void CircuitLibrary::set_model_verilog_netlist(const CircuitModelId& model_id, const std::string& verilog_netlist) {
  /* validate the model_id */
  VTR_ASSERT(valid_model_id(model_id));
  model_verilog_netlists_[model_id] = verilog_netlist;
  return;
}

/* Set the spice_netlist of a Circuit Model */
void CircuitLibrary::set_model_spice_netlist(const CircuitModelId& model_id, const std::string& spice_netlist) {
  /* validate the model_id */
  VTR_ASSERT(valid_model_id(model_id));
  model_spice_netlists_[model_id] = spice_netlist;
  return;
}

/* Set the is_default of a Circuit Model */
void CircuitLibrary::set_model_is_default(const CircuitModelId& model_id, const bool& is_default) {
  /* validate the model_id */
  VTR_ASSERT(valid_model_id(model_id));
  model_is_default_[model_id] = is_default;
  return;
}

/* Set the dump_structural_verilog of a Circuit Model */
void CircuitLibrary::set_model_dump_structural_verilog(const CircuitModelId& model_id, const bool& dump_structural_verilog) {
  /* validate the model_id */
  VTR_ASSERT(valid_model_id(model_id));
  dump_structural_verilog_[model_id] = dump_structural_verilog;
  return;
}

/* Set the dump_explicit_port_map of a Circuit Model */
void CircuitLibrary::set_model_dump_explicit_port_map(const CircuitModelId& model_id, const bool& dump_explicit_port_map) {
  /* validate the model_id */
  VTR_ASSERT(valid_model_id(model_id));
  dump_explicit_port_map_[model_id] = dump_explicit_port_map;
  return;
}

/* Set the type of design technology of a Circuit Model */
void CircuitLibrary::set_model_design_tech_type(const CircuitModelId& model_id, const enum e_spice_model_design_tech& design_tech_type) {
  /* validate the model_id */
  VTR_ASSERT(valid_model_id(model_id));
  design_tech_types_[model_id] = design_tech_type;
  return;
}

/* Set the power-gated flag of a Circuit Model */
void CircuitLibrary::set_model_is_power_gated(const CircuitModelId& model_id, const bool& is_power_gated) {
  /* validate the model_id */
  VTR_ASSERT(valid_model_id(model_id));
  is_power_gated_[model_id] = is_power_gated;
  return;
}

/* Set input buffer information for the circuit model */
void CircuitLibrary::set_model_input_buffer(const CircuitModelId& model_id, 
                                                    const bool& existence, const std::string& model_name) {
  /* Just call the base function and give the proper type */
  set_model_buffer(model_id, INPUT, existence, model_name);
  return;
}

/* Set output buffer information for the circuit model */
void CircuitLibrary::set_model_output_buffer(const CircuitModelId& model_id, 
                                                    const bool& existence, const std::string& model_name) {
  /* Just call the base function and give the proper type */
  set_model_buffer(model_id, OUTPUT, existence, model_name);
  return;
}

/* Set input buffer information for the circuit model, only applicable to LUTs! */
void CircuitLibrary::set_model_lut_input_buffer(const CircuitModelId& model_id, 
                                                        const bool& existence, const std::string& model_name) {
  /* validate the model_id */
  VTR_ASSERT(valid_model_id(model_id));
  /* Make sure the circuit model is a LUT! */
  VTR_ASSERT(SPICE_MODEL_LUT == model_types_[model_id]);
  /* Just call the base function and give the proper type */
  set_model_buffer(model_id, LUT_INPUT_BUFFER, existence, model_name);
  return;
}

/* Set input inverter information for the circuit model, only applicable to LUTs! */
void CircuitLibrary::set_model_lut_input_inverter(const CircuitModelId& model_id, 
                                                          const bool& existence, const std::string& model_name) {
  /* validate the model_id */
  VTR_ASSERT(valid_model_id(model_id));
  /* Make sure the circuit model is a LUT! */
  VTR_ASSERT(SPICE_MODEL_LUT == model_types_[model_id]);
  /* Just call the base function and give the proper type */
  set_model_buffer(model_id, LUT_INPUT_INVERTER, existence, model_name);
  return;
}

/* Set intermediate buffer information for the circuit model, only applicable to LUTs! */
void CircuitLibrary::set_model_lut_intermediate_buffer(const CircuitModelId& model_id, 
                                                               const bool& existence, const std::string& model_name) {
  /* validate the model_id */
  VTR_ASSERT(valid_model_id(model_id));
  /* Make sure the circuit model is a LUT! */
  VTR_ASSERT(SPICE_MODEL_LUT == model_types_[model_id]);
  /* Just call the base function and give the proper type */
  set_model_buffer(model_id, LUT_INTER_BUFFER, existence, model_name);
  return;
}

void CircuitLibrary::set_model_lut_intermediate_buffer_location_map(const CircuitModelId& model_id,
                                                                            const std::string& location_map) {
  /* validate the model_id */
  VTR_ASSERT(valid_model_id(model_id));
  buffer_location_maps_[model_id][LUT_INTER_BUFFER] = location_map; 
  return;
}


/* Set pass-gate logic information of a circuit model */
void CircuitLibrary::set_model_pass_gate_logic(const CircuitModelId& model_id, const std::string& model_name) {
  /* validate the model_id */
  VTR_ASSERT(valid_model_id(model_id));
  pass_gate_logic_model_names_[model_id] = model_name;
  return;
}

/* Add a port to a circuit model */
CircuitPortId CircuitLibrary::add_model_port(const CircuitModelId& model_id) {
  /* validate the model_id */
  VTR_ASSERT(valid_model_id(model_id));
  /* Create a port id */
  CircuitPortId circuit_port_id = CircuitPortId(port_ids_[model_id].size()); 
  /* Update the id list */
  port_ids_[model_id].push_back(circuit_port_id);
  
  /* Initialize other attributes */
  port_types_[model_id].push_back(NUM_CIRCUIT_MODEL_PORT_TYPES);
  port_sizes_[model_id].push_back(-1);
  port_prefix_[model_id].emplace_back();
  port_lib_names_[model_id].emplace_back();
  port_inv_prefix_[model_id].emplace_back();
  port_default_values_[model_id].push_back(-1);
  port_is_mode_select_[model_id].push_back(false);
  port_is_global_[model_id].push_back(false);
  port_is_reset_[model_id].push_back(false);
  port_is_set_[model_id].push_back(false);
  port_is_config_enable_[model_id].push_back(false);
  port_is_prog_[model_id].push_back(false);
  port_model_names_[model_id].emplace_back();
  port_model_ids_[model_id].push_back(CircuitModelId::INVALID());
  port_inv_model_names_[model_id].emplace_back();
  port_inv_model_ids_[model_id].push_back(CircuitModelId::INVALID());
  port_tri_state_maps_[model_id].emplace_back();
  port_lut_frac_level_[model_id].push_back(-1);
  port_lut_output_masks_[model_id].emplace_back();
  port_sram_orgz_[model_id].push_back(NUM_CIRCUIT_MODEL_SRAM_ORGZ_TYPES);

  /* For timing graphs */
  port_in_edge_ids_[model_id].emplace_back();
  port_out_edge_ids_[model_id].emplace_back();

  return circuit_port_id;
}

/* Set the type for a port of a circuit model */
void CircuitLibrary::set_port_type(const CircuitModelId& model_id, 
                                   const CircuitPortId& circuit_port_id, 
                                   const enum e_spice_model_port_type& port_type) {
  /* validate the circuit_port_id */
  VTR_ASSERT(valid_circuit_port_id(model_id, circuit_port_id));
  port_types_[model_id][circuit_port_id] = port_type;
  /* Build the fast look-up for circuit model ports */
  build_model_port_lookup(model_id);
  return;
}

/* Set the size for a port of a circuit model */
void CircuitLibrary::set_port_size(const CircuitModelId& model_id, 
                                   const CircuitPortId& circuit_port_id, 
                                   const size_t& port_size) {
  /* validate the circuit_port_id */
  VTR_ASSERT(valid_circuit_port_id(model_id, circuit_port_id));
  port_sizes_[model_id][circuit_port_id] = port_size;
  return;
}

/* Set the prefix for a port of a circuit model */
void CircuitLibrary::set_port_prefix(const CircuitModelId& model_id, 
                                     const CircuitPortId& circuit_port_id, 
                                     const std::string& port_prefix) {
  /* validate the circuit_port_id */
  VTR_ASSERT(valid_circuit_port_id(model_id, circuit_port_id));
  port_prefix_[model_id][circuit_port_id] = port_prefix;
  return;
}

/* Set the lib_name for a port of a circuit model */
void CircuitLibrary::set_port_lib_name(const CircuitModelId& model_id, 
                                       const CircuitPortId& circuit_port_id, 
                                       const std::string& lib_name) {
  /* validate the circuit_port_id */
  VTR_ASSERT(valid_circuit_port_id(model_id, circuit_port_id));
  port_lib_names_[model_id][circuit_port_id] = lib_name;
  return;
}

/* Set the inv_prefix for a port of a circuit model */
void CircuitLibrary::set_port_inv_prefix(const CircuitModelId& model_id, 
                                         const CircuitPortId& circuit_port_id, 
                                         const std::string& inv_prefix) {
  /* validate the circuit_port_id */
  VTR_ASSERT(valid_circuit_port_id(model_id, circuit_port_id));
  port_inv_prefix_[model_id][circuit_port_id] = inv_prefix;
  return;
}

/* Set the default value for a port of a circuit model */
void CircuitLibrary::set_port_default_value(const CircuitModelId& model_id, 
                                            const CircuitPortId& circuit_port_id, 
                                            const size_t& default_value) {
  /* validate the circuit_port_id */
  VTR_ASSERT(valid_circuit_port_id(model_id, circuit_port_id));
  port_default_values_[model_id][circuit_port_id] = default_value;
  return;
}

/* Set the is_mode_select for a port of a circuit model */
void CircuitLibrary::set_port_is_mode_select(const CircuitModelId& model_id, 
                                             const CircuitPortId& circuit_port_id, 
                                             const bool& is_mode_select) {
  /* validate the circuit_port_id */
  VTR_ASSERT(valid_circuit_port_id(model_id, circuit_port_id));
  port_is_mode_select_[model_id][circuit_port_id] = is_mode_select;
  return;
}

/* Set the is_global for a port of a circuit model */
void CircuitLibrary::set_port_is_global(const CircuitModelId& model_id, 
                                        const CircuitPortId& circuit_port_id, 
                                        const bool& is_global) {
  /* validate the circuit_port_id */
  VTR_ASSERT(valid_circuit_port_id(model_id, circuit_port_id));
  port_is_global_[model_id][circuit_port_id] = is_global;
  return;
}

/* Set the is_reset for a port of a circuit model */
void CircuitLibrary::set_port_is_reset(const CircuitModelId& model_id, 
                                       const CircuitPortId& circuit_port_id, 
                                       const bool& is_reset) {
  /* validate the circuit_port_id */
  VTR_ASSERT(valid_circuit_port_id(model_id, circuit_port_id));
  port_is_reset_[model_id][circuit_port_id] = is_reset;
  return;
}

/* Set the is_set for a port of a circuit model */
void CircuitLibrary::set_port_is_set(const CircuitModelId& model_id, 
                                     const CircuitPortId& circuit_port_id, 
                                     const bool& is_set) {
  /* validate the circuit_port_id */
  VTR_ASSERT(valid_circuit_port_id(model_id, circuit_port_id));
  port_is_set_[model_id][circuit_port_id] = is_set;
  return;
}

/* Set the is_config_enable for a port of a circuit model */
void CircuitLibrary::set_port_is_config_enable(const CircuitModelId& model_id, 
                                               const CircuitPortId& circuit_port_id, 
                                               const bool& is_config_enable) {
  /* validate the circuit_port_id */
  VTR_ASSERT(valid_circuit_port_id(model_id, circuit_port_id));
  port_is_config_enable_[model_id][circuit_port_id] = is_config_enable;
  return;
}

/* Set the is_prog for a port of a circuit model */
void CircuitLibrary::set_port_is_prog(const CircuitModelId& model_id, 
                                      const CircuitPortId& circuit_port_id, 
                                      const bool& is_prog) {
  /* validate the circuit_port_id */
  VTR_ASSERT(valid_circuit_port_id(model_id, circuit_port_id));
  port_is_prog_[model_id][circuit_port_id] = is_prog;
  return;
}

/* Set the model_name for a port of a circuit model */
void CircuitLibrary::set_port_model_name(const CircuitModelId& model_id, 
                                                 const CircuitPortId& circuit_port_id, 
                                                 const std::string& model_name) {
  /* validate the circuit_port_id */
  VTR_ASSERT(valid_circuit_port_id(model_id, circuit_port_id));
  port_model_names_[model_id][circuit_port_id] = model_name;
  return;
}

/* Set the model_id for a port of a circuit model */
void CircuitLibrary::set_port_model_id(const CircuitModelId& model_id, 
                                               const CircuitPortId& circuit_port_id, 
                                               const CircuitModelId& port_model_id) {
  /* validate the circuit_port_id */
  VTR_ASSERT(valid_circuit_port_id(model_id, circuit_port_id));
  port_model_ids_[model_id][circuit_port_id] = port_model_id;
  return;
}

/* Set the inv_model_name for a port of a circuit model */
void CircuitLibrary::set_port_inv_model_name(const CircuitModelId& model_id, 
                                                 const CircuitPortId& circuit_port_id, 
                                                 const std::string& inv_model_name) {
  /* validate the circuit_port_id */
  VTR_ASSERT(valid_circuit_port_id(model_id, circuit_port_id));
  port_inv_model_names_[model_id][circuit_port_id] = inv_model_name;
  return;
}

/* Set the inv_model_id for a port of a circuit model */
void CircuitLibrary::set_port_inv_model_id(const CircuitModelId& model_id, 
                                                 const CircuitPortId& circuit_port_id, 
                                                 const CircuitModelId& inv_model_id) {
  /* validate the circuit_port_id */
  VTR_ASSERT(valid_circuit_port_id(model_id, circuit_port_id));
  port_inv_model_ids_[model_id][circuit_port_id] = inv_model_id;
  return;
}

/* Set the tri-state map for a port of a circuit model */
void CircuitLibrary::set_port_tri_state_map(const CircuitModelId& model_id, 
                                            const CircuitPortId& circuit_port_id, 
                                            const std::string& tri_state_map) {
  /* validate the circuit_port_id */
  VTR_ASSERT(valid_circuit_port_id(model_id, circuit_port_id));
  port_tri_state_maps_[model_id][circuit_port_id] = tri_state_map;
  return;
}

/* Set the LUT fracturable level for a port of a circuit model, only applicable to LUTs */
void CircuitLibrary::set_port_lut_frac_level(const CircuitModelId& model_id, 
                                             const CircuitPortId& circuit_port_id, 
                                             const size_t& lut_frac_level) {
  /* validate the circuit_port_id */
  VTR_ASSERT(valid_circuit_port_id(model_id, circuit_port_id));
  /* Make sure this is a LUT */
  VTR_ASSERT(SPICE_MODEL_LUT == model_type(model_id));
  port_lut_frac_level_[model_id][circuit_port_id] = lut_frac_level;
  return;
}

/* Set the LUT fracturable level for a port of a circuit model, only applicable to LUTs */
void CircuitLibrary::set_port_lut_output_mask(const CircuitModelId& model_id, 
                                              const CircuitPortId& circuit_port_id, 
                                              const std::vector<size_t>& lut_output_masks) {
  /* validate the circuit_port_id */
  VTR_ASSERT(valid_circuit_port_id(model_id, circuit_port_id));
  /* Make sure this is a LUT */
  VTR_ASSERT(SPICE_MODEL_LUT == model_type(model_id));
  port_lut_output_masks_[model_id][circuit_port_id] = lut_output_masks;
  return;
}

/* Set the SRAM organization for a port of a circuit model, only applicable to SRAM ports */
void CircuitLibrary::set_port_sram_orgz(const CircuitModelId& model_id, 
                                        const CircuitPortId& circuit_port_id, 
                                        const enum e_sram_orgz& sram_orgz) {
  /* validate the circuit_port_id */
  VTR_ASSERT(valid_circuit_port_id(model_id, circuit_port_id));
  /* Make sure this is a SRAM port */
  VTR_ASSERT(SPICE_MODEL_PORT_SRAM == port_type(model_id, circuit_port_id));
  port_sram_orgz_[model_id][circuit_port_id] = sram_orgz;
  return;
}

/* Delay information */
/* Add a delay info:
 * Check if the delay type is in the range of vector
 * if yes, assign values 
 * if no, resize and assign values
 */
void CircuitLibrary::add_delay_info(const CircuitModelId& model_id,
                                    const enum spice_model_delay_type& delay_type) {
  /* validate the model_id */
  VTR_ASSERT(valid_model_id(model_id));
  /* Check the range of vector */
  if (size_t(delay_type) >= delay_types_[model_id].size()) {
    /* Resize */
    delay_types_[model_id].resize(size_t(delay_type) + 1); 
    delay_in_port_names_[model_id].resize(size_t(delay_type) + 1); 
    delay_out_port_names_[model_id].resize(size_t(delay_type) + 1); 
    delay_values_[model_id].resize(size_t(delay_type) + 1); 
  }
  delay_types_[model_id][size_t(delay_type)] = delay_type; 
  return;
}

void CircuitLibrary::set_delay_in_port_names(const CircuitModelId& model_id,
                                             const enum spice_model_delay_type& delay_type,
                                             const std::string& in_port_names) {
  /* validate the model_id */
  VTR_ASSERT(valid_model_id(model_id));
  /* Validate delay_type */
  VTR_ASSERT(valid_delay_type(model_id, delay_type));
  delay_in_port_names_[model_id][size_t(delay_type)] = in_port_names; 
  return; 
}

void CircuitLibrary::set_delay_out_port_names(const CircuitModelId& model_id,
                                             const enum spice_model_delay_type& delay_type,
                                             const std::string& out_port_names) {
  /* validate the model_id */
  VTR_ASSERT(valid_model_id(model_id));
  /* Validate delay_type */
  VTR_ASSERT(valid_delay_type(model_id, delay_type));
  delay_out_port_names_[model_id][size_t(delay_type)] = out_port_names; 
  return; 
}

void CircuitLibrary::set_delay_values(const CircuitModelId& model_id,
                                      const enum spice_model_delay_type& delay_type,
                                      const std::string& delay_values) {
  /* validate the model_id */
  VTR_ASSERT(valid_model_id(model_id));
  /* Validate delay_type */
  VTR_ASSERT(valid_delay_type(model_id, delay_type));
  delay_values_[model_id][size_t(delay_type)] = delay_values; 
  return; 
}

/* Buffer/Inverter-related parameters */
void CircuitLibrary::set_buffer_type(const CircuitModelId& model_id,
                                     const enum e_spice_model_buffer_type& buffer_type) {
  /* validate the model_id */
  VTR_ASSERT(valid_model_id(model_id));
  /* validate that the type of this model should be BUFFER or INVERTER */
  VTR_ASSERT(SPICE_MODEL_INVBUF == model_type(model_id));
  buffer_types_[model_id] = buffer_type; 
  return;
}

void CircuitLibrary::set_buffer_size(const CircuitModelId& model_id,
                                     const float& buffer_size) {
  /* validate the model_id */
  VTR_ASSERT(valid_model_id(model_id));
  /* validate that the type of this model should be BUFFER or INVERTER */
  VTR_ASSERT(SPICE_MODEL_INVBUF == model_type(model_id));
  buffer_sizes_[model_id] = buffer_size; 
  return;
}

void CircuitLibrary::set_buffer_num_levels(const CircuitModelId& model_id,
                                           const size_t& num_levels) {
  /* validate the model_id */
  VTR_ASSERT(valid_model_id(model_id));
  /* validate that the type of this model should be BUFFER or INVERTER */
  VTR_ASSERT(SPICE_MODEL_INVBUF == model_type(model_id));
  buffer_num_levels_[model_id] = num_levels; 
  return;
}

void CircuitLibrary::set_buffer_f_per_stage(const CircuitModelId& model_id,
                                            const size_t& f_per_stage) {
  /* validate the model_id */
  VTR_ASSERT(valid_model_id(model_id));
  /* validate that the type of this model should be BUFFER or INVERTER */
  VTR_ASSERT(SPICE_MODEL_INVBUF == model_type(model_id));
  buffer_f_per_stage_[model_id] = f_per_stage; 
  return;
}

/* Pass-gate-related parameters */
void CircuitLibrary::set_pass_gate_logic_type(const CircuitModelId& model_id,
                                              const enum e_spice_model_pass_gate_logic_type& pass_gate_logic_type) {
  /* validate the model_id */
  VTR_ASSERT(valid_model_id(model_id));
  /* validate that the type of this model should be BUFFER or INVERTER */
  VTR_ASSERT(SPICE_MODEL_PASSGATE == model_type(model_id));
  pass_gate_logic_types_[model_id] = pass_gate_logic_type; 
  return;
}

void CircuitLibrary::set_pass_gate_logic_nmos_size(const CircuitModelId& model_id,
                                                   const float& nmos_size) {
  /* validate the model_id */
  VTR_ASSERT(valid_model_id(model_id));
  /* validate that the type of this model should be BUFFER or INVERTER */
  VTR_ASSERT(SPICE_MODEL_PASSGATE == model_type(model_id));
  pass_gate_logic_sizes_[model_id].set_x(nmos_size); 
  return;
}

void CircuitLibrary::set_pass_gate_logic_pmos_size(const CircuitModelId& model_id,
                                                   const float& pmos_size) {
  /* validate the model_id */
  VTR_ASSERT(valid_model_id(model_id));
  /* validate that the type of this model should be BUFFER or INVERTER */
  VTR_ASSERT(SPICE_MODEL_PASSGATE == model_type(model_id));
  pass_gate_logic_sizes_[model_id].set_y(pmos_size); 
  return;
}

/* Multiplexer-related parameters */
void CircuitLibrary::set_mux_structure(const CircuitModelId& model_id,
                                       const enum e_spice_model_structure& mux_structure) {
  /* validate the model_id */
  VTR_ASSERT(valid_model_id(model_id));
  /* validate that the type of this model should be MUX or LUT */
  VTR_ASSERT( (SPICE_MODEL_MUX == model_type(model_id))
           || (SPICE_MODEL_LUT == model_type(model_id)) );
  mux_structure_[model_id] = mux_structure; 
  return;
}

void CircuitLibrary::set_mux_num_levels(const CircuitModelId& model_id,
                                        const size_t& num_levels) {
  /* validate the model_id */
  VTR_ASSERT(valid_model_id(model_id));
  /* validate that the type of this model should be MUX or LUT */
  VTR_ASSERT( (SPICE_MODEL_MUX == model_type(model_id))
           || (SPICE_MODEL_LUT == model_type(model_id)) );
  mux_num_levels_[model_id] = num_levels; 
  return;
}

void CircuitLibrary::set_mux_const_input_value(const CircuitModelId& model_id,
                                               const size_t& const_input_value) {
  /* validate the model_id */
  VTR_ASSERT(valid_model_id(model_id));
  /* validate that the type of this model should be MUX or LUT */
  VTR_ASSERT( (SPICE_MODEL_MUX == model_type(model_id))
           || (SPICE_MODEL_LUT == model_type(model_id)) );
  /* validate the const input values */
  VTR_ASSERT( valid_mux_const_input_value(const_input_value) );
  mux_const_input_values_[model_id] = const_input_value; 
  return;
}

void CircuitLibrary::set_mux_use_local_encoder(const CircuitModelId& model_id,
                                               const bool& use_local_encoder) {
  /* validate the model_id */
  VTR_ASSERT(valid_model_id(model_id));
  /* validate that the type of this model should be MUX or LUT */
  VTR_ASSERT( (SPICE_MODEL_MUX == model_type(model_id))
           || (SPICE_MODEL_LUT == model_type(model_id)) );
  mux_use_local_encoder_[model_id] = use_local_encoder; 
  return;
}

void CircuitLibrary::set_mux_use_advanced_rram_design(const CircuitModelId& model_id,
                                                      const bool& use_advanced_rram_design) {
  /* validate the model_id */
  VTR_ASSERT(valid_model_id(model_id));
  /* validate that the type of this model should be MUX or LUT */
  VTR_ASSERT( (SPICE_MODEL_MUX == model_type(model_id))
           || (SPICE_MODEL_LUT == model_type(model_id)) );
  mux_use_advanced_rram_design_[model_id] = use_advanced_rram_design; 
  return;
}

/* LUT-related parameters */
void CircuitLibrary::set_lut_is_fracturable(const CircuitModelId& model_id,
                                            const bool& is_fracturable) {
  /* validate the model_id */
  VTR_ASSERT(valid_model_id(model_id));
  /* validate that the type of this model should be LUT */
  VTR_ASSERT(SPICE_MODEL_LUT == model_type(model_id));
  lut_is_fracturable_[model_id] = is_fracturable; 
  return;
}

/* Gate-related parameters */
void CircuitLibrary::set_gate_type(const CircuitModelId& model_id,
                                   const enum e_spice_model_gate_type& gate_type) {
  /* validate the model_id */
  VTR_ASSERT(valid_model_id(model_id));
  /* validate that the type of this model should be GATE */
  VTR_ASSERT(SPICE_MODEL_GATE == model_type(model_id));
  gate_types_[model_id] = gate_type; 
  return;
}


/* RRAM-related design technology information */
void CircuitLibrary::set_rram_rlrs(const CircuitModelId& model_id,
                                   const float& rlrs) {
  /* validate the model_id */
  VTR_ASSERT(valid_model_id(model_id));
  /* validate that the design_tech of this model should be RRAM */
  VTR_ASSERT(SPICE_MODEL_DESIGN_RRAM == design_tech_type(model_id));
  rram_res_[model_id].set_x(rlrs); 
  return;
}

void CircuitLibrary::set_rram_rhrs(const CircuitModelId& model_id,
                                   const float& rhrs) {
  /* validate the model_id */
  VTR_ASSERT(valid_model_id(model_id));
  /* validate that the design_tech of this model should be RRAM */
  VTR_ASSERT(SPICE_MODEL_DESIGN_RRAM == design_tech_type(model_id));
  rram_res_[model_id].set_y(rhrs); 
  return;
}

void CircuitLibrary::set_rram_wprog_set_nmos(const CircuitModelId& model_id,
                                             const float& wprog_set_nmos) {
  /* validate the model_id */
  VTR_ASSERT(valid_model_id(model_id));
  /* validate that the design_tech of this model should be RRAM */
  VTR_ASSERT(SPICE_MODEL_DESIGN_RRAM == design_tech_type(model_id));
  wprog_set_[model_id].set_x(wprog_set_nmos); 
  return;
}

void CircuitLibrary::set_rram_wprog_set_pmos(const CircuitModelId& model_id,
                                             const float& wprog_set_pmos) {
  /* validate the model_id */
  VTR_ASSERT(valid_model_id(model_id));
  /* validate that the design_tech of this model should be RRAM */
  VTR_ASSERT(SPICE_MODEL_DESIGN_RRAM == design_tech_type(model_id));
  wprog_set_[model_id].set_y(wprog_set_pmos); 
  return;
}

void CircuitLibrary::set_rram_wprog_reset_nmos(const CircuitModelId& model_id,
                                               const float& wprog_reset_nmos) {
  /* validate the model_id */
  VTR_ASSERT(valid_model_id(model_id));
  /* validate that the design_tech of this model should be RRAM */
  VTR_ASSERT(SPICE_MODEL_DESIGN_RRAM == design_tech_type(model_id));
  wprog_reset_[model_id].set_x(wprog_reset_nmos); 
  return;
}

void CircuitLibrary::set_rram_wprog_reset_pmos(const CircuitModelId& model_id,
                                               const float& wprog_reset_pmos) {
  /* validate the model_id */
  VTR_ASSERT(valid_model_id(model_id));
  /* validate that the design_tech of this model should be RRAM */
  VTR_ASSERT(SPICE_MODEL_DESIGN_RRAM == design_tech_type(model_id));
  wprog_reset_[model_id].set_y(wprog_reset_pmos); 
  return;
}

/* Wire parameters */
void CircuitLibrary::set_wire_type(const CircuitModelId& model_id,
                                   const enum e_wire_model_type& wire_type) {
  /* validate the model_id */
  VTR_ASSERT(valid_model_id(model_id));
  /* validate that the type of this model should be WIRE or CHAN_WIRE */
  VTR_ASSERT( (SPICE_MODEL_WIRE      == model_type(model_id))
           || (SPICE_MODEL_CHAN_WIRE == model_type(model_id)) );
  wire_types_[model_id] = wire_type; 
  return;
}

void CircuitLibrary::set_wire_r(const CircuitModelId& model_id,
                                const float& r_val) {
  /* validate the model_id */
  VTR_ASSERT(valid_model_id(model_id));
  /* validate that the type of this model should be WIRE or CHAN_WIRE */
  VTR_ASSERT( (SPICE_MODEL_WIRE      == model_type(model_id))
           || (SPICE_MODEL_CHAN_WIRE == model_type(model_id)) );
  wire_rc_[model_id].set_x(r_val); 
  return;
}

void CircuitLibrary::set_wire_c(const CircuitModelId& model_id,
                                const float& c_val) {
  /* validate the model_id */
  VTR_ASSERT(valid_model_id(model_id));
  /* validate that the type of this model should be WIRE or CHAN_WIRE */
  VTR_ASSERT( (SPICE_MODEL_WIRE      == model_type(model_id))
           || (SPICE_MODEL_CHAN_WIRE == model_type(model_id)) );
  wire_rc_[model_id].set_y(c_val); 
  return;
}

void CircuitLibrary::set_wire_num_levels(const CircuitModelId& model_id,
                                         const size_t& num_level) {
  /* validate the model_id */
  VTR_ASSERT(valid_model_id(model_id));
  /* validate that the type of this model should be WIRE or CHAN_WIRE */
  VTR_ASSERT( (SPICE_MODEL_WIRE      == model_type(model_id))
           || (SPICE_MODEL_CHAN_WIRE == model_type(model_id)) );
  wire_num_levels_[model_id] = num_level; 
  return;
}

/************************************************************************
 * Internal Mutators: builders and linkers 
 ***********************************************************************/
/* Set the information for a buffer 
 * For a buffer type, we check if it is in the range of vector
 * If yes, just assign values
 * If no, resize the vector and then assign values 
 */
void CircuitLibrary::set_model_buffer(const CircuitModelId& model_id, const enum e_buffer_type buffer_type, 
                                              const bool& existence, const std::string& model_name) {
  /* validate the model_id */
  VTR_ASSERT(valid_model_id(model_id));
  /* Check the range of vector */
  if (size_t(buffer_type) >= buffer_existence_[model_id].size()) {
    /* Resize and assign values */
    buffer_existence_[model_id].resize(size_t(buffer_type) + 1); 
    buffer_model_names_[model_id].resize(size_t(buffer_type) + 1); 
    buffer_model_ids_[model_id].resize(size_t(buffer_type) + 1); 
    buffer_location_maps_[model_id].resize(size_t(buffer_type) + 1); 
  }
  /* Now we are in the range, assign values */
  buffer_existence_[model_id][size_t(buffer_type)] = existence;
  buffer_model_names_[model_id][size_t(buffer_type)] = model_name;
  buffer_model_ids_[model_id][size_t(buffer_type)] = CircuitModelId::INVALID(); /* Set an OPEN id here, which will be linked later */
  return;
}

/* Link the model_id for each port of a circuit model.
 * We search the inv_model_name in the CircuitLibrary and 
 * configure the port inv_model_id
 */
void CircuitLibrary::link_port_model(const CircuitModelId& model_id) { 
  /* validate the model_id */
  VTR_ASSERT(valid_model_id(model_id));
  /* Walk through each ports, get the port id and find the circuit model id by name */
  for (auto& port_id : ports(model_id)) {
    /* Bypass empty name */
    if (true == port_model_names_[model_id][port_id].empty()) {
      continue;
    }
    port_model_ids_[model_id][port_id] = model(port_model_names_[model_id][port_id]);
  }
  return;
}      

/* Link the inv_model_id for each port of a circuit model.
 * We search the inv_model_name in the CircuitLibrary and 
 * configure the port inv_model_id
 */
void CircuitLibrary::link_port_inv_model(const CircuitModelId& model_id) { 
  /* validate the model_id */
  VTR_ASSERT(valid_model_id(model_id));
  /* Walk through each ports, get the port id and find the circuit model id by name */
  for (auto& port_id : ports(model_id)) {
    /* Bypass empty name */
    if (true == port_inv_model_names_[model_id][port_id].empty()) {
      continue;
    }
    port_inv_model_ids_[model_id][port_id] = model(port_inv_model_names_[model_id][port_id]);
  }
  return;
}      

/* Link all the circuit model ids for each port of a circuit model */
void CircuitLibrary::link_port_models(const CircuitModelId& model_id) {
  link_port_model(model_id); 
  link_port_inv_model(model_id); 
  return;
}

/* Link the buffer_model
 * We search the buffer_model_name in the CircuitLibrary and 
 * configure the buffer_model_id
 */
void CircuitLibrary::link_buffer_model(const CircuitModelId& model_id) { 
  /* validate the model_id */
  VTR_ASSERT(valid_model_id(model_id));
  /* Get the circuit model id by name, skip those with empty names*/
  for (size_t buffer_id = 0; buffer_id < buffer_model_names_[model_id].size(); ++buffer_id) {
    if (true == buffer_model_names_[model_id][buffer_id].empty()) {
      return;
    }
    buffer_model_ids_[model_id][buffer_id] = model(buffer_model_names_[model_id][buffer_id]);
  }
  return;
}      

/* Link the buffer_model
 * We search the buffer_model_name in the CircuitLibrary and 
 * configure the buffer_model_id
 */
void CircuitLibrary::link_pass_gate_logic_model(const CircuitModelId& model_id) { 
  /* validate the model_id */
  VTR_ASSERT(valid_model_id(model_id));
  /* Get the circuit model id by name, skip those with empty names*/
  if (true == pass_gate_logic_model_names_[model_id].empty()) {
    return;
  }
  pass_gate_logic_model_ids_[model_id] = model(pass_gate_logic_model_names_[model_id]);
  return;
}      

/* Build the links for attributes of each model by searching the model_names */
void CircuitLibrary::build_model_links() {
  /* Walk through each circuit model, build links one by one */
  for (auto& model_id : models()) {
    /* Build links for buffers, pass-gates model */
    link_buffer_model(model_id); 
    link_pass_gate_logic_model(model_id); 
    /* Build links for ports */
    link_port_models(model_id); 
  }
  return;
}

/* Build the timing graph for a circuit models*/
void CircuitLibrary::build_model_timing_graph(const CircuitModelId& model_id) {
  /* Now we start allocating a timing graph 
   * Add outgoing edges for each input pin of the circuit model 
   */
  for (auto& from_port_id : input_ports(model_id)) {
    /* Add edges for each input pin  */
    for (auto& from_pin_id : pins(model_id, from_port_id)) {
      /* We should walk through output pins here */
      for (auto& to_port_id : output_ports(model_id)) {
        for (auto& to_pin_id : pins(model_id, to_port_id)) {
          /* Skip self-loops */
          if (from_port_id == to_port_id) {
            continue;
          }
          /* Add an edge to bridge the from_pin_id and to_pin_id */
          add_edge(model_id, from_port_id, from_pin_id, to_port_id, to_pin_id);
        }
      }
    }
  }
  return;
}

/* Build the timing graph for a circuit models*/
void CircuitLibrary::build_timing_graphs() {
  /* Walk through each circuit model, build timing graph one by one */
  for (auto& model_id : models()) {
    /* Free the timing graph if it already exists, we will rebuild one */
    invalidate_model_timing_graph(model_id);
    build_model_timing_graph(model_id); 
    /* Annotate timing information */
    set_timing_graph_delays(model_id);
  }
  return;
}

/************************************************************************
 * Internal mutators: build timing graphs 
 ***********************************************************************/
/* Add an edge between two pins of two ports, and assign an default timing value */
void CircuitLibrary::add_edge(const CircuitModelId& model_id,
                              const CircuitPortId& from_port, const size_t& from_pin, 
                              const CircuitPortId& to_port,   const size_t& to_pin) {
  /* validate the model_id */
  VTR_ASSERT(valid_model_id(model_id));

  /* Create an edge in the edge id list */ 
  CircuitEdgeId edge_id = CircuitEdgeId(edge_ids_[model_id].size());
  /* Expand the edge list  */
  edge_ids_[model_id].push_back(edge_id);
  
  /* Initialize other attributes */

  /* Update the list of incoming edges for to_port */
  /* Resize upon need */
  if (to_pin >= port_in_edge_ids_[model_id][to_port].size()) {
    port_in_edge_ids_[model_id][to_port].resize(to_pin + 1);
  }
  port_in_edge_ids_[model_id][to_port][to_pin] = edge_id;

  /* Update the list of outgoing edges for from_port */
  /* Resize upon need */
  if (from_pin >= port_out_edge_ids_[model_id][from_port].size()) {
    port_out_edge_ids_[model_id][from_port].resize(from_pin + 1);
  }
  port_out_edge_ids_[model_id][from_port][from_pin] = edge_id;

  /* Update source ports and pins of the edge */
  edge_src_port_ids_[model_id].push_back(from_port);
  edge_src_pin_ids_[model_id].push_back(from_pin);

  /* Update sink ports and pins of the edge */
  edge_sink_port_ids_[model_id].push_back(to_port);
  edge_sink_pin_ids_[model_id].push_back(to_pin);

  /* Give a default value for timing values */
  std::vector<float> timing_info(NUM_CIRCUIT_MODEL_DELAY_TYPES, 0);
  edge_timing_info_[model_id].push_back(timing_info);

  return;
}

void CircuitLibrary::set_edge_delay(const CircuitModelId& model_id, 
                                    const CircuitEdgeId& circuit_edge_id, 
                                    const enum spice_model_delay_type& delay_type, 
                                    const float& delay_value) {
  /* validate the circuit_edge_id */
  VTR_ASSERT(valid_circuit_edge_id(model_id, circuit_edge_id));
  VTR_ASSERT(valid_delay_type(model_id, delay_type));
  
  edge_timing_info_[model_id][circuit_edge_id][size_t(delay_type)] = delay_value;
  return;
}

/* Annotate delay values on a timing graph */
void CircuitLibrary::set_timing_graph_delays(const CircuitModelId& model_id) {
  /* validate the model_id */
  VTR_ASSERT(valid_model_id(model_id));
  /* Go one delay_info by another */
  for (const auto& delay_type : delay_types_[model_id]) {
    /* Parse the input port names and output names.
     * We will store the parsing results in vectors:
     * 1. vector for port ids for each port name 
     * 2. vector for pin ids for each port name
     */

    /* Parse the string for inputs */
    MultiPortParser input_port_parser(delay_in_port_names_[model_id][size_t(delay_type)]);
    std::vector<BasicPort> input_ports = input_port_parser.ports();
    std::vector<CircuitPortId> input_port_ids;
    std::vector<size_t> input_pin_ids;
    /* Check each element */
    for (auto& port_info : input_ports) {
      /* Try to find a port by the given name */
      CircuitPortId port_id = port(model_id, port_info.get_name());
      /* We must have a valid port and Port width must be 1! */
      VTR_ASSERT(CircuitPortId::INVALID() != port_id);
      if (0 == port_info.get_width()) {
        /* we need to configure the port width if it is zero.
         * This means that parser find some compact port defintion such as <port_name> 
         */
        size_t port_width = port_size(model_id, port_id);
        port_info.set_width(port_width);
      } else {
        VTR_ASSERT(1 == port_info.get_width());
      }
      /* The pin id should be valid! */
      VTR_ASSERT(true == valid_circuit_pin_id(model_id, port_id, port_info.get_lsb()));
      /* This must be an input port! */
      VTR_ASSERT(true == is_input_port(model_id, port_id));
      /* Push to */
      input_port_ids.push_back(port_id);
      input_pin_ids.push_back(port_info.get_lsb());
    }

    /* Parse the string for outputs */
    MultiPortParser output_port_parser(delay_out_port_names_[model_id][size_t(delay_type)]);
    std::vector<BasicPort> output_ports = output_port_parser.ports();
    std::vector<CircuitPortId> output_port_ids;
    std::vector<size_t> output_pin_ids;
    /* Check each element */
    for (auto& port_info : output_ports) {
      /* Try to find a port by the given name */
      CircuitPortId port_id = port(model_id, port_info.get_name());
      /* We must have a valid port and Port width must be 1! */
      VTR_ASSERT(CircuitPortId::INVALID() != port_id);
      if (0 == port_info.get_width()) {
        /* we need to configure the port width if it is zero.
         * This means that parser find some compact port defintion such as <port_name> 
         */
        size_t port_width = port_size(model_id, port_id);
        port_info.set_width(port_width);
      } else {
        VTR_ASSERT(1 == port_info.get_width());
      }
      /* The pin id should be valid! */
      VTR_ASSERT(true == valid_circuit_pin_id(model_id, port_id, port_info.get_lsb()));
      /* This must be an output port! */
      VTR_ASSERT(true == is_output_port(model_id, port_id));
      /* Push to */
      output_port_ids.push_back(port_id);
      output_pin_ids.push_back(port_info.get_lsb());
    }

    /* Parse the delay matrix */
    PortDelayParser port_delay_parser(delay_values_[model_id][size_t(delay_type)]);

    /* Make sure the delay matrix size matches */
    VTR_ASSERT(port_delay_parser.height() == output_port_ids.size());
    VTR_ASSERT(port_delay_parser.height() == output_pin_ids.size());
    VTR_ASSERT(port_delay_parser.width() == input_port_ids.size());
    VTR_ASSERT(port_delay_parser.width() == input_pin_ids.size());

    /* Configure timing graph */
    for (size_t i = 0; i < port_delay_parser.height(); ++i) {
      for (size_t j = 0; j < port_delay_parser.width(); ++j) {
        float delay_value = port_delay_parser.delay(i, j);
        CircuitEdgeId edge_id = edge(model_id, 
                                     input_port_ids[j], input_pin_ids[j],
                                     output_port_ids[i], output_pin_ids[i]); 
        /* make sure we have an valid edge_id */
        VTR_ASSERT(true == valid_circuit_edge_id(model_id, edge_id));
        set_edge_delay(model_id, edge_id,
                       delay_type, delay_value);
      }
    }
  }
  return;
}

/************************************************************************
 * Internal mutators: build fast look-ups 
 ***********************************************************************/
/* Build fast look-up for circuit models */
void CircuitLibrary::build_model_lookup() {
  /* invalidate fast look-up */
  invalidate_model_lookup();
  /* Classify circuit models by type */
  model_lookup_.resize(NUM_CIRCUIT_MODEL_TYPES);
  /* Walk through models and categorize */
  for (auto& id : model_ids_) {
    model_lookup_[model_types_[id]].push_back(id);
  }
  /* Make the default model to be the first element for each type */
  for (auto& type : model_lookup_) {
    /* Skip zero-length parts of look-up */
    if (true == type.empty()) {
      continue;
    }
    /* if the first element is already a default model, we skip this  */
    if (true == model_is_default_[type[0]]) {
      continue;
    }
    /* Check the array, and try to find a default model */
    for (size_t id = 0; id < type.size(); ++id) {
      if (false == model_is_default_[type[id]]) {
        continue;
      }
      /* Once we find a default model, swap with the first element and finish the loop */
      std::swap(type[0], type[id]);
      break;
    }
  }
  return;
}

/* Build fast look-up for circuit model ports */
void CircuitLibrary::build_model_port_lookup(const CircuitModelId& model_id) {
  /* invalidate fast look-up */
  invalidate_model_port_lookup(model_id);
  /* Classify circuit models by type */
  model_port_lookup_[size_t(model_id)].resize(NUM_CIRCUIT_MODEL_PORT_TYPES);
  /* Walk through models and categorize */
  for (auto& port_id : port_ids_[model_id]) {
    model_port_lookup_[size_t(model_id)][port_type(model_id, port_id)].push_back(port_id);
  }
  return;
}

/************************************************************************
 * Internal invalidators/validators 
 ***********************************************************************/
/* Validators */
bool CircuitLibrary::valid_model_id(const CircuitModelId& model_id) const {
  return ( size_t(model_id) < model_ids_.size() ) && ( model_id == model_ids_[model_id] ); 
}

bool CircuitLibrary::valid_circuit_port_id(const CircuitModelId& model_id, const CircuitPortId& circuit_port_id) const {
  /* validate the model_id */
  VTR_ASSERT(valid_model_id(model_id));
  return ( size_t(circuit_port_id) < port_ids_[model_id].size() ) && ( circuit_port_id == port_ids_[model_id][circuit_port_id] ); 
}

bool CircuitLibrary::valid_circuit_pin_id(const CircuitModelId& model_id, const CircuitPortId& circuit_port_id, const size_t& pin_id) const {
  /* validate the model_id */
  VTR_ASSERT(valid_circuit_port_id(model_id, circuit_port_id));
  return ( size_t(pin_id) < port_size(model_id, circuit_port_id) ); 
}

bool CircuitLibrary::valid_delay_type(const CircuitModelId& model_id, const enum spice_model_delay_type& delay_type) const { 
  /* validate the model_id */
  VTR_ASSERT(valid_model_id(model_id));
  return ( size_t(delay_type) < delay_types_[model_id].size() ) && ( delay_type == delay_types_[model_id][size_t(delay_type)] ); 
}

bool CircuitLibrary::valid_circuit_edge_id(const CircuitModelId& model_id, const CircuitEdgeId& circuit_edge_id) const {
  /* validate the model_id */
  VTR_ASSERT(valid_model_id(model_id));
  return ( size_t(circuit_edge_id) < edge_ids_[model_id].size() ) && ( circuit_edge_id == edge_ids_[model_id][circuit_edge_id] ); 
}

/* Validate the value of constant input 
 * A -1 value for the const values means there is no const inputs 
 * A 0 value for the const values means it is logic 0 
 * A 1 value for the const values means it is logic 1 
 * Others are invalid 
 */
bool CircuitLibrary::valid_mux_const_input_value(const size_t& const_input_value) const {
  return ( (size_t(-1) == const_input_value) 
        || (0 == const_input_value)
        || (1 == const_input_value) );
}

/* Invalidators */
/* Empty fast lookup for models*/
void CircuitLibrary::invalidate_model_lookup() const {
  model_lookup_.clear();
  return;
}

/* Empty fast lookup for circuit ports for a model */
void CircuitLibrary::invalidate_model_port_lookup(const CircuitModelId& model_id) const {
  /* validate the model_id */
  VTR_ASSERT(valid_model_id(model_id));
  model_port_lookup_[size_t(model_id)].clear();
  return;
}

/* Clear all the data structure related to the timing graph */
void CircuitLibrary::invalidate_model_timing_graph(const CircuitModelId& model_id) {
  /* validate the model_id */
  VTR_ASSERT(valid_model_id(model_id));
  edge_ids_[model_id].clear();

  for (const auto& port_id : ports(model_id)) {
    port_in_edge_ids_[model_id][port_id].clear();
    port_out_edge_ids_[model_id][port_id].clear();
  } 

  edge_src_port_ids_[model_id].clear();
  edge_src_pin_ids_[model_id].clear();

  edge_sink_port_ids_[model_id].clear();
  edge_sink_pin_ids_[model_id].clear();

  edge_timing_info_[model_id].clear();
  return;
}

/************************************************************************
 * End of file : circuit_library.cpp 
 ***********************************************************************/
