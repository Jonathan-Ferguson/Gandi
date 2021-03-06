/*
 * IS-IS Rout(e)ing protocol - isisd.h   
 *
 * Copyright (C) 2001,2002   Sampo Saaristo
 *                           Tampere University of Technology      
 *                           Institute of Communications Engineering
 *
 * This program is free software; you can redistribute it and/or modify it 
 * under the terms of the GNU General Public Licenseas published by the Free 
 * Software Foundation; either version 2 of the License, or (at your option) 
 * any later version.
 *
 * This program is distributed in the hope that it will be useful,but WITHOUT 
 * ANY WARRANTY; without even the implied warranty of MERCHANTABILITY or 
 * FITNESS FOR A PARTICULAR PURPOSE.  See the GNU General Public License for 
 * more details.

 * You should have received a copy of the GNU General Public License along 
 * with this program; if not, write to the Free Software Foundation, Inc., 
 * 59 Temple Place - Suite 330, Boston, MA  02111-1307, USA.
 */

#ifndef ISISD_H
#define ISISD_H

#define ISISD_VERSION "0.0.7"

/* uncomment if you are a developer in bug hunt */
/* #define EXTREME_DEBUG  */
/* #define EXTREME_TLV_DEBUG */

struct rtnl_handle
{
        int                     fd;
        struct sockaddr_nl      local;
        struct sockaddr_nl      peer;
        __u32                   seq;
        __u32                   dump;
        int                     proto;
        FILE                   *dump_fp;
};

struct rmap
{
  char *name;
  struct route_map *map;
};

struct isis
{
  u_long process_id;
  int sysid_set;
  u_char sysid[ISIS_SYS_ID_LEN];	/* SystemID for this IS */
  u_int32_t router_id;          /* Router ID from zebra */
  struct list *area_list;	/* list of IS-IS areas */
  struct list *init_circ_list;
  struct list *nexthops;	/* IPv4 next hops from this IS */
#ifdef HAVE_IPV6
  struct list *nexthops6;	/* IPv6 next hops from this IS */
#endif				/* HAVE_IPV6 */
  u_char max_area_addrs;	/* maximumAreaAdresses */
  struct area_addr *man_area_addrs;	/* manualAreaAddresses */
  u_int32_t debugs;		/* bitmap for debug */
  time_t uptime;		/* when did we start */
  struct thread *t_dync_clean;	/* dynamic hostname cache cleanup thread */

  /* Redistributed external information. */
  struct route_table *external_info[ZEBRA_ROUTE_MAX + 1];
  /* Redistribute metric info. */
  struct
  {
    int type;			/* Internal or External  */
    int value;			/* metric value */
  } dmetric[ZEBRA_ROUTE_MAX + 1];

  struct
  {
    char *name;
    struct route_map *map;
  } rmap[ZEBRA_ROUTE_MAX + 1];
#ifdef HAVE_IPV6
  struct
  {
    struct
    {
      char *name;
      struct route_map *map;
    } rmap[ZEBRA_ROUTE_MAX + 1];
  } inet6_afmode;
#endif
#ifdef HAVE_TRILL_MONITORING
  int mport;
  int mfd;
  struct isis_passwd mpass;
#endif
};

extern struct isis *isis;

struct isis_area
{
  struct isis *isis;				  /* back pointer */
  dict_t *lspdb[ISIS_LEVELS];			  /* link-state dbs */
  struct isis_spftree *spftree[ISIS_LEVELS];	  /* The v4 SPTs */
  struct route_table *route_table[ISIS_LEVELS];	  /* IPv4 routes */
#ifdef HAVE_IPV6
  struct isis_spftree *spftree6[ISIS_LEVELS];	  /* The v6 SPTs */
  struct route_table *route_table6[ISIS_LEVELS];  /* IPv6 routes */
#endif
  unsigned int min_bcast_mtu;
  struct list *circuit_list;	/* IS-IS circuits */
  struct flags flags;
  struct thread *t_tick;	/* LSP walker */
#ifdef HAVE_TRILL
  struct thread *nl_tick; /*netlink recev tick*/
  int api_version;
#ifdef HAVE_TRILL_MONITORING
  struct thread *mon_tick; /* monitoring tick */
#endif
  struct nl_sock *sock_genl;
  struct rtnl_handle *rth;
  int genl_family;
  int group_number;
  /* used to specify bridge when using multiple one */
  int bridge_id;
#endif
  struct thread *t_lsp_refresh[ISIS_LEVELS];
  int lsp_regenerate_pending[ISIS_LEVELS];

