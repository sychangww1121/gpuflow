/*
 * Copyright 2017 of original authors and authors.
 *
 * We use MIT license for this project, checkout LICENSE file in the root of source tree.
 */

#ifndef _ASYNC_LCORE_FUNCTION_CU_H_
#define _ASYNC_LCORE_FUNCTION_CU_H_

#include <vector>
#include <rte_ethdev.h>
#include "cuda_lpm_factory.h"

namespace gpuflow {
namespace cu {

namespace IP_FAMILY {

const uint8_t PTYPE_IPV4 = 0x12;
const uint8_t PTYPE_IPV6 = 0x13;

} // namespace IP_FAMILY

extern "C" {

// Use this for force alignment.
struct CustomEtherIPHeader {
  struct ether_hdr ether_header;
  struct ipv6_hdr ipv6_header;
  uint8_t dst_port;
} __attribute__((__packed__));

}

struct ProcessingBatchFrame {
 public:
  explicit ProcessingBatchFrame(uint8_t batch_size);
  CustomEtherIPHeader *host_custom_ether_ip_headers_burst;
  CustomEtherIPHeader *dev_custom_ether_ip_headers_burst;
  struct rte_mbuf *pkts_burst[128];
  cudaStream_t cuda_stream;
  uint8_t batch_size;
  uint8_t nb_rx;
  bool busy;
  bool ready_to_burst;
  ~ProcessingBatchFrame() {
    cudaFree(host_custom_ether_ip_headers_burst);
    cudaFree(dev_custom_ether_ip_headers_burst);
  }
};

class CudaASyncLCoreFunction {
 public:
  CudaASyncLCoreFunction(uint8_t _port_id, unsigned int _num_of_eth_devs,
                         std::vector<ether_addr> *_mac_addresses_ptr,
                         IPv4RuleEntry *_lpm4_table_ptr, IPv6RuleEntry *_lpm6_table_ptr);
  int SetupCudaDevices();
  void CreateProcessingBatchFrame(int num_of_batch, uint8_t batch_size);
  ProcessingBatchFrame **batch_head;
  int ProcessPacketsBatch(ProcessingBatchFrame *self_batch);
  ~CudaASyncLCoreFunction() {
    delete(batch_head);
  }
 private:
  uint8_t port_id;
  unsigned int num_of_eth_devs;
  std::vector<ether_addr> *mac_addresses_ptr;
  ether_addr *dev_mac_addresses_array;
  IPv4RuleEntry *lpm4_table_ptr;
  IPv6RuleEntry *lpm6_table_ptr;
};

} // namespace cu
} // namespace gpuflow

#endif // _ASYNC_LCORE_FUNCTION_CU_H_
