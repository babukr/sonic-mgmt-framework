#!/usr/bin/python
###########################################################################
#
# Copyright 2019 Dell, Inc.
#
# Licensed under the Apache License, Version 2.0 (the "License");
# you may not use this file except in compliance with the License.
# You may obtain a copy of the License at
#
#     http://www.apache.org/licenses/LICENSE-2.0
#
# Unless required by applicable law or agreed to in writing, software
# distributed under the License is distributed on an "AS IS" BASIS,
# WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.
# See the License for the specific language governing permissions and
# limitations under the License.
#
###########################################################################

import sys
import time
import json
import ast
from rpipe_utils import pipestr
import cli_client as cc
from scripts.render_cli import show_cli_output

import urllib3
urllib3.disable_warnings()

nat_type_map = {"snat" : "SNAT", "dnat": "DNAT"}
nat_protocol_map = {"icmp": "1", "tcp": "6", "udp": "17"}
clear_nat_map = {"translations": "ENTRIES", "statistics": "STATISTICS"}
config = True

def invoke_api(func, args=[]):
    global config

    api = cc.ApiClient()

    # Enable/Disable NAT Feature
    if func == 'patch_openconfig_nat_nat_instances_instance_config_enable':
        path = cc.Path('/restconf/data/openconfig-nat:nat/instances/instance={id}/config/enable', id=args[0])
        if args[1] == "True":
           body = { "openconfig-nat:enable": True }
        else:
           body = { "openconfig-nat:enable": False }
        return api.patch(path,body)

    # Config NAT Timeout
    elif func == 'patch_openconfig_nat_nat_instances_instance_config_timeout':
        path = cc.Path('/restconf/data/openconfig-nat:nat/instances/instance={id}/config/timeout', id=args[0])
        body = { "openconfig-nat:timeout":  int(args[1]) }
        return api.patch(path, body)

    # Config NAT TCP Timeout
    elif func == 'patch_openconfig_nat_nat_instances_instance_config_tcp_timeout':
        path = cc.Path('/restconf/data/openconfig-nat:nat/instances/instance={id}/config/tcp-timeout', id=args[0])
        body = { "openconfig-nat:tcp-timeout":  int(args[1]) }
        return api.patch(path, body)

    # Config NAT UDP Timeout
    elif func == 'patch_openconfig_nat_nat_instances_instance_config_udp_timeout':
        path = cc.Path('/restconf/data/openconfig-nat:nat/instances/instance={id}/config/udp-timeout', id=args[0])
        body = { "openconfig-nat:udp-timeout":  int(args[1]) }
        return api.patch(path, body)

    # Config NAT Static basic translation entry
    elif func == 'patch_openconfig_nat_nat_instances_instance_nat_mapping_table_nat_mapping_entry_config':
        path = cc.Path('/restconf/data/openconfig-nat:nat/instances/instance={id}/nat-mapping-table/nat-mapping-entry={externaladdress}/config', id=args[0], externaladdress=args[1])
        body = { "openconfig-nat:config" : { "internal-address": args[2]} }
        l = len(args)
        if l >= 4:
            body["openconfig-nat:config"].update( {"type": nat_type_map[args[3]] } )
        if l == 5:
            body["openconfig-nat:config"].update( {"twice-nat-id": int(args[4])} )
        return api.patch(path, body)

    # Remove NAT Static basic translation entry
    elif func == 'delete_openconfig_nat_nat_instances_instance_nat_mapping_table_nat_mapping_entry':
        path = cc.Path('/restconf/data/openconfig-nat:nat/instances/instance={id}/nat-mapping-table/nat-mapping-entry={externaladdress}', id=args[0], externaladdress=args[1])
        return api.delete(path)

    # Remove all NAT Static basic translation entries
    elif func == 'delete_openconfig_nat_nat_instances_instance_nat_mapping_table':
        path = cc.Path('/restconf/data/openconfig-nat:nat/instances/instance={id}/nat-mapping-table', id=args[0])
        return api.delete(path)

    # Config NAPT Static translation entry
    elif func == 'patch_openconfig_nat_nat_instances_instance_napt_mapping_table_napt_mapping_entry_config':
        path = cc.Path('/restconf/data/openconfig-nat:nat/instances/instance={id}/napt-mapping-table/napt-mapping-entry={externaladdress},{protocol},{externalport}/config', id=args[0],externaladdress=args[1],protocol=nat_protocol_map[args[2]],externalport=args[3])
        body = { "openconfig-nat:config" : {"internal-address": args[4], "internal-port": int(args[5])} }
        l = len(args)
        if l >= 7:
            body["openconfig-nat:config"].update( {"type": nat_type_map[args[6]] } )
        if l == 8:
            body["openconfig-nat:config"].update( {"twice-nat-id": int(args[7])} )
        return api.patch(path, body)

    # Remove NAPT Static translation entry
    elif func == 'delete_openconfig_nat_nat_instances_instance_napt_mapping_table_napt_mapping_entry':
        path = cc.Path('/restconf/data/openconfig-nat:nat/instances/instance={id}/napt-mapping-table/napt-mapping-entry={externaladdress},{protocol},{externalport}', id=args[0],externaladdress=args[1],protocol=nat_protocol_map[args[2]],externalport=args[3])
        return api.delete(path)

    # Config NAT Pool
    elif func == 'patch_openconfig_nat_nat_instances_instance_nat_pool_nat_pool_entry_config':
        path = cc.Path('/restconf/data/openconfig-nat:nat/instances/instance={id}/nat-pool/nat-pool-entry={poolname}/config', id=args[0],poolname=args[1])
        ip = args[2].split("-")
        if len(ip) == 1:
            body = { "openconfig-nat:config": {"IP-ADDRESS": args[2]} }
        else:
            body =  { "openconfig-nat:config": {"IP-ADDRESS-RANGE": args[2]} }

        if len(args) > 3:
            body["openconfig-nat:config"].update( {"nat-port": args[3] } )
        return api.patch(path, body)

    # Remove all NAPT Static basic translation entries
    elif func == 'delete_openconfig_nat_nat_instances_instance_napt_mapping_table':
        path = cc.Path('/restconf/data/openconfig-nat:nat/instances/instance={id}/napt-mapping-table', id=args[0])
        return api.delete(path)

    # Remove NAT Pool
    elif func == 'delete_openconfig_nat_nat_instances_instance_nat_pool_nat_pool_entry':
        path = cc.Path('/restconf/data/openconfig-nat:nat/instances/instance={id}/nat-pool/nat-pool-entry={poolname}',id=args[0],poolname=args[1])
        return api.delete(path)

    # Remove all NAT Pools
    elif func == 'delete_openconfig_nat_nat_instances_instance_nat_pool':
        path = cc.Path('/restconf/data/openconfig-nat:nat/instances/instance={id}/nat-pool', id=args[0])
        return api.delete(path)

    # Config NAT Binding
    elif func == 'patch_openconfig_nat_nat_instances_instance_nat_acl_pool_binding_nat_acl_pool_binding_entry_config':
        path = cc.Path('/restconf/data/openconfig-nat:nat/instances/instance={id}/nat-acl-pool-binding/nat-acl-pool-binding-entry={name}/config', id=args[0],name=args[1])

        body = { "openconfig-nat:config": {"nat-pool": args[2] }}

        # ACL Name
        acl_name = args[3].split("=")[1]
        if acl_name != "" :
            body["openconfig-nat:config"].update( {"access-list": acl_name } )

        # NAT Type
        nat_type = args[4].split("=")[1]
        if nat_type != "":
            body["openconfig-nat:config"].update( {"type": nat_type_map[nat_type] } )

        # Twice NAT ID
        twice_nat_id = args[5].split("=")[1]
        if twice_nat_id != "":
            body["openconfig-nat:config"].update( {"twice-nat-id": int(twice_nat_id)} )

        return api.patch(path, body)

    # Remove NAT Binding
    elif func == 'delete_openconfig_nat_nat_instances_instance_nat_acl_pool_binding_nat_acl_pool_binding_entry_config':
        path = cc.Path('/restconf/data/openconfig-nat:nat/instances/instance={id}/nat-acl-pool-binding/nat-acl-pool-binding-entry={name}/config', id=args[0],name=args[1])
        return api.delete(path)

    # Remove all NAT Bindings
    elif func == 'delete_openconfig_nat_nat_instances_instance_nat_acl_pool_binding':
        path = cc.Path('/restconf/data/openconfig-nat:nat/instances/instance={id}/nat-acl-pool-binding', id=args[0])
        return api.delete(path)

    # Config NAT Zone
    elif func == 'patch_openconfig_interfaces_ext_interfaces_interface_nat_zone_config_nat_zone':
        path = cc.Path('/restconf/data/openconfig-interfaces:interfaces/interface={name}/openconfig-interfaces-ext:nat-zone/config/nat-zone', name=args[1])
        body = { "openconfig-interfaces-ext:nat-zone": int(args[2]) }
        return api.patch(path, body)

    # Remove NAT Zone
    elif func == 'delete_openconfig_interfaces_ext_interfaces_interface_nat_zone_config_nat_zone':
        path = cc.Path('/restconf/data/openconfig-interfaces:interfaces/interface={name}/openconfig-interfaces-ext:nat-zone/config/nat-zone', name=args[1])
        return api.delete(path)

    # Clear NAT Translations/Statistics
    elif func == 'rpc_nat_clear':
        path = cc.Path('/restconf/operations/sonic-nat:clear_nat')
        body = {"sonic-nat:input":{"nat-param": clear_nat_map[args[1]]}}
        return api.post(path,body)


    # Get NAT Global Config
    elif func == 'get_openconfig_nat_nat_instances_instance_config':
        config = False
        path = cc.Path('/restconf/data/openconfig-nat:nat/instances/instance={id}/config', id=args[0])
        return api.get(path)

    # Get NAT Bindings
    elif func == 'get_openconfig_nat_nat_instances_instance_nat_acl_pool_binding':
        config = False
        path = cc.Path('/restconf/data/openconfig-nat:nat/instances/instance={id}/nat-acl-pool-binding', id=args[0])
        return api.get(path)

    # Get NAT Pools
    elif func == 'get_openconfig_nat_nat_instances_instance_nat_pool':
        config = False
        path = cc.Path('/restconf/data/openconfig-nat:nat/instances/instance={id}/nat-pool', id=args[0])
        return api.get(path)

    ## Get NAT Translations
    elif func == 'get_openconfig_nat_nat_instances_instance_nat_mapping_table':
        config = False
        path = cc.Path('/restconf/data/openconfig-nat:nat/instances/instance={id}/nat-mapping-table', id=args[0])
        return api.get(path)

    elif func == 'get_openconfig_nat_nat_instances_instance_napt_mapping_table':
        config = False
        path = cc.Path('/restconf/data/openconfig-nat:nat/instances/instance={id}/napt-mapping-table', id=args[0])
        return api.get(path)

    elif func == 'get_openconfig_nat_nat_instances_instance_nat_twice_mapping_table':
        config = False
        path = cc.Path('/restconf/data/openconfig-nat:nat/instances/instance={id}/nat-twice-mapping-table', id=args[0])
        return api.get(path)

    elif func == 'get_openconfig_nat_nat_instances_instance_napt_twice_mapping_table':
        config = False
        path = cc.Path('/restconf/data/openconfig-nat:nat/instances/instance={id}/napt-twice-mapping-table', id=args[0])
        return api.get(path)

    # Get all L3 interfaces (needed for NAT Zone)
    elif func == 'get_sonic_interface_sonic_interface_interface':
        config = False
        path = cc.Path('/restconf/data/sonic-interface:sonic-interface/INTERFACE')
        return api.get(path)

    elif func == 'get_sonic_vlan_interface_sonic_vlan_interface_vlan_interface':
        config = False
        path = cc.Path('/restconf/data/sonic-vlan-interface:sonic-vlan-interface/VLAN_INTERFACE')
        return api.get(path)

    elif func == 'get_sonic_portchannel_interface_sonic_portchannel_interface_portchannel_interface':
        config = False
        path = cc.Path('/restconf/data/sonic-portchannel-interface:sonic-portchannel-interface/PORTCHANNEL_INTERFACE')
        return api.get(path)

    elif func == 'get_sonic_loopback_interface_sonic_loopback_interface_loopback_interface':
        config = False
        path = cc.Path('/restconf/data/sonic-loopback-interface:sonic-loopback-interface/LOOPBACK_INTERFACE')
        return api.get(path)

    else:
        return api.cli_not_implemented(func)


