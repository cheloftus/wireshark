/* packet-dcerpc-srvsvc.c
 * Routines for SMB \\PIPE\\srvsvc packet disassembly
 * Copyright 2001, Tim Potter <tpot@samba.org>
 * Copyright 2002, Richard Sharpe <rsharpe@ns.aus.com>
 *   decode srvsvc calls where Samba knows them ...
 *
 * $Id: packet-dcerpc-srvsvc.c,v 1.4 2002/05/24 07:09:56 guy Exp $
 *
 * Ethereal - Network traffic analyzer
 * By Gerald Combs <gerald@ethereal.com>
 * Copyright 1998 Gerald Combs
 * 
 * This program is free software; you can redistribute it and/or
 * modify it under the terms of the GNU General Public License
 * as published by the Free Software Foundation; either version 2
 * of the License, or (at your option) any later version.
 * 
 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.
 * 
 * You should have received a copy of the GNU General Public License
 * along with this program; if not, write to the Free Software
 * Foundation, Inc., 59 Temple Place - Suite 330, Boston, MA  02111-1307, USA.
 */

#ifdef HAVE_CONFIG_H
#include "config.h"
#endif

#include <glib.h>
#include <epan/packet.h>
#include "packet-dcerpc.h"
#include "packet-dcerpc-srvsvc.h"
#include "packet-dcerpc-nt.h"
#include "packet-smb-common.h"
#include "smb.h"

static int proto_dcerpc_srvsvc = -1;
static gint ett_dcerpc_srvsvc = -1;
static gint ett_srvsvc_server_info = -1;
static gint ett_srvsvc_share_info = -1;

static gint hf_srvsvc_server = -1;
static gint hf_srvsvc_info_level = -1;
static gint hf_srvsvc_info = -1;
static gint hf_srvsvc_rc = -1;
static gint hf_srvsvc_platform_id = -1;
static gint hf_srvsvc_ver_major = -1;
static gint hf_srvsvc_ver_minor = -1;
static gint hf_srvsvc_server_type = -1;
static gint hf_srvsvc_server_comment = -1;
static gint hf_srvsvc_users = -1;
static gint hf_srvsvc_hidden = -1;
static gint hf_srvsvc_announce = -1;
static gint hf_srvsvc_ann_delta = -1;
static gint hf_srvsvc_licences = -1;
static gint hf_srvsvc_user_path = -1;
static gint hf_srvsvc_share = -1;
static gint hf_srvsvc_share_info = -1;
static gint hf_srvsvc_share_comment = -1;
static gint hf_srvsvc_share_type = -1;

static e_uuid_t uuid_dcerpc_srvsvc = {
        0x4b324fc8, 0x1670, 0x01d3,
        { 0x12, 0x78, 0x5a, 0x47, 0xbf, 0x6e, 0xe1, 0x88 }
};

static guint16 ver_dcerpc_srvsvc = 3;

static int
srvsvc_dissect_pointer_UNICODE_STRING(tvbuff_t *tvb, int offset, 
				      packet_info *pinfo, proto_tree *tree, 
				      char *drep)
{
	dcerpc_info *di;

	di=pinfo->private_data;
	if(di->conformant_run){
		/*just a run to handle conformant arrays, nothing to dissect */
		return offset;
	}

	offset = dissect_ndr_nt_UNICODE_STRING_str(tvb, offset, pinfo, tree, 
						   drep);
	return offset;
}

static int
srvsvc_dissect_SHARE_INFO_struct(tvbuff_t *tvb, int offset, 
				 packet_info *pinfo, proto_tree *tree, 
				 char *drep)
{
  dcerpc_info *di = pinfo->private_data;
  int level = *(int *)(di->private_data);

  switch (level) {
  case 1:

    offset = dissect_ndr_pointer(tvb, offset, pinfo, tree, drep,
				 srvsvc_dissect_pointer_UNICODE_STRING,
				 NDR_POINTER_UNIQUE, "Share",
				 hf_srvsvc_share, 0);

    offset = dissect_ndr_uint32(tvb, offset, pinfo, tree, drep, 
				hf_srvsvc_share_type, NULL);
    offset = dissect_ndr_uint32(tvb, offset, pinfo, tree, drep, 
				hf_srvsvc_share_type, NULL);

    offset = dissect_ndr_pointer(tvb, offset, pinfo, tree, drep, 
				 srvsvc_dissect_pointer_UNICODE_STRING,
				 NDR_POINTER_UNIQUE, "Comment",
				 hf_srvsvc_share_comment, 0);

    break;

  case 2:

    break;

  }

  return offset;
}

