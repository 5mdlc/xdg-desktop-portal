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
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.	See the GNU
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

#include <libusb.h>

#include "lib-usb.h"
#include "request.h"
#include "xdp-dbus.h"
#include "xdp-utils.h"




typedef struct _libusb_context_portal
{
	int id;
	libusb_context **context;
	libusb_device **devs;
	int cnt_devs;
} libusb_context_portal;

//libusb_context_portal *context_portal;
void
free_context_portal(libusb_context_portal *context_portal)
{
	if(0 && context_portal){
		//if(context_portal->context) free(*context_portal->context);
		context_portal->context = 0L;

		if(context_portal->devs && context_portal->cnt_devs > 0){
			for(int i = 0; i < context_portal->cnt_devs; i++){
				free(context_portal->devs[i]);
				context_portal->devs[i] = 0L;
			}//endfor
			free(context_portal->devs) ;
		}//endif true

		context_portal->devs = 0L;

		context_portal = 0L;
	}//endif true
}//end free_context_portal

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
handle_libusb_get_device_list (XdpLibUsb *object,
									 GDBusMethodInvocation *invocation,
									gulong context,
									gulong device_list)
{
	fprintf(stderr, "lib-usb.c::handle_lib_get_device_list context := 0x%lx, device_list := 0x%lx \n", 
						 context, device_list);
	libusb_context_portal *context_portal = (libusb_context_portal *) (context);

	context_portal->cnt_devs = libusb_get_device_list(*(context_portal->context), &(context_portal->devs));
	fprintf(stderr, "devs := 0x%lx, context_portal->cnt_devs := %d\n", 
						 (unsigned long)context_portal->devs, context_portal->cnt_devs);

	g_dbus_method_invocation_return_value (invocation, g_variant_new ("(ti)", context_portal->devs, context_portal->cnt_devs));

	return TRUE;
}

static gboolean
handle_libusb_free_device_list (XdpLibUsb *object,
									 GDBusMethodInvocation *invocation,
									gulong devices_list,
									gulong unref_devices)
{
	fprintf(stderr, 
				 "lib-usb.c::handle_libusb_free_device_list devices_list := 0x%lx, unref_devices := 0x%ld\n",
				devices_list, unref_devices);
	//libusb_context_portal *context_portal = (libusb_context_portal *) (context);
	libusb_device **devs = (libusb_device **)devices_list;

	libusb_free_device_list(devs, unref_devices);

	/*
	context_portal->devs = 0L;
	context_portal->cnt_devs = 0L;
	*/

	return TRUE;
}

static gboolean
handle_libusb_init (XdpLibUsb *object,
									 GDBusMethodInvocation *invocation,
									gulong context)
{
	fprintf(stderr, "lib-usb.c::handle_lib_init context := 0x%lx \n", context);

	libusb_context_portal *context_portal = malloc(sizeof(libusb_context_portal));
	memset(context_portal, 0, sizeof(libusb_context_portal));

	gint result = libusb_init((libusb_context **)&context_portal->context);
	fprintf(stderr, "context_portal := 0x%lx, context := 0x%lx\n", (unsigned long)context_portal,
	(unsigned long)context_portal->context);

	g_dbus_method_invocation_return_value (invocation, g_variant_new ("(ti)", context_portal, result));

	return TRUE;
}

static gboolean
handle_libusb_exit (XdpLibUsb *object,
									 GDBusMethodInvocation *invocation,
									gulong context)
{
	fprintf(stderr, "lib-usb.c::handle_lib_exit context := 0x%lx\n", context);

	libusb_context_portal *context_portal = (libusb_context_portal *) (context);
	fprintf(stderr, "context_portal := 0x%lx", (unsigned long)context_portal);
	fprintf(stderr, ", context := 0x%lx\n", (unsigned long)context_portal->context);

	libusb_exit((libusb_context *)context_portal->context);

	free_context_portal(context_portal);

	return TRUE;
}

static void
lib_usb_iface_init (XdpLibUsbIface *iface)
{
	fprintf(stderr, "lib-usb.c::lib_usb_iface_init \n");

	iface->handle_libusb_init = handle_libusb_init;
	iface->handle_libusb_exit = handle_libusb_exit;
	iface->handle_libusb_get_device_list = handle_libusb_get_device_list;
	iface->handle_libusb_free_device_list = handle_libusb_free_device_list;
}

static void
lib_usb_init (LibUsb *lu)
{
	//lu->monitor = g_lib_usb_get_default ();
	fprintf(stderr, "lib-usb.c::lib_usb_init \n");

	//g_signal_connect (lu->monitor, "usb-changed", G_CALLBACK (usb_changed), lu);

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