def get_response_dict(response):
    api_response = {}

    if response.ok():
        if response.content is not None:
            # Get Command Output
            api_response = response.content
    else:
        print response.error_message()

    return api_response

def get_nat_napt_tables(args):
    response = {}

    resp = invoke_api('get_openconfig_nat_nat_instances_instance_nat_mapping_table', args)
    resp = get_response_dict(resp)
    response.update(resp)

    resp = invoke_api('get_openconfig_nat_nat_instances_instance_napt_mapping_table', args)
    resp = get_response_dict(resp)
    response.update(resp)

    return response

def get_twice_nat_napt_tables(args):
    response = {}

    resp = invoke_api('get_openconfig_nat_nat_instances_instance_nat_twice_mapping_table', args)
    resp = get_response_dict(resp)
    response.update(resp)

    resp = invoke_api('get_openconfig_nat_nat_instances_instance_napt_twice_mapping_table', args)
    resp = get_response_dict(resp)
    response.update(resp)


    return response
    

def get_nat_translations(func, args):
    response = {}
    resp = get_nat_napt_tables(args)
    response.update(resp)

    resp = get_twice_nat_napt_tables(args)
    response.update(resp)

    return response

 
def get_nat_zones(func,args):
    output  = {}

    # Get INTERFACE Table
    response = invoke_api("get_sonic_interface_sonic_interface_interface")
    api_response = get_response_dict(response)

    if 'sonic-interface:INTERFACE' in api_response and \
          'INTERFACE_LIST' in api_response['sonic-interface:INTERFACE']:
        for val in api_response['sonic-interface:INTERFACE']['INTERFACE_LIST']:
            if 'nat_zone' in val and 'portname' in val:
                output.update( {val['portname']: val['nat_zone']} )


    # Get VLAN_INTERFACE table
    response1 = invoke_api("get_sonic_vlan_interface_sonic_vlan_interface_vlan_interface")
    api_response1 = get_response_dict(response1)

    if 'sonic-vlan-interface:VLAN_INTERFACE' in api_response1 and \
          'VLAN_INTERFACE_LIST' in api_response1['sonic-vlan-interface:VLAN_INTERFACE']:
        for val in api_response1['sonic-vlan-interface:VLAN_INTERFACE']['VLAN_INTERFACE_LIST']:
            if 'nat_zone' in val and 'vlanName' in val:
                output.update( {val['vlanName']: val['nat_zone']} )

    # Get PORTCHANNEL_INTERFACE table
    response2 = invoke_api("get_sonic_portchannel_interface_sonic_portchannel_interface_portchannel_interface")
    api_response2 = get_response_dict(response2)

    if 'sonic-portchannel-interface:PORTCHANNEL_INTERFACE' in api_response2 and \
          'PORTCHANNEL_INTERFACE_LIST' in api_response2['sonic-portchannel-interface:PORTCHANNEL_INTERFACE']:
        for val in api_response2['sonic-portchannel-interface:PORTCHANNEL_INTERFACE']['PORTCHANNEL_INTERFACE_LIST']:
            if 'nat_zone' in val and 'pch_name' in val:
                output.update( {val['pch_name']: val['nat_zone']} )

    # Get LOOPBACK_INTERFACE table
    response3 = invoke_api("get_sonic_loopback_interface_sonic_loopback_interface_loopback_interface")
    api_response3 = get_response_dict(response3)

    if 'sonic-loopback-interface:LOOPBACK_INTERFACE' in api_response3 and \
          'LOOPBACK_INTERFACE_LIST' in api_response3['sonic-loopback-interface:LOOPBACK_INTERFACE']:
        for val in api_response3['sonic-loopback-interface:LOOPBACK_INTERFACE']['LOOPBACK_INTERFACE_LIST']:
            if 'nat_zone' in val and 'loIfName' in val:
                output.update( {val['loIfName']: val['nat_zone']} )

    return output