static int
srvsvc_dissect_SRV_INFO_100_struct(tvbuff_t *tvb, int offset, 
				   packet_info *pinfo, proto_tree *tree, 
				   char *drep)
{

  offset = dissect_ndr_uint32(tvb, offset, pinfo, tree, drep, 
			      hf_srvsvc_platform_id, NULL);

  offset = dissect_ndr_pointer(tvb, offset, pinfo, tree, drep, 
			       srvsvc_dissect_pointer_UNICODE_STRING,
			       NDR_POINTER_UNIQUE, "Server",
			       hf_srvsvc_server, 0);

  return offset;

}

static int
srvsvc_dissect_pointer_comment_UNICODE_STRING(tvbuff_t *tvb, int offset, 
					      packet_info *pinfo, 
					      proto_tree *tree, 
					      char *drep)
{
  offset = dissect_ndr_pointer(tvb, offset, pinfo, tree, drep,
			       srvsvc_dissect_pointer_UNICODE_STRING,
			       NDR_POINTER_UNIQUE, "Comment",
			       hf_srvsvc_server_comment, 0);

  return offset;

}

static int
srvsvc_dissect_SRV_INFO_101_struct(tvbuff_t *tvb, int offset, 
				   packet_info *pinfo, proto_tree *tree, 
				   char *drep)
{
  offset = dissect_ndr_uint32(tvb, offset, pinfo, tree, drep, 
			      hf_srvsvc_platform_id, NULL);

  offset = dissect_ndr_pointer(tvb, offset, pinfo, tree, drep, 
			       srvsvc_dissect_pointer_UNICODE_STRING,
			       NDR_POINTER_PTR, "Server",
			       hf_srvsvc_server, 0);

  offset = dissect_ndr_uint32(tvb, offset, pinfo, tree, drep, 
			      hf_srvsvc_ver_major, NULL);

  offset = dissect_ndr_uint32(tvb, offset, pinfo, tree, drep, 
			      hf_srvsvc_ver_minor, NULL);

  offset = dissect_ndr_uint32(tvb, offset, pinfo, tree, drep,
			      hf_srvsvc_server_type, NULL);

  offset = dissect_ndr_pointer(tvb, offset, pinfo, tree, drep, 
			       srvsvc_dissect_pointer_UNICODE_STRING,
			       NDR_POINTER_UNIQUE, "Comment",
			       hf_srvsvc_server_comment, 0);

  return offset;

}

/* Seems silly to cut and paste, but that is what I have done ... */
static int
srvsvc_dissect_SRV_INFO_102_struct(tvbuff_t *tvb, int offset, 
				   packet_info *pinfo, proto_tree *tree, 
				   char *drep)
{
  offset = dissect_ndr_uint32(tvb, offset, pinfo, tree, drep, 
			      hf_srvsvc_platform_id, NULL);

  offset = dissect_ndr_pointer(tvb, offset, pinfo, tree, drep, 
			       srvsvc_dissect_pointer_UNICODE_STRING,
			       NDR_POINTER_PTR, "Server",
			       hf_srvsvc_server, 0);

  offset = dissect_ndr_uint32(tvb, offset, pinfo, tree, drep, 
			      hf_srvsvc_ver_major, NULL);

  offset = dissect_ndr_uint32(tvb, offset, pinfo, tree, drep, 
			      hf_srvsvc_ver_minor, NULL);

  offset = dissect_ndr_uint32(tvb, offset, pinfo, tree, drep,
			      hf_srvsvc_server_type, NULL);

  offset = dissect_ndr_pointer(tvb, offset, pinfo, tree, drep, 
			       srvsvc_dissect_pointer_UNICODE_STRING,
			       NDR_POINTER_UNIQUE, "Comment",
			       hf_srvsvc_server_comment, 0);

  offset = dissect_ndr_uint32(tvb, offset, pinfo, tree, drep, 
			      hf_srvsvc_users, NULL);

  offset = dissect_ndr_uint32(tvb, offset, pinfo, tree, drep, 
			      hf_srvsvc_hidden, NULL);

  offset = dissect_ndr_pointer(tvb, offset, pinfo, tree, drep, 
			       srvsvc_dissect_pointer_UNICODE_STRING,
			       NDR_POINTER_UNIQUE, "User Path",
			       hf_srvsvc_user_path, 0);

  return offset;

}

