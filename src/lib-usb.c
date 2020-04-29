/*
 * Copyright © 2016 Red Hat, Inc
 *
 * This program is free software; you can redistribute it and/or
 * modify it under the terms of the GNU Lesser General Public
 * License as published by the Free Software Foundation; either
 * version 2 of the License, or (at your option) any later version.
 *
 * This library is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU
 * Lesser General Public License for more details.
 *
 * You should have received a copy of the GNU Lesser General Public
 * License along with this library. If not, see <http://www.gnu.org/licenses/>.
 *
 * Authors:
 */

#include "config.h"

#include <stdio.h>

#include <string.h>
#include <gio/gio.h>

#include "lib-usb.h"
#include "request.h"
#include "xdp-dbus.h"
#include "xdp-utils.h"

typedef struct _LibUsb LibUsb;
typedef struct _LibUsbClass LibUsbClass;

typedef void GLibUsb;

struct _LibUsb
{
  XdpLibUsbSkeleton parent_instance;

  GLibUsb *monitor;
};

struct _LibUsbClass
{
  XdpLibUsbSkeletonClass parent_class;
};

static LibUsb *lib_usb;

GType lib_usb_get_type (void) G_GNUC_CONST;
static void lib_usb_iface_init (XdpLibUsbIface *iface);

G_DEFINE_TYPE_WITH_CODE (LibUsb, lib_usb, XDP_TYPE_LIB_USB_SKELETON,
                         G_IMPLEMENT_INTERFACE (XDP_TYPE_LIB_USB, lib_usb_iface_init));

static gboolean
handle_get_available (XdpLibUsb     *object,
                      GDBusMethodInvocation *invocation)
{
  Request *request = request_from_invocation (invocation);
  fprintf(stderr, "lib-usb.c::handle_get_available \n");

  if (!xdp_app_info_has_network (request->app_info))
    {
      g_dbus_method_invocation_return_error (invocation,
                                             XDG_DESKTOP_PORTAL_ERROR,
                                             XDG_DESKTOP_PORTAL_ERROR_NOT_ALLOWED,
                                             "This call is not available inside the sandbox");
    }
  else
    {
      //LibUsb *lu = (LibUsb *)object;
      gboolean available = TRUE; //g_lib_usb_get_network_available (lu->monitor);

      g_dbus_method_invocation_return_value (invocation, g_variant_new ("(b)", available));
    }

  return TRUE;
}

static gboolean
handle_get_metered (XdpLibUsb     *object,
                    GDBusMethodInvocation *invocation)
{
  Request *request = request_from_invocation (invocation);
  fprintf(stderr, "lib-usb.c::handle_get_metered \n");

  if (!xdp_app_info_has_network (request->app_info))
    {
      g_dbus_method_invocation_return_error (invocation,
                                             XDG_DESKTOP_PORTAL_ERROR,
                                             XDG_DESKTOP_PORTAL_ERROR_NOT_ALLOWED,
                                             "This call is not available inside the sandbox");
    }
  else
    {
      //LibUsb *lu = (LibUsb *)object;
      gboolean metered = TRUE; //g_lib_usb_get_network_metered (lu->monitor);

      g_dbus_method_invocation_return_value (invocation, g_variant_new ("(b)", metered));
    }

  return TRUE;
}

static gboolean
handle_get_connectivity (XdpLibUsb     *object,
                         GDBusMethodInvocation *invocation)
{
  Request *request = request_from_invocation (invocation);
  fprintf(stderr, "lib-usb.c::handle_get_connectivity \n");

  if (!xdp_app_info_has_network (request->app_info))
    {
      g_dbus_method_invocation_return_error (invocation,
                                             XDG_DESKTOP_PORTAL_ERROR,
                                             XDG_DESKTOP_PORTAL_ERROR_NOT_ALLOWED,
                                             "This call is not available inside the sandbox");
    }
  else
    {
      //LibUsb *lu = (LibUsb *)object;
      //GNetworkConnectivity connectivity = g_lib_usb_get_connectivity (lu->monitor);

      //g_dbus_method_invocation_return_value (invocation, g_variant_new ("(u)", connectivity));
    }

  return TRUE;
}

static gboolean
handle_libusb_init (XdpLibUsb     *object,
                   GDBusMethodInvocation *invocation,
                  gint                  context)
{
  fprintf(stderr, "lib-usb.c::handle_lib_init \n");
  return TRUE;
}

static gboolean
handle_libusb_exit (XdpLibUsb     *object,
                   GDBusMethodInvocation *invocation,
                  gint                  context)
{
  fprintf(stderr, "lib-usb.c::handle_lib_init \n");
  return TRUE;
}