def get_count(count, table_name, l):

    table_count_map = {
                        'openconfig-nat:nat-mapping-table': ['static_nat', 'dynamic_nat'],
                        'openconfig-nat:napt-mapping-table': ['static_napt', 'dynamic_napt'],
                        'openconfig-nat:nat-twice-mapping-table': ['static_twice_nat', 'dynamic_twice_nat'],
                        'openconfig-nat:napt-twice-mapping-table': ['static_twice_napt', 'dynamic_twice_napt'],
                      }
    if 'state' in l:
        count['total_entries'] += 1

    if 'state' in l and 'entry-type' in l['state']:
        if l['state']['entry-type'] == 'openconfig-nat:STATIC':
            count[table_count_map[table_name][0]]+=1;
        else:
            count[table_count_map[table_name][1]]+=1;

    if 'state' in l and 'type' in l['state']:
        if l['state']['type'] == 'openconfig-nat:SNAT':
            count['snat_snapt']+=1;
        else:
            count['dnat_dnapt']+=1
    return
    

def get_nat_translations_count(func, args):
    response = get_nat_translations(func, args)
    count = { 'static_nat': 0, 
              'static_napt': 0,
              'dynamic_nat': 0,
              'dynamic_napt': 0,
              'static_twice_nat': 0,
              'static_twice_napt': 0,
              'dynamic_twice_nat': 0,
              'dynamic_twice_napt': 0,
              'snat_snapt': 0,
              'dnat_dnapt': 0,
              'total_entries': 0
	     }

    for key in response:
        for entry in response[key]:
            for l in response[key][entry]:
                get_count(count, key, l)

    return count