static int
srvsvc_dissect_SVR_INFO_CTR(tvbuff_t *tvb, int offset, 
			    packet_info *pinfo, proto_tree *tree, 
			    char *drep)
{
  proto_item *item = NULL;
  proto_tree *stree = NULL;
  int old_offset = offset;
  guint level;

  if (tree) {
    item = proto_tree_add_text(tree, tvb, offset, -1, "Server Info:");
    stree = proto_item_add_subtree(item, ett_srvsvc_server_info);
  }

  /* [out] LONG switch_value */
  offset = dissect_ndr_uint32(tvb, offset, pinfo, stree, drep, 
			      hf_srvsvc_info_level, &level);

  /* [OUT] LONG pointer to info struct */

  switch (level) {
  case 100:
    offset = dissect_ndr_pointer(tvb, offset, pinfo, stree, drep,
				 srvsvc_dissect_SRV_INFO_100_struct,
				 NDR_POINTER_UNIQUE, "Info Level 100", -1, 0);

      break;

  case 101:
    offset = dissect_ndr_pointer(tvb, offset, pinfo, stree, drep,
				 srvsvc_dissect_SRV_INFO_101_struct,
				 NDR_POINTER_UNIQUE, "Info Level 101", -1, 0);

    break;

  case 102:
    offset = dissect_ndr_pointer(tvb, offset, pinfo, stree, drep,
				 srvsvc_dissect_SRV_INFO_102_struct,
				 NDR_POINTER_UNIQUE, "Info Level 102", -1, 0);

    break;

  }
 
  /* Should set the field here too ...*/
  proto_item_set_len(item, offset - old_offset);
  return offset;

}

static int
srvsvc_dissect_net_srv_get_info_rqst(tvbuff_t *tvb, int offset, 
				     packet_info *pinfo, proto_tree *tree, 
				     char *drep)
{
  /* [in] UNICODE_STRING_2 *srv*/

  offset = dissect_ndr_pointer(tvb, offset, pinfo, tree, drep,
			       srvsvc_dissect_pointer_UNICODE_STRING,
			       NDR_POINTER_UNIQUE, "Server",
			       hf_srvsvc_server, 0);

  /* [in] ULONG level */ 
  offset = dissect_ndr_uint32(tvb, offset, pinfo, tree, drep, 
			      hf_srvsvc_info_level, NULL);


  return offset;
}

static int
srvsvc_dissect_net_srv_get_info_reply(tvbuff_t *tvb, int offset, 
				     packet_info *pinfo, proto_tree *tree, 
				     char *drep)
{

  offset = dissect_ndr_pointer(tvb, offset, pinfo, tree, drep,
			       srvsvc_dissect_SVR_INFO_CTR, NDR_POINTER_REF,
			       "Info", hf_srvsvc_info, 0);

  /* [out] LONG response_code */
  offset = dissect_ndr_uint32(tvb, offset, pinfo, tree, drep, hf_srvsvc_rc, NULL);

  return offset;
}