static gboolean
handle_get_status (XdpLibUsb     *object,
                   GDBusMethodInvocation *invocation)
{
  Request *request = request_from_invocation (invocation);
  fprintf(stderr, "lib-usb.c::handle_get_status \n");

  if (!xdp_app_info_has_network (request->app_info))
    {
      g_dbus_method_invocation_return_error (invocation,
                                             XDG_DESKTOP_PORTAL_ERROR,
                                             XDG_DESKTOP_PORTAL_ERROR_NOT_ALLOWED,
                                             "This call is not available inside the sandbox");
    }
  else
    {
      //LibUsb *lu = (LibUsb *)object;
      GVariantBuilder status;
      gboolean b;
      guint c = 0;

      g_variant_builder_init (&status, G_VARIANT_TYPE_VARDICT);
      b = TRUE; //g_lib_usb_get_network_available (lu->monitor);
      g_variant_builder_add (&status, "{sv}",
                             "available", g_variant_new_boolean (b));
      //b = g_lib_usb_get_network_metered (lu->monitor);
      g_variant_builder_add (&status, "{sv}",
                             "metered", g_variant_new_boolean (b));
      //c = g_lib_usb_get_connectivity (lu->monitor);
      g_variant_builder_add (&status, "{sv}", "connectivity", g_variant_new_uint32 (c));
      g_dbus_method_invocation_return_value (invocation, g_variant_new ("(a{sv})", &status));
    }

  return TRUE;
}

	/*
static void
can_reach_done (GObject      *source,
                GAsyncResult *result,
                gpointer      data)
{
  //GLibUsb *monitor = G_LIB_USB (source);
  //g_autoptr(GDBusMethodInvocation) invocation = data;
  //gboolean reachable;
  fprintf(stderr, "lib-usb.c::can_reach_done \n");

  reachable = g_lib_usb_can_reach_finish (monitor, result, NULL);

  g_dbus_method_invocation_return_value (invocation, g_variant_new ("(b)", reachable));
}
  */

static gboolean
handle_can_reach (XdpLibUsb     *object,
                  GDBusMethodInvocation *invocation,
                  const char            *hostname,
                  guint                  port)
{
  Request *request = request_from_invocation (invocation);
  fprintf(stderr, "lib-usb.c::handle_can_reach \n");

  if (!xdp_app_info_has_network (request->app_info))
    {
      g_dbus_method_invocation_return_error (invocation,
                                             XDG_DESKTOP_PORTAL_ERROR,
                                             XDG_DESKTOP_PORTAL_ERROR_NOT_ALLOWED,
                                             "This call is not available inside the sandbox");
    }
  else
    {
      //LibUsb *lu = (LibUsb *)object;
      g_autoptr(GSocketConnectable) address = NULL;

      address = g_network_address_new (hostname, port);
      //g_lib_usb_can_reach_async (lu->monitor, address, NULL, can_reach_done, g_object_ref (invocation)); 
    }

  return TRUE;
}

static void
usb_changed (GObject *object,
                 gboolean network_available,
                 LibUsb *lu)
{
  fprintf(stderr, "lib-usb.c::usb_changed \n");
  //xdp_lib_usb_emit_changed (XDP_LIB_USB (lu));
}

static void
lib_usb_iface_init (XdpLibUsbIface *iface)
{
  fprintf(stderr, "lib-usb.c::lib_usb_iface_init \n");
  iface->handle_get_available = handle_get_available;
  iface->handle_get_metered = handle_get_metered;
  iface->handle_get_connectivity = handle_get_connectivity;
  iface->handle_can_reach = handle_can_reach;

  iface->handle_libusb_init = handle_libusb_init;
  iface->handle_libusb_exit = handle_libusb_exit;
}

static void
lib_usb_init (LibUsb *lu)
{
  //lu->monitor = g_lib_usb_get_default ();
  fprintf(stderr, "lib-usb.c::lib_usb_init \n");

  g_signal_connect (lu->monitor, "usb-changed", G_CALLBACK (usb_changed), lu);

  xdp_lib_usb_set_version (XDP_LIB_USB (lu), 1);
}

static void
lib_usb_class_init (LibUsbClass *klass)
{
  fprintf(stderr, "lib-usb.c::lib_usb_class_init \n");
}

GDBusInterfaceSkeleton *
lib_usb_create (GDBusConnection *connection)
{
  fprintf(stderr, "lib-usb.c::lib_usb_create \n");
  lib_usb = g_object_new (lib_usb_get_type (), NULL);

  return G_DBUS_INTERFACE_SKELETON (lib_usb);
}