  /*
   * Configurables 
   */
  struct isis_passwd area_passwd;
  struct isis_passwd domain_passwd;
  /* do we support dynamic hostnames?  */
  char dynhostname;
  /* do we support new style metrics?  */
  char newmetric;
  char oldmetric;
  /* identifies the routing instance   */
  char *area_tag;
  /* area addresses for this area      */
  struct list *area_addrs;
  u_int16_t max_lsp_lifetime[ISIS_LEVELS];
  char is_type;			/* level-1 level-1-2 or level-2-only */
  /* are we overloaded? */
  char overload_bit;
  u_int16_t lsp_refresh[ISIS_LEVELS];
  /* minimum time allowed before lsp retransmission */
  u_int16_t lsp_gen_interval[ISIS_LEVELS];
  /* min interval between between consequtive SPFs */
  u_int16_t min_spf_interval[ISIS_LEVELS];
  /* the percentage of LSP mtu size used, before generating a new frag */
  int lsp_frag_threshold;
  int ip_circuits;
  /* logging adjacency changes? */
  u_char log_adj_changes;
#ifdef HAVE_IPV6
  int ipv6_circuits;
#endif				/* HAVE_IPV6 */
  /* Counters */
  u_int32_t circuit_state_changes;
#ifdef HAVE_TRILL
  struct trill *trill;     /* TRILL structure */
#ifdef HAVE_TRILL_MONITORING
  int lost_hello_reset_timer;
#endif /* HAVE_TRILL_MONITORING */
#endif /* HAVE_TRILL */
#ifdef TOPOLOGY_GENERATE
  struct list *topology;
  u_char topology_baseis[ISIS_SYS_ID_LEN];  /* IS for the first IS emulated. */
  char *topology_basedynh;                /* Dynamic hostname base. */
  char top_params[200];                   /* FIXME: what is reasonable? */
#endif /* TOPOLOGY_GENERATE */
};

void isis_init (void);
void isis_new(unsigned long
#ifdef HAVE_TRILL_MONITORING
				, int
#endif
);
struct isis_area *isis_area_create(const char *);
struct isis_area *isis_area_lookup (const char *);
int isis_area_get (struct vty *vty, const char *area_tag);
void print_debug(struct vty *, int, int);
int show_isis_neighbor_common(struct vty *, const char *id, char
#ifdef HAVE_TRILL_MONITORING
                              , uint8_t lost
#endif
                             );

/* Master of threads. */
extern struct thread_master *master;

#define DEBUG_ADJ_PACKETS                (1<<0)
#define DEBUG_CHECKSUM_ERRORS            (1<<1)
#define DEBUG_LOCAL_UPDATES              (1<<2)
#define DEBUG_PROTOCOL_ERRORS            (1<<3)
#define DEBUG_SNP_PACKETS                (1<<4)
#define DEBUG_UPDATE_PACKETS             (1<<5)
#define DEBUG_SPF_EVENTS                 (1<<6)
#define DEBUG_SPF_STATS                  (1<<7)
#define DEBUG_SPF_TRIGGERS               (1<<8)
#define DEBUG_RTE_EVENTS                 (1<<9)
#define DEBUG_EVENTS                     (1<<10)
#define DEBUG_ZEBRA                      (1<<11)
#define DEBUG_PACKET_DUMP                (1<<12)
#ifdef HAVE_TRILL
#define DEBUG_TRILL_EVENTS               (1<<13)
#endif
#endif /* ISISD_H */
