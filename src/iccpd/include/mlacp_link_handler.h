/*
 * mlacp_link_handler.h
 * Copyright(c) 2016-2019 Nephos/Estinet.
 *
 * This program is free software; you can redistribute it and/or modify it
 * under the terms and conditions of the GNU General Public License,
 * version 2, as published by the Free Software Foundation.
 *
 * This program is distributed in the hope it will be useful, but WITHOUT
 * ANY WARRANTY; without even the implied warranty of MERCHANTABILITY or
 * FITNESS FOR A PARTICULAR PURPOSE.  See the GNU General Public License for
 * more details.
 *
 * You should have received a copy of the GNU General Public License along with
 * this program; if not, see <http://www.gnu.org/licenses/>.
 *
 * The full GNU General Public License is included in this distribution in
 * the file called "COPYING".
 *
 *  Maintainer: jianjun, grace Li from nephos
 */

#ifndef __MLACP_LINK_HANDLER__
#define __MLACP_LINK_HANDLER__

#include <stdbool.h>
#include "../include/iccp_csm.h"
#include "../include/mlacp_tlv.h"

#define ICCP_MLAGSYNCD_RECV_MSG_BUFFER_SIZE CSM_BUFFER_SIZE

extern char g_iccp_recv_buf[];

/*****************************************
* Link Handler
*
* ***************************************/
void mlacp_portchannel_state_handler(struct CSM* csm, struct LocalInterface* local_if, int po_state);
void mlacp_peer_conn_handler(struct CSM* csm);
void mlacp_peer_disconn_handler(struct CSM* csm);
void mlacp_peerlink_up_handler(struct CSM* csm);
void mlacp_peerlink_down_handler(struct CSM* csm);
void update_stp_peer_link(struct CSM *csm, struct PeerInterface *peer_if, int po_state, int new_create);
void update_peerlink_isolate_from_pif(struct CSM *csm, struct PeerInterface *pif, int po_state, int new_create);
void mlacp_mlag_link_add_handler(struct CSM *csm, struct LocalInterface *lif);
void mlacp_mlag_link_del_handler(struct CSM *csm, struct LocalInterface *lif);
void set_peerlink_mlag_port_learn(struct LocalInterface *lif, int enable);
void peerlink_port_isolate_cleanup(struct CSM* csm);
void update_peerlink_isolate_from_all_csm_lif(struct CSM* csm);

int mlacp_fsm_arp_set(char *ifname, uint32_t ip, char *mac);
int mlacp_fsm_arp_del(char *ifname, uint32_t ip);
void del_mac_from_chip(struct MACMsg* mac_msg);
void add_mac_to_chip(struct MACMsg* mac_msg, uint8_t mac_type);
void del_l2mc_from_chip(struct L2MCMsg* l2mc_msg);
void add_l2mc_to_chip(struct L2MCMsg* l2mc_msg, uint8_t l2mc_type);
uint8_t set_mac_local_age_flag(struct CSM *csm, struct MACMsg* mac_msg, uint8_t set, uint8_t update_peer);

extern int mclagd_ctl_sock_create();
extern int mclagd_ctl_sock_accept(int fd);
extern int mclagd_ctl_interactive_process(int client_fd);
extern int parseMacString(const char *str_mac, uint8_t *bin_mac);

char *show_ip_str(uint32_t ipv4_addr);
char *show_ipv6_str(char *ipv6_addr);

void syncd_info_close();
int iccp_connect_syncd();

void mlacp_link_disable_traffic_distribution(struct LocalInterface *lif);
void mlacp_link_enable_traffic_distribution(struct LocalInterface *lif);
int mlacp_link_set_iccp_state(int mlag_id, bool is_oper_up);
int mlacp_link_set_iccp_role(int mlag_id, bool is_active_role, uint8_t *system_id);
int mlacp_link_set_iccp_system_id(int mlag_id, uint8_t *system_id);
int mlacp_link_del_iccp_info(int mlag_id);
int mlacp_link_set_remote_if_state(int mlag_id, char *po_name, bool is_oper_up);
int mlacp_link_del_remote_if_info(int mlag_id, char *po_name);


void mlacp_mlag_intf_detach_handler(struct CSM* csm, struct LocalInterface* local_if);
void mlacp_peer_mlag_intf_delete_handler(struct CSM* csm, char *mlag_if_name);

int iccp_mclagsyncd_msg_handler(struct System *sys);
int syn_local_neigh_mac_info_to_peer(struct LocalInterface *local_if, int sync_add, int is_v4, int is_v6, int sync_mac, int ack);
int syn_local_mac_info_to_peer(struct CSM* csm, struct LocalInterface *local_if, int sync_add);
int syn_local_arp_info_to_peer(struct CSM* csm, struct LocalInterface *local_if, int sync_add, int ack);
int syn_local_nd_info_to_peer(struct CSM* csm, struct LocalInterface *local_if, int sync_add, int ack);
int is_unique_ip_configured(char *ifname);
#endif