static int
srvsvc_dissect_net_share_get_info_rqst(tvbuff_t *tvb, int offset, 
				       packet_info *pinfo, proto_tree *tree, 
				       char *drep)
{
  proto_item *item = NULL;
  proto_tree *stree = NULL;
  dcerpc_info *di;

  di=pinfo->private_data;

  /* [IN] UNICODE_STRING_2 *srv */
  offset = dissect_ndr_pointer(tvb, offset, pinfo, tree, drep,
			       srvsvc_dissect_pointer_UNICODE_STRING,
			       NDR_POINTER_UNIQUE, "Server",
			       hf_srvsvc_server, 0);

  /* We need a label for this string here ... */

  item = proto_tree_add_text(tree, tvb, offset, -1, "Share");
  stree = proto_item_add_subtree(item, ett_srvsvc_share_info);
  di->hf_index = hf_srvsvc_share;
  di->levels = 0;

  offset = dissect_ndr_nt_UNICODE_STRING_str(tvb, offset, pinfo, stree, drep);

  offset = dissect_ndr_uint32(tvb, offset, pinfo, tree, drep, 
			       hf_srvsvc_info_level, NULL);

  return offset;
}

static int
srvsvc_dissect_net_share_get_info_reply(tvbuff_t *tvb, int offset, 
					packet_info *pinfo, proto_tree *tree, 
					char *drep)
{
  int level;
  dcerpc_info *di = pinfo->private_data;

  offset = dissect_ndr_uint32(tvb, offset, pinfo, tree, drep,
			      hf_srvsvc_info_level, &level);

  di->private_data = &level; /* Pass this on */

  offset = dissect_ndr_pointer(tvb, offset, pinfo, tree, drep,
			       srvsvc_dissect_SHARE_INFO_struct, 
			       NDR_POINTER_REF, "Info", 
			       hf_srvsvc_share_info, 0);

  offset = dissect_ndr_uint32(tvb, offset, pinfo, tree, drep,
			      hf_srvsvc_rc, NULL);
  return offset;
}

static dcerpc_sub_dissector dcerpc_srvsvc_dissectors[] = {
        { SRV_NETCONNENUM, "SRV_NETCONNENUM", NULL, NULL },
        { SRV_NETFILEENUM, "SRV_NETFILEENUM", NULL, NULL },
        { SRV_NETSESSENUM, "SRV_NETSESSENUM", NULL, NULL },
        { SRV_NET_SHARE_ADD, "SRV_NET_SHARE_ADD", NULL, NULL },
        { SRV_NETSHAREENUM_ALL, "SRV_NETSHAREENUM_ALL", NULL, NULL },
        { SRV_NET_SHARE_GET_INFO, "SRV_NET_SHARE_GET_INFO", 
	  srvsvc_dissect_net_share_get_info_rqst,
	  srvsvc_dissect_net_share_get_info_reply},
        { SRV_NET_SHARE_SET_INFO, "SRV_NET_SHARE_SET_INFO", NULL, NULL },
        { SRV_NET_SHARE_DEL, "SRV_NET_SHARE_DEL", NULL, NULL },
        { SRV_NET_SRV_GET_INFO, "SRV_NET_SRV_GET_INFO", 
	  srvsvc_dissect_net_srv_get_info_rqst, 
	  srvsvc_dissect_net_srv_get_info_reply},
        { SRV_NET_SRV_SET_INFO, "SRV_NET_SRV_SET_INFO", NULL, NULL },
        { SRV_NET_DISK_ENUM, "SRV_NET_DISK_ENUM", NULL, NULL },
        { SRV_NET_REMOTE_TOD, "SRV_NET_REMOTE_TOD", NULL, NULL },
        { SRV_NET_NAME_VALIDATE, "SRV_NET_NAME_VALIDATE", NULL, NULL },
        { SRV_NETSHAREENUM, "SRV_NETSHAREENUM", NULL, NULL },
        { SRV_NETFILEQUERYSECDESC, "SRV_NETFILEQUERYSECDESC", NULL, NULL },
        { SRV_NETFILESETSECDESC, "SRV_NETFILESETSECDESC", NULL, NULL },

        {0, NULL, NULL,  NULL },
};

