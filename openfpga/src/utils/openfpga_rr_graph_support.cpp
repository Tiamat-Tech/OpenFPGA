#include "openfpga_rr_graph_support.h"

#include "vtr_log.h"

namespace openfpga {

/********************************************************************
 * A function to identify if the routing resource graph generated by
 * VPR is support by OpenFPGA
 * - Currently we only support uni-directional
 *   It means every routing tracks must have a direction
 *******************************************************************/
bool is_vpr_rr_graph_supported(const RRGraphView& rr_graph) {
  /* Check if the rr_graph is uni-directional*/
  for (const RRNodeId& node : rr_graph.nodes()) {
    if (e_rr_type::CHANX != rr_graph.node_type(node) &&
        e_rr_type::CHANY != rr_graph.node_type(node)) {
      continue;
    }
    if (Direction::BIDIR == rr_graph.node_direction(node)) {
      VTR_LOG_ERROR(
        "Routing resource graph is bi-directional. OpenFPGA currently supports "
        "uni-directional routing architecture only.\n");
      return false;
    }
    if (Direction::NONE == rr_graph.node_direction(node)) {
      VTR_LOG_ERROR(
        "Routing resource graph contains routing tracks which has not specific "
        "direction. OpenFPGA currently supports uni-directional routing "
        "architecture only.\n");
      return false;
    }
  }

  return true;
}

} /* end namespace openfpga */