def get_stats(key, l):

    stats = { "protocol": "all",
              "source": "---",
              "destination": "---",
              "pkts": "0",
              "bytes": "0"}

    if key == "openconfig-nat:napt-mapping-table" or key == "openconfig-nat:napt-twice-mapping-table" : 
        for k,v in nat_protocol_map.items():
            if v ==  str(l["protocol"]):
                stats["protocol"] = k

    if key == "openconfig-nat:nat-mapping-table":
        if 'type' in l['state'] and l['state']['type'] == 'openconfig-nat:SNAT' :
            stats["source"] = l['external-address']
        elif 'type' in l['state'] and l['state']['type'] == 'openconfig-nat:DNAT' :
            stats["destination"] = l['external-address']
    elif key == "openconfig-nat:napt-mapping-table":
        addr = l['external-address']+":"+str(l['external-port'])
        if 'type' in l['state'] and l['state']['type'] == 'openconfig-nat:SNAT' :
            stats["source"] = addr
        elif 'type' in l['state'] and l['state']['type'] == 'openconfig-nat:DNAT' :
            stats["destination"] = addr
    elif key == "openconfig-nat:nat-twice-mapping-table":
        stats["source"] = l["src-ip"]
        stats["destination"] = l["dst-ip"]
    else:
        stats["source"] = l["src-ip"]+":"+str(l["src-port"])
        stats["destination"] = l["dst-ip"]+":"+str(l["dst-port"])

    if 'state' in l and 'counters' in l['state']:
        if 'nat-translations-bytes' in l['state']['counters']:
            stats["bytes"] = l['state']['counters']['nat-translations-bytes']
        if 'nat-translations-pkts' in l['state']['counters']:
            stats["pkts"] = l['state']['counters']['nat-translations-pkts']

    return stats 


