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

typedef struct _libusb_device_list_portal
{
	int id;
	libusb_context_portal **context_portal;
	libusb_device **devs;
	int cnt_devs;
} libusb_device_portal;

GList *list_libusb_context_portal_alloc = NULL;
GList *list_libusb_devs_alloc = NULL;
GList *list_libusb_devs_list_alloc = NULL;


libusb_device *
get_device_portal(libusb_device *dev)
{
	libusb_device *rc = NULL;
	GList *tmp = g_list_first(list_libusb_devs_list_alloc);
	fprintf(stderr, "lib-usb.c::get_device_portal dev := 0x%lx.\n", dev);

	if(tmp){
		libusb_device_portal *devtmp = tmp->data;
		fprintf(stderr, "lib-usb.c::get_device_portal tmp := 0x%lx, tmp->devs := 0x%lx.\n", devtmp, devtmp->devs);

		if((unsigned long)dev >= (unsigned long)devtmp->devs && (unsigned long)dev <= (unsigned long)devtmp->devs + (unsigned long)devtmp->cnt_devs){
			unsigned long i = (unsigned long)dev - (unsigned long)devtmp->devs;
			fprintf(stderr, "i := %ld, devtmp->devs[%ld] := 0x%lx.\n", i, i, devtmp->devs[i]);
			rc = devtmp->devs[i];
		}//endif true

		tmp = tmp->next;
	}//endif true

	return(rc);
}

void
free_context_portal(libusb_context_portal *context_portal)
{
	if(0 && context_portal && g_list_find(list_libusb_context_portal_alloc, context_portal)){
		if(context_portal->context) g_free(*context_portal->context);
		context_portal->context = 0L;

		if(context_portal->devs && context_portal->cnt_devs > 0
			&& g_list_find(list_libusb_devs_alloc, context_portal->devs)){
			for(int i = 0; i < context_portal->cnt_devs; i++){
				free(context_portal->devs[i]);
				context_portal->devs[i] = 0L;
			}//endfor
			free(context_portal->devs) ;

			g_list_remove(list_libusb_devs_alloc, context_portal->devs);
		}//endif true

		context_portal->devs = 0L;

		context_portal = 0L;
	}//endif true

	g_list_remove(list_libusb_context_portal_alloc, context_portal);
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

	if(g_list_find(list_libusb_context_portal_alloc, context_portal)){
		context_portal->cnt_devs = libusb_get_device_list(*(context_portal->context), &(context_portal->devs));
		libusb_device_portal *device_portal = g_new(libusb_device_portal, 1);
		device_portal->context_portal = context_portal;
		device_portal->devs = context_portal->devs;
		device_portal->cnt_devs = context_portal->cnt_devs;

		list_libusb_devs_alloc = g_list_append(list_libusb_devs_alloc, context_portal->devs);
		list_libusb_devs_list_alloc = g_list_append(list_libusb_devs_list_alloc, device_portal);


		fprintf(stderr, "devs := 0x%lx (0x%lx), context_portal->cnt_devs := %d\n", 
						 (unsigned long)context_portal->devs, (unsigned long)context_portal->devs[0],
						 context_portal->cnt_devs);




		// TODO: Use GVariantBuilder ...
		/*
		GVariantBuilder *builder;
		builder = g_variant_builder_new(G_VARIANT_TYPE("a(t)"));
		for(int i = 0; i < context_portal->cnt_devs; i++)
			g_variant_builder_add(builder, "(t)", context_portal->devs[i]);
		GVariant *byte_array = g_variant_builder_end(builder);
		g_variant_builder_unref(builder);

		fprintf(stderr, "byte_array := 0x%lx.\n", byte_array);
		*/


		g_dbus_method_invocation_return_value (invocation, 
				g_variant_new ("(ti)", context_portal->devs, context_portal->cnt_devs));

	}//endif true
	else{
		g_dbus_method_invocation_return_value (invocation, g_variant_new ("(ti)", 0, -1));
	}//endif false

	// TODO: g_clear_object ?

	fprintf(stderr, "\n");
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
	libusb_device **devs = (libusb_device **)devices_list;

	if(g_list_find(list_libusb_devs_alloc, devs)){
		libusb_free_device_list(devs, unref_devices);
		g_list_remove(list_libusb_devs_alloc, devs);

		/*
		context_portal->devs = 0L;
		context_portal->cnt_devs = 0L;
		*/
	}//endif true

	g_dbus_method_invocation_return_value (invocation, NULL);

	fprintf(stderr, "\n");
	return TRUE;
}