void 
proto_register_dcerpc_srvsvc(void)
{
        static hf_register_info hf[] = {
	  { &hf_srvsvc_server,
	    { "Server", "srvsvc.server", FT_STRING, BASE_NONE,
	    NULL, 0x0, "Server Name", HFILL}},
	  { &hf_srvsvc_info_level,
	    { "Info Level", "svrsvc.info_level", FT_UINT32, 
	    BASE_DEC, NULL, 0x0, "Info Level", HFILL}},
	  { &hf_srvsvc_info,
	    { "Info Structure", "srvsvc.info_struct", FT_BYTES,
	    BASE_HEX, NULL, 0x0, "Info Structure", HFILL}},
	  { &hf_srvsvc_rc,
	    { "Return code", "srvsvc.rc", FT_UINT32, 
	      BASE_DEC, NULL, 0x0, "Return Code", HFILL}},

	  { &hf_srvsvc_platform_id,
	    { "Platform id", "srvsvc.info.platform_id", FT_UINT32,
	      BASE_HEX, NULL, 0x0, "Platform ID", HFILL}},
	  { &hf_srvsvc_ver_major,
	    { "Major Version", "srvsvc.version.major", FT_UINT32,
	      BASE_DEC, NULL, 0x0, "Major Version", HFILL}},
	  { &hf_srvsvc_ver_minor,
	    { "Minor Version", "srvsvc.version.minor", FT_UINT32,
	      BASE_DEC, NULL, 0x0, "Minor Version", HFILL}},
	  /* Should break this out */
	  { &hf_srvsvc_server_type,
	    { "Server Type", "srvsvc.server.type", FT_UINT32,
	      BASE_HEX, NULL, 0x0, "Server Type", HFILL}},
	  { &hf_srvsvc_server_comment, 
	    { "Server Comment", "srvsvc.server.comment", FT_STRING,
	      BASE_NONE, NULL, 0x0, "Server Comment String", HFILL}},
	  { &hf_srvsvc_users,
	    { "Users", "srvsvc.users", FT_UINT32,
	      BASE_DEC, NULL, 0x0 , "User Count", HFILL}},
	  { &hf_srvsvc_hidden,
	    { "Hidden", "srvsvc.hidden", FT_UINT32,
	      BASE_DEC, NULL, 0x0, "Hidden", HFILL}},
	  { &hf_srvsvc_announce,
	    { "Announce", "srvsvc.announce", FT_UINT32,
	      BASE_DEC, NULL, 0x0, "Announce", HFILL }},
	  { &hf_srvsvc_ann_delta,
	    { "Announce Delta", "srvsvc.ann_delta", FT_UINT32,
	      BASE_DEC, NULL, 0x0, "Announce Delta", HFILL}},
	  { &hf_srvsvc_licences,
	    { "Licences", "srvsvc.licences", FT_UINT32,
	      BASE_DEC, NULL, 0x0, "Licences", HFILL}},
	  { &hf_srvsvc_user_path,
	    { "User Path", "srvsvc.user_path", FT_STRING,
	      BASE_NONE, NULL, 0x0, "User Path", HFILL}},
	  { &hf_srvsvc_share, 
	    { "Share", "srvsvc.share", FT_STRING,
	      BASE_NONE, NULL, 0x0, "Share", HFILL}},
	  { &hf_srvsvc_share_info,
	    { "Share Info", "srvsvc.share_info", FT_BYTES,
	      BASE_HEX, NULL, 0x0, "Share Info", HFILL}},
	  { &hf_srvsvc_share_comment,
	    { "Share Comment", "srvsvc.share_comment", FT_STRING,
	      BASE_NONE, NULL, 0x0, "Share Comment", HFILL}},
	  { &hf_srvsvc_share_type,
	    { "Share Type", "srvsvc.share_type", FT_UINT32, 
	      BASE_HEX, NULL, 0x0, "Share Type", HFILL}},
	};

        static gint *ett[] = {
                &ett_dcerpc_srvsvc,
		&ett_srvsvc_server_info,
		&ett_srvsvc_share_info,
        };

        proto_dcerpc_srvsvc = proto_register_protocol(
                "Microsoft Server Service", "SRVSVC", "srvsvc");

	proto_register_field_array(proto_dcerpc_srvsvc, hf, array_length(hf));
        proto_register_subtree_array(ett, array_length(ett));
}

void
proto_reg_handoff_dcerpc_srvsvc(void)
{
        /* Register protocol as dcerpc */

        dcerpc_init_uuid(proto_dcerpc_srvsvc, ett_dcerpc_srvsvc, 
                         &uuid_dcerpc_srvsvc, ver_dcerpc_srvsvc, 
                         dcerpc_srvsvc_dissectors);
}