def get_nat_statistics(func, args):
    resp = []
    response = get_nat_translations(func, args)

    for key in response:
        for entry in response[key]:
            for l in response[key][entry]:
                if 'state' in l and 'counters' in l['state']:
                    stats = get_stats(key, l) 
                    if stats is not None:
                        resp.append(stats)
                    
 
    return resp

def get_configs(table_name, l):
    configs = {
                'nat_type': "dnat",
                'ip_protocol': "all",
                'global_ip': "",
                'global_l4_port': "----",
                'local_ip': "",
                'local_l4_port': "----",
                'twice_nat_id': "----"
              }
    if 'config' not in  l:
        return None

    # IP Protocol
    if 'openconfig-nat:napt-mapping-table' == table_name:
        if 'config' in l and 'protocol' in l['config']:
            proto = l['config']['protocol']
        for key,val in nat_protocol_map.items():
            if val ==  str(proto):
                configs['ip_protocol'] = key

    # Nat Type
    if 'config' in l and 'type' in l['config']:
        if l['config']['type'] == "openconfig-nat:SNAT":
            configs['nat_type'] = "snat"

    # Global IP
    if 'config' in l and 'external-address' in l['config']:
        configs['global_ip'] = l['config']['external-address']

    # Global L4 Port
    if 'config' in l and 'external-port' in l['config']:
        configs['global_l4_port'] = l['config']['external-port']

    # Local IP
    if 'config' in l and 'internal-address' in l['config']:
        configs['local_ip'] = l['config']['internal-address']

    # Local L4 Port
    if 'config' in l and 'internal-port' in l['config']:
        configs['local_l4_port'] = l['config']['internal-port']

    # Twice NAT ID
    if 'config' in l and 'twice-nat-id' in l['config']:
        configs['twice_nat_id'] = l['config']['twice-nat-id']
    
    return configs        


def get_nat_static_configs(func,args):
    response = get_nat_napt_tables(args)
    resp = []

    for key in response:
        for entry in response[key]:
            for l in response[key][entry]:
                configs = get_configs(key, l)
                if configs is not None:
                    resp.append(configs)

    return resp

def run(func, args):
    global config

    try:
       args.insert(0,"0")  # NAT instance 0

       if func == 'get_nat_translations':
           api_response = get_nat_translations(func,args)
       elif func == 'get_nat_zones':
           api_response = get_nat_zones(func,args)
       elif func == 'get_nat_statistics':
           api_response = get_nat_statistics(func,args)
       elif func == 'get_nat_translations_count':
           api_response = get_nat_translations_count(func,args)
       elif func == 'get_nat_static_configs':
           api_response = get_nat_static_configs(func,args)
       else:
           response = invoke_api(func, args)
           api_response = get_response_dict(response)

       if config == False:
           show_cli_output(args[1], api_response)
 
    except Exception as e:
        print("Failure: %s\n" %(e))


if __name__ == '__main__':

    pipestr().write(sys.argv)
    func = sys.argv[1]

    run(func, sys.argv[2:])