static gboolean
handle_libusb_init (XdpLibUsb *object,
									 GDBusMethodInvocation *invocation,
									gulong context)
{
	fprintf(stderr, "lib-usb.c::handle_lib_init context := 0x%lx \n", context);

	libusb_context_portal *context_portal = g_new(libusb_context_portal, 1);
	memset(context_portal, 0, sizeof(libusb_context_portal));
	list_libusb_context_portal_alloc = g_list_append(list_libusb_context_portal_alloc, context_portal);

	gint result = libusb_init((libusb_context **)&context_portal->context);
	fprintf(stderr, "context_portal := 0x%lx, context := 0x%lx\n", (unsigned long)context_portal,
	(unsigned long)context_portal->context);

	g_dbus_method_invocation_return_value (invocation, g_variant_new ("(ti)", context_portal, result));

	fprintf(stderr, "\n");
	return TRUE;
}

static gboolean
handle_libusb_get_device_descriptor (XdpLibUsb *object,
									 GDBusMethodInvocation *invocation,
									gulong device)
{
	fprintf(stderr, "lib-usb.c::handle_libusb_get_device_descriptor  device := 0x%lx\n", device);
	libusb_device *dev = get_device_portal(device);
	struct libusb_device_descriptor desc;

	int ret = libusb_get_device_descriptor(dev, &desc);
	fprintf(stderr, "lib-usb.c::handle_libusb_get_device_descriptor  ret := %d\n", ret);

	//g_dbus_method_invocation_return_value (invocation, g_variant_new("(i)", ret));
	// TODO: serialize desc struct to dbus response
		/*
	GVariantBuilder *builder;
	builder = g_variant_builder_new(G_VARIANT_TYPE("ai"));

	g_variant_builder_add(builder, "i", (int)desc.bLength);
	g_variant_builder_add(builder, "i", (int)ret);

	GVariant *byte_array = g_variant_builder_end(builder);

	fprintf(stderr, "byte_array := 0x%lx.\n", byte_array);

	GVariant *value = g_variant_new ("a(i)", builder);
	fprintf(stderr, "value := 0x%lx.\n", value);
	*/


	g_dbus_method_invocation_return_value (invocation, g_variant_new (
		"(iiiiiiiiiiiiiii)", 
		(int)desc.bLength,
		(int)desc.bDescriptorType,
		(unsigned int)desc.bcdUSB,
		(unsigned int)desc.bDeviceClass,
		(unsigned int)desc.bDeviceSubClass,
		(unsigned int)desc.bDeviceProtocol,
		(unsigned int)desc.bMaxPacketSize0,
		(unsigned int)desc.idVendor,
		(unsigned int)desc.idProduct,
		(unsigned int)desc.bcdDevice,
		(unsigned int)desc.iManufacturer,
		(unsigned int)desc.iProduct,
		(unsigned int)desc.iSerialNumber,
		(unsigned int)desc.bNumConfigurations,
		ret
		));
	//g_variant_builder_unref(builder);

	fprintf(stderr, "\n");
	return TRUE;
}

static gboolean
handle_libusb_exit (XdpLibUsb *object,
									 GDBusMethodInvocation *invocation,
									gulong context)
{
	fprintf(stderr, "lib-usb.c::handle_libusb_exit context := 0x%lx\n", context);

	libusb_context_portal *context_portal = (libusb_context_portal *) (context);
	fprintf(stderr, "context_portal := 0x%lx", (unsigned long)context_portal);

	if(g_list_find(list_libusb_context_portal_alloc, context_portal)){
		fprintf(stderr, ", context := 0x%lx\n", (unsigned long)context_portal->context);

		libusb_exit((libusb_context *)context_portal->context);

		free_context_portal(context_portal);
	}//endif true

	g_dbus_method_invocation_return_value (invocation, NULL);

	fprintf(stderr, "\n");
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
	iface->handle_libusb_get_device_descriptor = handle_libusb_get_device_descriptor;
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
